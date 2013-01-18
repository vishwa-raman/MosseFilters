#ifndef __FEATURERX_H
#define __FEATURERX_H

// FeatureRX.h
// The right eye location feature

#include "Feature.h"

class FeatureRX : public Feature {
 public:
  FeatureRX() : Feature(Feature::RX) {
    minVal = FLT_MAX;
    maxVal = FLT_MIN;
  }
  virtual ~FeatureRX() {
  }

  // the extract method is expected to be implemented in each derived class
  virtual double extract(FrameAnnotation* annotation) { 
    double result = annotation->getRightIris().x; 
    if (minVal > result)
      minVal = result;
    if (maxVal < result)
      maxVal = result;
    return result;
  }
};

#endif
