#ifndef __LEFTEYELOCATION_H
#define __LEFTEYELOCATION_H

// LeftEyeLocation.h
// This file contains the definition of the class LeftEyeLocation.
// This class is used to determine the location of the left eye given an
// appropriate filter. It extends the Location class.

#include "Location.h"
#include <iostream>
#include <fstream>

using namespace std;

class LeftEyeLocation : public Location {
 private:
  string filterName;       // the name of the left eye filter

 public:
   LeftEyeLocation(string filename) : Location(filename) {
    filterName = filename;
  }
  virtual ~LeftEyeLocation() {
  }

  void printImageBytes(IplImage* image, string filename) {
    ofstream file;

    IplImage* inputImg = cvCreateImage(imgSize, IPL_DEPTH_8U, 1);
    cvCvtColor(image, inputImg, CV_BGR2GRAY);
    unsigned char* imageData = (unsigned char*)inputImg->imageData;
    int step = inputImg->widthStep;

    unsigned char* data = (unsigned char*)inputImg->imageData;
    file.open(filename.c_str());
    if (file.good()) {
      file << imgSize.height << " " << imgSize.width << endl;
      for (int i = 0; i < imgSize.height; i++) {
	for (int j = 0; j < imgSize.width; j++) {
	  file << (int)*(imageData++) << " " << (int)data[i * imgSize.width + j] << endl;
	}
	imageData += step / sizeof(char) - imgSize.width;
      }
      file.close();
    }
  }
};

#endif
