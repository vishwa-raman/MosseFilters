// Trainer.cpp
// This file contains the definition of the methods of class Trainer.
// The steps followed here are as follows,
// 1. Create a set of feature extraction objects
// 2. Create an annotations object and read annotations
// 3. Iterate over annotations and extract features
// 4. Write features into files
// 5. Invoke SVM-Light to generate models
// 
// At this point we use SVM-Light as an external process. Eventually, 
// we see creating an SVM-Light library that we can then use directly
// without resorting to using it as an extraneous process

#include "Trainer.h"

// static member initialization
int Trainer::nSamplesBeforeFlush = Globals::largeBufferSize;

// Class construction and destruction

// The outputDirectory will be used to generate all training data and the
// eventual models. The modelNamePrefix is used to name the models. It is
// defined in Globals.cpp. Each model is for a particular zone and the 
// name of the model has the format <modelNamePrefix>_<zone>.model

Trainer::Trainer(string output, KernelType type,
		 Location* le, Location* re, Location* n,
		 roiFnT roiFn, string svm) {
  // check if the output directory exists, or else bail
  DIR* dir;
  struct dirent *ent;
  dir = opendir(output.c_str());
  if (dir == NULL) {
    string err = "Trainer::Trainer. The directory " + output + " does not exist. Bailing out.";
    throw (err);
  }

  // now that we have confirmed the existence of the directory, we remove all training
  // data from the output directory and start from scratch
  while ((ent = readdir(dir))) {
    string fileToDelete = ent->d_name;
    string filename = output + "/" + fileToDelete;
    if (filename.find("zone_") != string::npos)
      remove(filename.c_str());
  }
  closedir(dir);
  
  kernelType = type;
  outputDirectory = output;
  svmPath = (svm != "")? svm + "/" : svm;
  roiFunction = roiFn;
  leftEye = le;
  rightEye = re;
  nose = n;

  // Build a set of feature extraction objects and stuff them into the
  // extractor vector
  Feature* f = (Feature*) new FeatureLX();
  featureExtractors.push_back(f);
  f = (Feature*) new FeatureRX();
  featureExtractors.push_back(f);
  f = (Feature*) new FeatureNX();
  featureExtractors.push_back(f);
  f = (Feature*) new FeatureLRDist();
  featureExtractors.push_back(f);
  f = (Feature*) new FeatureLNDist();
  featureExtractors.push_back(f);
  f = (Feature*) new FeatureRNDist();
  featureExtractors.push_back(f);
  f = (Feature*) new FeatureLNAngle();
  featureExtractors.push_back(f);
  f = (Feature*) new FeatureRNAngle();
  featureExtractors.push_back(f);
  f = (Feature*) new FeatureLRNArea();
  featureExtractors.push_back(f);

  nFeatures = (int)Feature::End - 1;
}

Trainer::~Trainer() {
  for (int i = 0; i < nFeatures; i++)
    delete featureExtractors[i];
  for (unsigned int i = 0; i < data.size(); i++)
    delete data[i];
}

// getLocations
// Method used to get the LOIs for a given frame using the filters for the
// left eye, right eye and nose

bool Trainer::getLocations(IplImage* frame, FrameAnnotation& fa) {
  if (!nose || !leftEye || !rightEye) {
    string err = "Classifier::getZone. Location extractors malformed.";
    throw (err);
  }

  bool isValid = true;

  // the roi offset
  CvPoint offset;

  // LOIs
  CvPoint leftEyeLocation;
  CvPoint rightEyeLocation;
  CvPoint noseLocation;

  // computing the confidence of the location identification
  double leftPSR;
  double rightPSR;
  double nosePSR;

  offset.x = offset.y = 0;
  IplImage* roi = (roiFunction)? roiFunction(frame, fa, offset, Annotations::Face) : 0;

  // all location extractors do identical preprocessing. Therefore, preprocess
  // once using say the left eye extractor and re-use it for all three extractors
  fftw_complex* preprocessedImage = leftEye->getPreprocessedImage((roi)? roi : frame);

  #pragma omp parallel sections num_threads(2)
  {
    #pragma omp section
    {
      leftEye->setImage(preprocessedImage);
      leftEye->apply();
      leftEye->getMaxLocation(leftEyeLocation, leftPSR);
      leftEyeLocation.x += offset.x;
      leftEyeLocation.y += offset.y;
    }

    #pragma omp section
    {
      // get the location of the right eye
      rightEye->setImage(preprocessedImage);
      rightEye->apply();
      rightEye->getMaxLocation(rightEyeLocation, rightPSR);
      rightEyeLocation.x += offset.x;
      rightEyeLocation.y += offset.y;
    }
  }

  if (roi)
    cvReleaseImage(&roi);

  CvPoint center;
  center.x = (leftEyeLocation.x + rightEyeLocation.x) / 2;
  center.y = leftEyeLocation.y + Globals::noseDrop;

  fa.setNose(center);

  offset.x = offset.y = 0;
  roi = (roiFunction)? roiFunction(frame, fa, offset, Annotations::Nose) : 0;

  // free the preprocessed image
  fftw_free(preprocessedImage);

  // all location extractors do identical preprocessing. Therefore, preprocess
  // once using say the left eye extractor and re-use it for all three extractors
  preprocessedImage = nose->getPreprocessedImage((roi)? roi : frame);

  // get the location of the nose
  nose->setImage(preprocessedImage);
  nose->apply();
  nose->getMaxLocation(noseLocation, nosePSR);
  noseLocation.x += offset.x;
  noseLocation.y += offset.y;

  // free the preprocessed image
  fftw_free(preprocessedImage);

  fa.setLeftIris(leftEyeLocation);
  fa.setRightIris(rightEyeLocation);
  fa.setNose(noseLocation);

  // we are done with the images at this point. Free roi if not zero
  if (roi)
    cvReleaseImage(&roi);

  return isValid;
}

// addTrainingSet
// This method is used to add a training set as a directory name. The directory
// is expected to contain a file called <annotations.xml> which contains the 
// locations of interest for each frame and a zone of interest for that frame.
// For every annotation we extract a set of features using the extractors and 
// store them as strings in the format expected by SVM-Light

void Trainer::addTrainingSet(string trainingDataDirName) {
  Annotations annotations;

  // first capture the mapping from file names to locations of interest
  // Unlike in the case of filter generation, we don't bin frames here
  string locationsFileName = trainingDataDirName + "/" + Globals::annotationsFileName;
  annotations.readAnnotations(locationsFileName);

  // get face center
  CvPoint& center = annotations.getCenter();
  cout << "Center in " << trainingDataDirName << " is (" << center.x << ", " << 
    center.y << ")" << endl;

  // now get the set of all annotations
  vector<FrameAnnotation*> frameAnnotations = annotations.getFrameAnnotations();
  for (unsigned int i = 0; i < frameAnnotations.size(); i++) {
    FrameAnnotation* fa = frameAnnotations[i];

    // check if the annotations exist. If they don't, we extract them using
    // the filters and then train based on the identified locations
    CvPoint& leftEye = fa->getLOI(Annotations::LeftEye);
    if (!leftEye.x && !leftEye.y) {
      fa->setFace(center);

      char buffer[Globals::midBufferSize];
      sprintf(buffer, "frame_%d.png", fa->getFrameNumber());
      string fileName = buffer;
      fileName = trainingDataDirName + "/" + fileName;
      IplImage* frame = cvLoadImage((const char*)fileName.c_str());
      if (!frame) {
        string str = "Trainer::addTrainingSet. Fatal. Cannot load image " + fileName;
        throw(str);
      }
      bool isValid = getLocations(frame, *fa);
      cvReleaseImage(&frame);
      if (!isValid)
	continue;
    }

    // allocate a DataClass for feature values and their class
    DataClass* dc = new DataClass(fa->getZone());

    // for each annotation, extract all features and create a line with
    // training data in the SVM Light format
    for (unsigned int j = 0; j < featureExtractors.size(); j++) {
      double value = featureExtractors[j]->extract(fa);
      dc->add(value);
    }

    // now stuff the DataClass object into the data vector
    data.push_back(dc);
  }  
}

// normalize
// Method to normalize all training data to be in the range [-1, 1]. We accumulate
// the min and max feature values when we extract features in the feature extractors.
// Notice that the min and max values may change as we process different training
// data sets. Once all training sets have been added, we have the global min and max
// values for all features across all training samples. We therefore expect this
// method to be called from within generate after all calls to addTrainingSet have
// been made

void Trainer::normalize() {
  // first compute averages and ranges
  vector<double> average;
  vector<double> spread;

  // we normalize all data points to lie in the interval [-1, 1] using the
  // average of the extremal values and their spread
  for (unsigned int i = 0; i < featureExtractors.size(); i++) {
    // feature specific normalization
    double min = featureExtractors[i]->getMinVal();
    double max = featureExtractors[i]->getMaxVal();

    average.push_back((max + min) / 2.0);
    spread.push_back(max - min);
  }

  // iterate over all data vectors and normalize each of them using the vector
  // of averages.
  DataClass* dc;
  vector<double>* values;
  unsigned int j;

  #pragma omp parallel for num_threads(omp_get_max_threads()) private(dc, values, j)
  for (unsigned int i = 0; i < data.size(); i++) {
    dc = data[i];
    values = &(dc->getData());

    // normalize each value
    // note that the size of values will be the same as the number of feature 
    // extractors
    for (j = 0; j < values->size(); j++) {
      (*values)[j] = ((*values)[j] - average[j]) / spread[j]; 
    }
  }
}

// write
// Method used to write all training samples into their respective zone specific
// files

void Trainer::write() {
  // we open as many files as the number of zones. We compose a line in the SVM
  // Light format for each DataClass object in the data vector, prepend whether
  // or not it is a positive or negative sample for a given zone and write it 
  // into the zone specific output file

  // training and model data filename prefix
  string prefix = "zone_";

  // output files
  vector<ofstream*> files;

  for (unsigned int i = 0; i < Globals::numZones; i++) {
    char buffer[Globals::smallBufferSize];
    sprintf(buffer, "%d.data", i + 1);
    string str = buffer;
    string filename = outputDirectory + "/" + prefix + str;
    ofstream* file = new ofstream();
    file->open((const char*)filename.c_str());
    if (!file->good()) {
      string err = "Trainer::write. Unable to open file " + filename;
      throw (err);
    }
    files.push_back(file);
  }

  // now iterate over all data vectors and write them to each file
  for (unsigned int i = 0; i < data.size(); i++) {
    DataClass* dc = data[i];
    
    string line = dc->getDataSVMLight();
    for (unsigned int i = 0; i < Globals::numZones; i++) {
      int zone = i + 1;
      string str = "";
      if (dc->getZone() == zone)
	str = "1" + line;
      else
	str = "-1" + line;
      (*files[i]) << str << endl;
    }
  }

  // close all files and free up memory
  for (unsigned int i = 0; i < files.size(); i++) {
    files[i]->close();
    delete files[i];
  }

  // Now write out a parameter values file that will be used for
  // classification
  ofstream paramsFile;
  
  // compose filename
  string filename = outputDirectory + "/" + Globals::paramsFileName;

  paramsFile.open((const char*)filename.c_str());
  if (paramsFile.good()) {
    paramsFile << "<?xml version=\"1.0\"?>" << endl;
    paramsFile << "<parameters>" << endl;
    for (unsigned int i = 0; i < featureExtractors.size(); i++) {
      paramsFile << "  <feature id=\"" << Feature::featureNames[i] << "\">" << endl;
      paramsFile << "    <min>" << featureExtractors[i]->getMinVal() << "</min>" << endl;
      paramsFile << "    <max>" << featureExtractors[i]->getMaxVal() << "</max>" << endl;
      paramsFile << "  </feature>" << endl;
    }
    paramsFile << "</parameters>" << endl;
    paramsFile.close();
  }
}

// generate
// Method to generate models from training data files. This method is expected
// to be called once the training data has been used to extract features. At this 
// point we call SVM-Light as a separate executable for training after generating
// data files int the format required by SVM-Light

void Trainer::generate() {
  // first normalize all data points
  normalize();

  // generate data in the format expected by SVM-Light
  write();

  // generate models using svm_learn
  for (unsigned int i = 0; i < Globals::numZones; i++) {
    char buffer[Globals::smallBufferSize];
    sprintf(buffer, "%d.model", i + 1);
    string str = buffer;
    string modelName = outputDirectory + "/" + Globals::modelNamePrefix + str;
    
    sprintf(buffer, "zone_%d.data", i + 1);
    str = buffer;
    string dataFileName = outputDirectory + "/" + str;

    // compose command
    sprintf(buffer, "-t %d", kernelType);
    string kernelTypeStr = buffer;
    string cmd = svmPath + "svm_learn " + kernelTypeStr + " " + dataFileName + " " + modelName;
    int result = system((const char*)cmd.c_str());
    if (result != 0) {
      ; // ignore for now
    }
  }
}
