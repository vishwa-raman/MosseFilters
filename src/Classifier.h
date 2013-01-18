// Classifier.h
// The declaration of class Classifier. The classifier uses the set of feature extraction
// classes to extract features from LOIs. The locations are picked up using instances of
// the Location class for each LOI. The classification is done using models generated 
// using SVM Light

#ifndef __CLASSIFIER_H
#define __CLASSIFIER_H

#include <sys/types.h>
#include <dirent.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <algorithm>
#include <iostream>
#include <fstream>
#include <sstream>

#include "Globals.h"
#include "Location.h"
#include "Annotations.h"
#include "Trainer.h"

// openCV stuff
#include <cv.h>
#include <highgui.h>

// SVM Light stuff
#ifdef __cplusplus
extern "C" {
#include "svm_common.h"
}
#endif

using namespace std;

class Classifier {
 private:
  string outputDirectory;         // the output directory name for all generated data

  // vector of feature extraction objects
  vector<Feature*> featureExtractors;

  // locations of interest extractors
  Location* leftEye;              // the left eye location extractor
  Location* rightEye;             // the right eye location extractor
  Location* nose;                 // the nose extractor

  roiFnT roiFunction;             // the ROI extractor

  // SVM models vector, one per zone
  vector<MODEL*> models;

  // Kernel type
  Trainer::KernelType kernelType; // The kernel type used for training

  int nFeatures;                  // the number of features

 public:
  // public types
  enum ErrorType {
    OneNorm,
    TwoNorm,
    MSE
  };

 public:
  Classifier(string outputDirectory, Trainer::KernelType kernelType, 
	     Location* le, Location* re, Location* n,
	     roiFnT roiFunction = 0);
  virtual ~Classifier();

  // method to get the gaze zone and confidence
  virtual int getZone(IplImage* frame, double& confidence, FrameAnnotation& fa);

  // method to get the error rate
  virtual pair<double, string> getError(string trainingDirectory);

  // method to get filter error given a LOI
  virtual double getFilterError(string trainingDirectory, Annotations::Tag tag,
				ErrorType errorType);

  // methods to get and set Location extractors
  void setLeftEyeExtractor(Location* le) { leftEye = le; }
  void setRightEyeExtractor(Location* re) { rightEye = re; }
  void setNoseExtractor(Location* n) { nose = n; }
  Filter* getFilter(Annotations::Tag tag) {
    switch (tag) {
    case Annotations::LeftEye: return leftEye->getFilter();
    case Annotations::RightEye: return rightEye->getFilter();
    case Annotations::Nose: return nose->getFilter();
    default:
      string err = "Classifier::getFilter. Unknown tag";
      throw (err);
    }
  }

 private:
  void readParameters();
  void normalize(vector<double>& data);
};

#endif
