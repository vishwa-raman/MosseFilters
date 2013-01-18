#ifndef __FEATURERCANGLE_H
#define __FEATURERCANGLE_H

// FeatureRCANGLE.h
// The angle between the right eye and image center with respect to vertical

#include "Feature.h"

class FeatureRCAngle : public Feature {
 public:
  FeatureRCAngle() : Feature(Feature::RCAngle) {
    minVal = FLT_MAX;
    maxVal = FLT_MIN;
  }
  virtual ~FeatureRCAngle() {
  }

  // the extract method is expected to be implemented in each derived class
  virtual double extract(FrameAnnotation* annotation) { 
    CvPoint& pointr = annotation->getRightIris();
    CvPoint center;
    center.x = Globals::imgWidth / 2;
    center.y = Globals::imgHeight / 2;

    // compute the drop point directly below the eye that is in line with
    // the nose
    CvPoint drop;
    drop.x = pointr.x;
    drop.y = center.y;

    // compute distances
    double rcdist = distance(pointr, center);
    double rddist = distance(pointr, drop);

    // compute and return angle
    double rads = acos(rddist / rcdist);
    double result = (pointr.x < center.x)? 
      90 - ((180 / M_PI) * rads) : 90 + ((180 / M_PI) * rads);
    if (minVal > result)
      minVal = result;
    if (maxVal < result)
      maxVal = result;
    return result;
  }
};

#endif
