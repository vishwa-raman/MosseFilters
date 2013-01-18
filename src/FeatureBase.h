// FeatureBase.h
// This file contains the definition of the abstract base feature class.
// For each feature we need a set of locations in an image. We expect that
// each feature will be extracted by an extension of this class using one
// or more locations of interest from a given image

#ifndef __FEATUREBASE_H
#define __FEATUREBASE_H

#include <stdio.h>
#include <iostream>
#include <algorithm>

#include "Globals.h"
#include "Annotations.h"

// openCV stuff
#include <cv.h>
#include <highgui.h>

using namespace std;

class FeatureBase {
 public:
  FeatureBase() { }
  virtual ~FeatureBase() { }

  virtual double extract(FrameAnnotation* annotation) = 0;
};

#endif
