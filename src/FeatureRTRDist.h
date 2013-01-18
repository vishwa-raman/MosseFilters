#ifndef __FEATURERTRDIST_H
#define __FEATURERTRDIST_H

// FeatureRTRDist.h
// The distance between the right top corner and the right eye location feature

#include "Feature.h"

class FeatureRTRDist : public Feature {
 public:
  FeatureRTRDist() : Feature(Feature::RTRDist) {
    minVal = FLT_MAX;
    maxVal = FLT_MIN;
  }
  virtual ~FeatureRTRDist() {
  }

  // the extract method is expected to be implemented in each derived class
  virtual double extract(FrameAnnotation* annotation) { 
    CvPoint& pointr = annotation->getRightIris();
    CvPoint rt;
    rt.x = Globals::imgWidth;
    rt.y = 0;

    double xdiff = rt.x - pointr.x;
    double ydiff = rt.y - pointr.y;

    double result = sqrt(xdiff * xdiff + ydiff * ydiff);
    if (minVal > result)
      minVal = result;
    if (maxVal < result)
      maxVal = result;
    return result;
  }
};

#endif
