#ifndef __FEATURERTLDIST_H
#define __FEATURERTLDIST_H

// FeatureRTLDist.h
// The distance between the right top corner and the left eye

#include "Feature.h"

class FeatureRTLDist : public Feature {
 public:
  FeatureRTLDist() : Feature(Feature::RTLDist) {
    minVal = FLT_MAX;
    maxVal = FLT_MIN;
  }
  virtual ~FeatureRTLDist() {
  }

  // the extract method is expected to be implemented in each derived class
  virtual double extract(FrameAnnotation* annotation) { 
    CvPoint& pointl = annotation->getLeftIris();
    CvPoint rt;
    rt.x = Globals::imgWidth;
    rt.y = 0;

    double xdiff = pointl.x - rt.x;
    double ydiff = pointl.y - rt.y;

    double result = sqrt(xdiff * xdiff + ydiff * ydiff);
    if (minVal > result)
      minVal = result;
    if (maxVal < result)
      maxVal = result;
    return result;
  }
};

#endif
