#ifndef __FEATURELTRDIST_H
#define __FEATURELTRDIST_H

// FeatureLTRDist.h
// The distance between the left top corner and the right eye

#include "Feature.h"

class FeatureLTRDist : public Feature {
 public:
  FeatureLTRDist() : Feature(Feature::LTRDist) {
    minVal = FLT_MAX;
    maxVal = FLT_MIN;
  }
  virtual ~FeatureLTRDist() {
  }

  // the extract method is expected to be implemented in each derived class
  virtual double extract(FrameAnnotation* annotation) { 
    CvPoint& pointr = annotation->getRightIris();

    double result = sqrt(pointr.x * pointr.x + pointr.y * pointr.y);
    if (minVal > result)
      minVal = result;
    if (maxVal < result)
      maxVal = result;
    return result;
  }
};

#endif
