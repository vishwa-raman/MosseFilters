#ifndef __FEATURE_H
#define __FEATURE_H

// Feature.h
// The main feature extraction class. It contains data and methods that apply 
// to all features

#include "FeatureBase.h"

// The feature extraction class declaration

class Feature : public FeatureBase {
 public:
  // The set of all features we extract are defined in the following enum.
  // Feature ids are unique integers that have values as defined in this
  // type

  typedef enum {
    LX = 1,                    // x-coord of left eye
    RX,                        // x-coord of right eye
    NX,                        // x-coord of nose
    LRDist,                    // distance between the eyes
    LNDist,                    // distance between the left eye and the nose
    RNDist,                    // distance between the right eye and the nose
    LNAngle,                   // angle between the left eye and the nose
    RNAngle,                   // angle between the right eye and the nose
    LRNArea,                   // the area of the triangle made by L, R and N
    End,
  } FeatureTag;

  // short names for the features
  static string featureNames[Feature::End];

 protected:
  FeatureTag id;               // the identifier for this feature
  double minVal;               // the smallest value of a feature
  double maxVal;               // the largest value of a feature

  // helper method to calculate distances
  double distance(CvPoint& X, CvPoint& Y) {
    double xdiff = X.x - Y.x;
    double ydiff = X.y - Y.y;
    return sqrt(xdiff * xdiff + ydiff * ydiff);
  }

 public:  
  Feature(FeatureTag t) : id(t) {
    featureNames[0] = "LX";
    featureNames[1] = "RX";
    featureNames[2] = "NX";
    featureNames[3] = "LRDist";
    featureNames[4] = "RNDist";
    featureNames[5] = "LNDist";
    featureNames[6] = "LNAngle";
    featureNames[7] = "RNAngle";
    featureNames[8] = "LRNArea";
  }
  virtual ~Feature() {
  }

  // the extract method is expected to be implemented in each derived class
  virtual double extract(FrameAnnotation* annotation) { return 0; }

  // get min and max values for this feature
  double getMinVal() { return minVal; }
  double getMaxVal() { return maxVal; }

  // set min and max values for this feature
  void setMinVal(double min) { minVal = min; }
  void setMaxVal(double max) { maxVal = max; }

  // get feature id as an integer for SVM Light
  long getId() { return (long)id; }
};

#endif
