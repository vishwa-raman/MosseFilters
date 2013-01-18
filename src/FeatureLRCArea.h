#ifndef __FEATURELRCAREA_H
#define __FEATURELRCAREA_H

// FeatureLRCArea.h
// The area of the triangle made by the left eye, right eye and the image center

#include "Feature.h"

class FeatureLRCArea : public Feature {
 public:
  FeatureLRCArea() : Feature(Feature::LRCArea) {
    minVal = FLT_MAX;
    maxVal = FLT_MIN;
  }
  virtual ~FeatureLRCArea() {
  }

  // the extract method is expected to be implemented in each derived class
  virtual double extract(FrameAnnotation* annotation) { 
    CvPoint& pointl = annotation->getLeftIris();
    CvPoint& pointr = annotation->getRightIris();
    CvPoint center;
    center.x = Globals::imgWidth / 2;
    center.y = Globals::imgHeight / 2;

    // compute distances
    double lcdist = abs(center.y - pointl.y);
    double lrdist = abs(pointl.x - pointr.x);

    // compute area
    double result = lcdist * lrdist / 2.0;
    if ((pointr.x + lrdist / 2) < center.x)
      result = -result;
    if (minVal > result)
      minVal = result;
    if (maxVal < result)
      maxVal = result;
    return result;
  }
};

#endif
