#ifndef __FEATURELX_H
#define __FEATURELX_H

// FeatureLX.h
// The left eye location feature

#include "Feature.h"

// Class that extracts the x-coordinate of the left eye

class FeatureLX : public Feature {
 public:
  FeatureLX() : Feature(Feature::LX) {
    minVal = FLT_MAX;
    maxVal = FLT_MIN;
  }
  virtual ~FeatureLX() {
  }

  // the extract method is expected to be implemented in each derived class
  virtual double extract(FrameAnnotation* annotation) { 
    double result = annotation->getLeftIris().x; 
    if (minVal > result)
      minVal = result;
    if (maxVal < result)
      maxVal = result;
    return result;
  }
};

#endif
