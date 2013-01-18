#ifndef __FEATURELNANGLE_H
#define __FEATURELNANGLE_H

// FeatureLNANGLE.h
// The angle between the left eye and nose with respect to vertical

#include "Feature.h"

class FeatureLNAngle : public Feature {
 public:
  FeatureLNAngle() : Feature(Feature::LNAngle) {
    minVal = FLT_MAX;
    maxVal = FLT_MIN;
  }
  virtual ~FeatureLNAngle() {
  }

  // the extract method is expected to be implemented in each derived class
  virtual double extract(FrameAnnotation* annotation) { 
    CvPoint& pointl = annotation->getLeftIris();
    CvPoint& pointn = annotation->getNose();

    // compute the drop point directly below the eye that is in line with
    // the nose
    CvPoint drop;
    drop.x = pointl.x;
    drop.y = pointn.y;

    // compute distances
    double lndist = distance(pointl, pointn);
    double lddist = distance(pointl, drop);

    // compute and return angle
    double rads = acos(lddist / lndist);
    double result = (pointl.x < pointn.x)? 
      90 - ((180 / M_PI) * rads) : 90 + ((180 / M_PI) * rads);
    if (minVal > result)
      minVal = result;
    if (maxVal < result)
      maxVal = result;
    return result;
  }
};

#endif
