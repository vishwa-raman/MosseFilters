#ifndef __TRAINER_H
#define __TRAINER_H

// Trainer.h
// This file contains the definition of class Trainer. We use this class to train
// a model based on a set of features that we extract using a set of feature objects.
// We use SVM-Light for training and classification. This class provides a wrapper
// around the functionality provided by SVM-Light to train our model

#include <sys/types.h>
#include <dirent.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <algorithm>
#include <iostream>
#include <fstream>

#include "Globals.h"
#include "Annotations.h"
#include "FeatureLX.h"
#include "FeatureRX.h"
#include "FeatureNX.h"
#include "FeatureLRDist.h"
#include "FeatureLNDist.h"
#include "FeatureRNDist.h"
#include "FeatureLNAngle.h"
#include "FeatureRNAngle.h"
#include "FeatureLRNArea.h"
#include "Location.h"

// openCV stuff
#include <cv.h>
#include <highgui.h>

using namespace std;

// helper type used to store data vectors and the class they belong to
// as annotated in the frame from which the data has been extracted

class DataClass {
 private:
  int zone;
  vector<double> data;
 
 public:
  DataClass(int z) : zone(z) { }
  ~DataClass() { }

  int getZone() { return zone; }
  void add(double value) { data.push_back(value); }
  vector<double>& getData() { return data; }

  string getDataSVMLight() {
    string line = "";
    for (unsigned int i = 0; i < data.size(); i++) {
      char buffer[32];
      sprintf(buffer, " %d:%f", i + 1, data[i]);
      string str = buffer;
      line += str;
    }
    return line;
  }
};

class Trainer {
 private:
  string outputDirectory;       // the output directory name for all generated data
  string svmPath;               // optional path to the SVM Light executables

  // vector of feature extraction objects
  vector<Feature*> featureExtractors;

  int nFeatures;                // the number of features

  // vector of DataClass objects used to store features and the class for
  // those features as specified in the frame annotations
  vector<DataClass*> data;

  // the number of samples to accumulate before flushing them to file
  static int nSamplesBeforeFlush;

  // locations of interest extractors
  Location* leftEye;            // the left eye location extractor
  Location* rightEye;           // the right eye location extractor
  Location* nose;               // the nose extractor

  roiFnT roiFunction;           // the ROI extractor

 public:
  enum KernelType {
    Linear = 0,
    Polynomial = 1,
    RBFunction = 2,
    Sigmoid = 3
  };

 public:
  Trainer(string output, KernelType kernelType, 
	  Location* le, Location* re, Location* n,
	  roiFnT roiFunction = 0, string svmPath = "");
  virtual ~Trainer();

  // method to add training data directories
  virtual void addTrainingSet(string trainingDataDirName);

  // method to create models
  virtual void generate();

 private:
  KernelType kernelType;        // the Kernel type for learning direct from SVM Light

  void write();
  void normalize();
  bool getLocations(IplImage* frame, FrameAnnotation& fa);
};

#endif
