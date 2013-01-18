#ifndef __FEATURELCDIST_H
#define __FEATURELCDIST_H

// FeatureLCDIST.h
// The distance between the left eye and the image center

#include "Feature.h"

class FeatureLCDist : public Feature {
 public:
  FeatureLCDist() : Feature(Feature::LCDist) {
    minVal = FLT_MAX;
    maxVal = FLT_MIN;
  }
  virtual ~FeatureLCDist() {
  }

  // the extract method is expected to be implemented in each derived class
  virtual double extract(FrameAnnotation* annotation) { 
    CvPoint& pointl = annotation->getLeftIris();
    CvPoint center;
    center.x = Globals::imgWidth / 2;
    center.y = Globals::imgHeight / 2;

    double xdiff = pointl.x - center.x;
    double ydiff = pointl.y - center.y;

    double result = sqrt(xdiff * xdiff + ydiff * ydiff);
    if (minVal > result)
      minVal = result;
    if (maxVal < result)
      maxVal = result;
    return result;
  }
};

#endif
