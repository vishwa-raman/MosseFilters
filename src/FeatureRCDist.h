#ifndef __FEATURERCDIST_H
#define __FEATURERCDIST_H

// FeatureRCDIST.h
// The distance between the right eye and the center

#include "Feature.h"

class FeatureRCDist : public Feature {
 public:
  FeatureRCDist() : Feature(Feature::RCDist) {
    minVal = FLT_MAX;
    maxVal = FLT_MIN;
  }
  virtual ~FeatureRCDist() {
  }

  // the extract method is expected to be implemented in each derived class
  virtual double extract(FrameAnnotation* annotation) { 
    CvPoint& pointr = annotation->getRightIris();
    CvPoint center;
    center.x = Globals::imgWidth / 2;
    center.y = Globals::imgHeight / 2;

    double xdiff = pointr.x - center.x;
    double ydiff = pointr.y - center.y;

    double result = sqrt(xdiff * xdiff + ydiff * ydiff);
    if (minVal > result)
      minVal = result;
    if (maxVal < result)
      maxVal = result;
    return result;
  }
};

#endif
