#ifndef __FEATURELTLDIST_H
#define __FEATURELTLDIST_H

// FeatureLTLDist.h
// The distance between the left top corner and the left eye

#include "Feature.h"

class FeatureLTLDist : public Feature {
 public:
  FeatureLTLDist() : Feature(Feature::LTLDist) {
    minVal = FLT_MAX;
    maxVal = FLT_MIN;
  }
  virtual ~FeatureLTLDist() {
  }

  // the extract method is expected to be implemented in each derived class
  virtual double extract(FrameAnnotation* annotation) { 
    CvPoint& pointl = annotation->getLeftIris();

    double result = sqrt(pointl.x * pointl.x + pointl.y * pointl.y);
    if (minVal > result)
      minVal = result;
    if (maxVal < result)
      maxVal = result;
    return result;
  }
};

#endif
