#ifndef __LOCATIONBASE_H
#define __LOCATIONBASE_H

// LocationBase.h
// This file contains the definition of the abstract base location class.
// Each image has a set of locations of interest. For example,
// a. The location of the center of the left eye
// b. The location of the center of the right eye
// c. The location of the left eye corner
// d. ...
// For each location, we have a filter that has been created using our
// training data.
// For each location of interest, we have an extension of this class that
// uses a particular filter trained for that location.
// Using the set of all location objects, we can extract the features we
// need to to gaze tracking.

#include <stdio.h>
#include <limits.h>
#include <float.h>
#include <iostream>
#include <algorithm>
#include <iostream>
#include <fstream>

// The standard OpenCV headers
#include <cv.h>
#include <highgui.h>

// The fftw headers
#include <fftw3.h>

#include "Globals.h"
#include "Filter.h"
#include "Annotations.h"

using namespace std;

class LocationBase {
 public:
  LocationBase() {}
  virtual ~LocationBase() {}

  virtual bool apply() = 0;
  virtual Filter* getFilter() = 0;
  virtual void setFilter(Filter* filter) = 0;

  virtual double getMinValue() = 0; 
  virtual double getMaxValue() = 0;
  virtual void getMinLocation(CvPoint& location, double& psr) = 0;
  virtual void getMaxLocation(CvPoint& location, double& psr) = 0;
};

#endif // __LOCATIONBASE_H
