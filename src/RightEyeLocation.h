#ifndef __RIGHTEYELOCATION_H
#define __RIGHTEYELOCATION_H

// RightEyeLocation.h
// This file contains the definition of the class RightEyeLocation.
// This class is used to determine the location of the left eye given an
// appropriate filter. It extends the Location class.

#include "Location.h"
#include <iostream>
#include <fstream>

using namespace std;

class RightEyeLocation : public Location {
 private:
  string filterName;       // the name of the left eye filter

 public:
  RightEyeLocation(string filename) : Location(filename) {
    filterName = filename;
  }
  virtual ~RightEyeLocation() {
  }
};

#endif

