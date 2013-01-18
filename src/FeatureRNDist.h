#ifndef __FEATURERNDIST_H
#define __FEATURERNDIST_H

// FeatureRNDIST.h
// The distance between the right eye and the nose

#include "Feature.h"

class FeatureRNDist : public Feature {
 public:
  FeatureRNDist() : Feature(Feature::RNDist) {
    minVal = FLT_MAX;
    maxVal = FLT_MIN;
  }
  virtual ~FeatureRNDist() {
  }

  // the extract method is expected to be implemented in each derived class
  virtual double extract(FrameAnnotation* annotation) { 
    CvPoint& pointr = annotation->getRightIris();
    CvPoint& pointn = annotation->getNose();

    double xdiff = pointr.x - pointn.x;
    double ydiff = pointr.y - pointn.y;

    double result = sqrt(xdiff * xdiff + ydiff * ydiff);
    if (minVal > result)
      minVal = result;
    if (maxVal < result)
      maxVal = result;
    return result;
  }
};

#endif
