#ifndef __FEATURERNANGLE_H
#define __FEATURERNANGLE_H

// FeatureRNAngle.h
// The angle between the right eye and nose with respect to vertical

#include "Feature.h"

class FeatureRNAngle : public Feature {
 public:
  FeatureRNAngle() : Feature(Feature::RNAngle) {
    minVal = FLT_MAX;
    maxVal = FLT_MIN;
  }
  virtual ~FeatureRNAngle() {
  }

  // the extract method is expected to be implemented in each derived class
  virtual double extract(FrameAnnotation* annotation) { 
    CvPoint& pointr = annotation->getRightIris();
    CvPoint& pointn = annotation->getNose();

    // compute the drop point directly below the eye that is in line with
    // the nose
    CvPoint drop;
    drop.x = pointr.x;
    drop.y = pointn.y;

    // compute distances
    double rndist = distance(pointr, pointn);
    double rddist = distance(pointr, drop);

    // compute and return angle
    double rads = acos(rddist / rndist);
    double result = (pointr.x < pointn.x)? 
      90 - ((180 / M_PI) * rads) : 90 + ((180 / M_PI) * rads);
    if (minVal > result)
      minVal = result;
    if (maxVal < result)
      maxVal = result;
    return result;
  }
};

#endif
