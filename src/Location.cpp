// Location.cpp
// This file contains the implementation of class Location. The Location class is the
// workhorse class that does all of the operations that are common to the set of all
// extractors of locations of interest. The steps performed are,
// a. Create a filter from file
// b. Take an image as input
// c. Transform the image as needed
// d. Apply the filter to the transformed image
// e. Provide min and max values, and locations for feature extraction

#include "Location.h"

// Class construction and destruction

Location::Location(string outputDirName, Annotations::Tag tag,
		   CvPoint& windowCenter) : xmlTag(tag) {
  inputImg = 0;
  postFilterImg = 0;
  imageFFT = 0;

  filter = new Filter(outputDirName, tag, windowCenter);
  if (!filter) {
    string err = "Location::Location. Filter does not appear to exist.";
    throw (err);
  }

  // initialize past locations
  for (int i = 0; i < Globals::nPastLocations; i++) {
    CvPoint* point = new CvPoint;
    point->x = point->y = -1;
    pastLocations.push_back(point);
  }
  pastLocationIndex = 0;
}

Location::Location(Filter* f) {
  inputImg = 0;
  postFilterImg = 0;
  imageFFT = 0;
  xmlTag = Annotations::Ignore;

  filter = f;

  for (int i = 0; i < Globals::nPastLocations; i++) {
    CvPoint* point = new CvPoint;
    point->x = point->y = -1;
    pastLocations.push_back(point);
  }
  pastLocationIndex = 0;
}

Location::~Location() {
  if (filter)
    delete filter;
  for (int i = 0; i < Globals::nPastLocations; i++)
    delete pastLocations[i];
}

// setImage
// Method that takes an image as input, converts it into first a grayscale
// image and then into a complex image which is stored

void Location::setImage(IplImage* image) {
  if (!image) {
    string err = "Location::setImage. image parameter is NULL.";
    throw (err);
  }

  inputImg = image;
  postFilterImg = 0;

  // check sizes
  imgSize = cvGetSize(image);
  CvSize filterSize = filter->getSize();

  if (imgSize.height != filterSize.height || imgSize.width != filterSize.width) {
    string err = "Location::setImage. filter size and image sizes do not match!";
    char buffer[Globals::midBufferSize];
    sprintf(buffer, "(%d, %d) vs. (%d, %d)", imgSize.height, imgSize.width,
	    filterSize.height, filterSize.width);
    err += buffer;
    throw (err);
  }
}

// setImage
// This overloaded method is used to pass a preprocessed image to the location
// object. This is for optimizing filter application

void Location::setImage(fftw_complex* image) {
  imageFFT = image;
}

// apply
// Method that applies a pre-loaded filter to an image. The method stores the
// result in the postFilterImg field. Subsequent to a call to apply, the various
// get methods can be used to extract LOIs

bool Location::apply() {
  if (!filter) {
    string err = "Location::apply. filter object is NULL.";
    throw (err);
  }

  // preprocess the image only if we don't already have a preprocessed one
  if (!imageFFT) {
    if (!inputImg) {
      string err = "Location::apply. Please call setImage first to set the input image.";
      throw (err);
    }

    bool releaseImage = false;
    IplImage* image = inputImg;

    // check if the image needs conversion to grayscale
    if ((inputImg->nChannels != 1 && strcmp(inputImg->colorModel, "GRAY"))) {
      image = cvCreateImage(imgSize, IPL_DEPTH_8U, 1);
      cvCvtColor(inputImg, image, CV_BGR2GRAY);
      releaseImage = true;
    }

    imageFFT = filter->preprocessImage(image);
    postFilterImg = filter->apply(imageFFT);
    imageFFT = 0;
    /*
    string str = "__postfilter_";
    str += Filter::filterName(xmlTag);
    Filter::showImage((const char*)str.c_str(), postFilterImg);
    */
    if (releaseImage)
      cvReleaseImage(&image);

    if (postFilterImg)
      return true;
  } else {
    postFilterImg = filter->apply(imageFFT);
    imageFFT = 0;
    /*
    string str = "__postfilter_";
    str += Filter::filterName(xmlTag);
    Filter::showImage((const char*)str.c_str(), postFilterImg);
    */
    if (postFilterImg)
      return true;
  }
  return false;
}

// getPreprocessedImage
// Method used to get a preprocessed image, given an IplImage. This method is
// used to do one time preprocessing and multiple time applications of a given
// image frame

fftw_complex* Location::getPreprocessedImage(IplImage* inputImg) {
  IplImage* image = inputImg;

  // get input image size. This function may get called without calling
  // setImage and hence we need to keep track of image size
  imgSize = cvGetSize(image);

  // check if the image needs conversion to grayscale
  bool releaseImage = false;
  if ((inputImg->nChannels != 1 && strcmp(inputImg->colorModel, "GRAY"))) {
    image = cvCreateImage(imgSize, IPL_DEPTH_8U, 1);
    cvCvtColor(inputImg, image, CV_BGR2GRAY);
    releaseImage = true;
  }

  // preprocess the input image
  imageFFT = filter->preprocessImage(image);

  // copy it into a buffer
  int nElements = imgSize.height * ((imgSize.width / 2) + 1);
  fftw_complex* buffer = (fftw_complex*)fftw_malloc(sizeof(fftw_complex) * nElements);
  memcpy(buffer, imageFFT, (sizeof(fftw_complex) * nElements));

  if (releaseImage)
    cvReleaseImage(&image);

  return buffer;
}

// getMinValue
// Method used to get the minimum element in a real image array

double Location::getMinValue() {
  if (!postFilterImg) {
    string err = "Location::getMinValue. Invalid image";
    throw (err);
  }
  double min;
  double max;

  cvMinMaxLoc(postFilterImg, &min, &max, NULL, NULL);
  return min;
}

// getMaxValue
// Method used to get the maximum element in a real image array

double Location::getMaxValue() {
  if (!postFilterImg) {
    string err = "Location::getMaxValue. Invalid image";
    throw (err);
  }
  double min;
  double max;

  cvMinMaxLoc(postFilterImg, &min, &max, NULL, NULL);
  return max;
}

// getMinLocation
// Method used to get the location of the min element as a point in the 2d space

void Location::getMinLocation(CvPoint& location, double& psr) {
  if (!postFilterImg) {
    string err = "Location::getMinLocation. Invalid image";
    throw (err);
  }
  double min;
  double max;
  CvPoint minLoc;
  CvPoint maxLoc;
  
  cvMinMaxLoc(postFilterImg, &min, &max, &minLoc, &maxLoc);
  location.x = minLoc.x;
  location.y = minLoc.y;

  // no PSR for min locations
  psr = 0.0;
}

// getMaxLocation
// Method used to get the location of the max element as a point in the 2d space

void Location::getMaxLocation(CvPoint& location, double& psr) {
  if (!postFilterImg) {
    string err = "Location::getMaxLocation. Invalid image";
    throw (err);
  }
  double min;
  double max;
  CvPoint minLoc;
  CvPoint maxLoc;
  
  cvMinMaxLoc(postFilterImg, &min, &max, &minLoc, &maxLoc);

  // apply smoothing by averaging the currently computed location and 
  // several past locations
  int xSum = maxLoc.x;
  int ySum = maxLoc.y;
  int nTerms = 1;

  for (int i = 0; i < Globals::nPastLocations; i++)
    if (pastLocations[i]->x != -1) {
      xSum += pastLocations[i]->x;
      ySum += pastLocations[i]->y;
      nTerms++;
    }

  location.x = xSum / nTerms;
  location.y = ySum / nTerms;

  // now push the latest location into the array of past locations
  pastLocations[pastLocationIndex]->x = maxLoc.x;
  pastLocations[pastLocationIndex]->y = maxLoc.y;
  pastLocationIndex = (pastLocationIndex + 1) % Globals::nPastLocations;

  psr = computePSR(max, maxLoc);
}

// computePSR
// Method that computes the PSR given a location

double Location::computePSR(double max, CvPoint& location) {
  if (!postFilterImg)
    return 0;

  int halfWidth = Globals::psrWidth / 2;
  if (location.x - halfWidth < 0)
    halfWidth += location.x - halfWidth;
  if (location.y - halfWidth < 0)
    halfWidth += location.y - halfWidth;
  if (halfWidth < 0)
    return 0;

  int psrWidth = halfWidth * 2;

  int x = location.x - halfWidth;
  int y = location.y - halfWidth;

  CvRect rect;
  rect.x = x; rect.y = y;
  rect.height = rect.width = psrWidth;
  cvSetImageROI(postFilterImg, rect);

  // now compute the average of the pixels inside the ROI
  CvScalar mean = cvAvg(postFilterImg, NULL);
  cvResetImageROI(postFilterImg);

  // if the mean is negative, then simply return 0
  if (mean.val[0] < 0)
    return 0;

  // compute sd by first computing the average boundary pixel value
  // along two edges of the PSR frame and then compute sd using the
  // average value
  int count = 0;
  double boundary = 0;
  for (int i = y; i < y + psrWidth; i++) {
    if (i < imgSize.height) {
      CvScalar vec = cvGet2D(postFilterImg, i, x);
      boundary += vec.val[0];
      count++;
    }
  }
  for (int i = x; i < x + psrWidth; i++) {
    if (i < imgSize.width) {
      CvScalar vec = cvGet2D(postFilterImg, y, i);
      boundary += vec.val[0];
      count++;
    }
  }
  boundary /= count;
    
  double sd = max - boundary;

  //  cout << "max = " << max << ", mean = " << mean.val[0] << ", boundary = " << boundary <<
  //    ", sd = " << sd << endl;
  
  return (max - mean.val[0]) / sd;
}
