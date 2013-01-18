#ifndef __FEATURELRDIST_H
#define __FEATURELRDIST_H

// FeatureLRDIST.h
// The distance between the left and right eye location feature

#include "Feature.h"

class FeatureLRDist : public Feature {
 public:
  FeatureLRDist() : Feature(Feature::LRDist) {
    minVal = FLT_MAX;
    maxVal = FLT_MIN;
  }
  virtual ~FeatureLRDist() {
  }

  // the extract method is expected to be implemented in each derived class
  virtual double extract(FrameAnnotation* annotation) { 
    CvPoint& pointl = annotation->getLeftIris();
    CvPoint& pointr = annotation->getRightIris();

    double xdiff = pointl.x - pointr.x;
    double ydiff = pointl.y - pointr.y;

    double result = sqrt(xdiff * xdiff + ydiff * ydiff);
    if (minVal > result)
      minVal = result;
    if (maxVal < result)
      maxVal = result;
    return result;
  }
};

#endif
