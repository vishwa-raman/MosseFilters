#ifndef __FEATURELCANGLE_H
#define __FEATURELCANGLE_H

// FeatureLCANGLE.h
// The angle between the left eye and image center with respect to vertical

#include "Feature.h"

class FeatureLCAngle : public Feature {
 public:
  FeatureLCAngle() : Feature(Feature::LCAngle) {
    minVal = FLT_MAX;
    maxVal = FLT_MIN;
  }
  virtual ~FeatureLCAngle() {
  }

  // the extract method is expected to be implemented in each derived class
  virtual double extract(FrameAnnotation* annotation) { 
    CvPoint& pointl = annotation->getLeftIris();
    CvPoint center;
    center.x = Globals::imgWidth / 2;
    center.y = Globals::imgHeight / 2;

    // compute the drop point directly below the eye that is in line with
    // the nose
    CvPoint drop;
    drop.x = pointl.x;
    drop.y = center.y;

    // compute distances
    double lcdist = distance(pointl, center);
    double lddist = distance(pointl, drop);

    // compute and return angle
    double rads = acos(lddist / lcdist);
    double result = (pointl.x < center.x)? 
      90 - ((180 / M_PI) * rads) : 90 + ((180 / M_PI) * rads);
    if (minVal > result)
      minVal = result;
    if (maxVal < result)
      maxVal = result;
    return result;
  }
};

#endif
