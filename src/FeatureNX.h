#ifndef __FEATURENX_H
#define __FEATURENX_H

// FeatureNX.h
// The nose location feature

#include "Feature.h"

class FeatureNX : public Feature {
 public:
  FeatureNX() : Feature(Feature::NX) {
    minVal = FLT_MAX;
    maxVal = FLT_MIN;
  }
  virtual ~FeatureNX() {
  }

  // the extract method is expected to be implemented in each derived class
  virtual double extract(FrameAnnotation* annotation) { 
    double result = annotation->getNose().x; 
    if (minVal > result)
      minVal = result;
    if (maxVal < result)
      maxVal = result;
    return result;
  }
};

#endif
