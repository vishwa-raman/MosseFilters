#ifndef __FEATURELNDIST_H
#define __FEATURELNDIST_H

// FeatureLNDIST.h
// The distance between the left eye and the nose

#include "Feature.h"

class FeatureLNDist : public Feature {
 public:
  FeatureLNDist() : Feature(Feature::LNDist) {
    minVal = FLT_MAX;
    maxVal = FLT_MIN;
  }
  virtual ~FeatureLNDist() {
  }

  // the extract method is expected to be implemented in each derived class
  virtual double extract(FrameAnnotation* annotation) { 
    CvPoint& pointl = annotation->getLeftIris();
    CvPoint& pointn = annotation->getNose();

    double xdiff = pointl.x - pointn.x;
    double ydiff = pointl.y - pointn.y;

    double result = sqrt(xdiff * xdiff + ydiff * ydiff);
    if (minVal > result)
      minVal = result;
    if (maxVal < result)
      maxVal = result;
    return result;
  }
};

#endif
