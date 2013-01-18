#ifndef __FEATURELRNAREA_H
#define __FEATURELRNAREA_H

// FeatureLRNArea.h
// The area of the triangle made by the left eye, right eye and nose

#include "Feature.h"

class FeatureLRNArea : public Feature {
 public:
  FeatureLRNArea() : Feature(Feature::LRNArea) {
    minVal = FLT_MAX;
    maxVal = FLT_MIN;
  }
  virtual ~FeatureLRNArea() {
  }

  // the extract method is expected to be implemented in each derived class
  virtual double extract(FrameAnnotation* annotation) { 
    CvPoint& pointl = annotation->getLeftIris();
    CvPoint& pointr = annotation->getRightIris();
    CvPoint& pointn = annotation->getNose();

    // compute distances
    double lndist = abs(pointn.y - pointl.y);
    double lrdist = abs(pointl.x - pointr.x);

    // compute area
    double result = lndist * lrdist / 2.0;
    if (pointn.x < (pointr.x + lrdist / 2))
      result = -result;
    if (minVal > result)
      minVal = result;
    if (maxVal < result)
      maxVal = result;
    return result;
  }
};

#endif
