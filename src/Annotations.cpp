// Annotations.cpp
// This file contains the implementation of class Annotations.
// This class is used to read an annotations XML doc and provide an
// iterator to the frame annotations.

#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <list>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>

#include "Annotations.h"

// Construction and desctruction

Annotations::Annotations() {
  framesDirectory = "";
  center.x = Globals::imgWidth / 2;
  center.y = Globals::imgHeight / 2;
  useBins = false;
  minZone = INT_MAX;
  maxZone = INT_MIN;

  minLeftEyeX = INT_MAX;
  maxLeftEyeX = INT_MIN;
  minRightEyeX = INT_MAX;
  maxRightEyeX = INT_MIN;
  minNoseX = INT_MAX;
  maxNoseX = INT_MIN;
}

Annotations::~Annotations()
{
  // delete annotations
  for (unsigned int i = 0; i < frameAnnotations.size(); i++)
    delete frameAnnotations[i];
}

// getData
// This function takes as input a string and returns an annotation tag 
// corresponding to the annotation. It fills the CvPoint object with
// the data read from that line

Annotations::Tag Annotations::getData(string str, CvPoint& point) {
  const char* token = strtok((char*)str.c_str(), " <>");
  if (token) {
    if (!strcmp(token, "/frame"))
      return EndFrame;
    else if (!strcmp(token, "annotations")) {
      token = strtok(NULL, " <>\"");
      if (token && !strncmp(token, "dir=", 4)) {
	token = strtok(NULL, " <>\"");
	if (!token) {
	  string err = "Annotations::getData. Malformed annotations.xml. No directory name.";
	  throw err;
	}
	framesDirectory = token;
      }
      token = strtok(NULL, " <>\"");
      if (token && !strncmp(token, "center=", 7)) {
	token = strtok(NULL, " <>\"");
	if (!token) {
	  string err = "Annotations::getData. Malformed annotations.xml. No center.";
	  throw err;
	}
	char* chP = (char*)strchr(token, ',');
	if (!chP) {
	  string err = "Annotations::getData. Malformed annotations.xml. No center.";
	  throw err;
	}
	*chP = 0;
	chP++;
	center.x = atoi(token);
	center.y = atoi(chP);
      }
      return Root;
    } else if (!strcmp(token, "frame"))
      return Frame;
    else if (!strcmp(token, "frameNumber")) {
      token = strtok(NULL, " <>");
      point.x = (token)? atoi(token) : 0;
      return FrameNumber;
    } else if (!strcmp(token, "zone")) {
      token = strtok(NULL, " <>");
      point.x = (token)? atoi(token) : 0;

      // keep track of min and max zone numbers
      if (minZone > point.x)
	minZone = point.x;
      if (maxZone < point.x)
	maxZone = point.x;

      return Zone;
    } else if (token[0] != '/') {
      string tag = token;

      token = strtok(NULL, " <>");
      const char* field = strtok((char*)token, ",");
      point.y = (field)? atoi(field) : 0;
      field = strtok(NULL, ",");
      point.x = (field)? atoi(field) : 0;

      if (tag == "leftEye") {
	if (minLeftEyeX > point.x)
	  minLeftEyeX = point.x;
	if (maxLeftEyeX < point.x)
	  maxLeftEyeX = point.x;
 	return LeftEye;
      } else if (tag == "rightEye") {
	if (minRightEyeX > point.x)
	  minRightEyeX = point.x;
	if (maxRightEyeX < point.x)
	  maxRightEyeX = point.x;
	return RightEye;
      } else if (tag == "face") {
	return Face;
      } else {
	if (minNoseX > point.x)
	  minNoseX = point.x;
	if (maxNoseX < point.x)
	  maxNoseX = point.x;
	return Nose;
      }
    }
  }
  return Ignore;
}

// readAnnotations
// The following method reads an XML file and populates the annotations vector

void Annotations::readAnnotations(string& filename) {
  ifstream file;

  file.open((const char*)filename.c_str());
  if (file.good()) {
    string line;
    int nFrame = 0;
    int zone = 0;
    CvPoint temp, face, leftEye, rightEye, nose;
    face.x = face.y = leftEye.x = leftEye.y = rightEye.x = rightEye.y = nose.x = nose.y = 0;

    getline(file, line); // ignore the first line

    while (!file.eof()) {
      getline(file, line);
      Tag tag = getData(line, temp);
      switch (tag) {
      case FrameNumber: {
	nFrame = temp.x;
	break;
      }
      case Zone: {
	zone = temp.x;
	break;
      }
      case Face: {
	face.x = temp.x;
	face.y = temp.y;
	break;
      }
      case LeftEye: {
	leftEye.x = temp.x;
	leftEye.y = temp.y;
	break;
      }
      case RightEye: {
	rightEye.x = temp.x;
	rightEye.y = temp.y;
	break;
      }
      case Nose: {
	nose.x = temp.x;
	nose.y = temp.y;
	break;
      }
      case EndFrame: {
	if (face.x && face.y) {
	  FrameAnnotation* annotation = 
	    new FrameAnnotation(nFrame, face, leftEye, rightEye, nose, zone);
	  frameAnnotations.push_back(annotation); 
	  face.x = face.y = leftEye.x = leftEye.y = rightEye.x = 
	    rightEye.y = nose.x = nose.y = 0;
	} else {
	  FrameAnnotation* annotation = 
	    new FrameAnnotation(nFrame, nose /* face */, leftEye, rightEye, nose, zone);
	  frameAnnotations.push_back(annotation); 
	}
	break;
      }
      default: {
	break;
      }
      }
    }
  }
}

// createBins
// This method is used to create bins of annotations. The method divides the distance
// in pixels between the extremal x-coordinates of the locations of interest into
// bins of width Globals::binWidth. Frame annotations are then placed in each bin such 
// that the location of interest in the frames per bin lie within the bin boundaries.
// Finally a vector of frame annotations such that there are n randomly drawn samples
// from each bin are placed in a vector of annotations for filter creation. The n is
// chosen to be the smallest bin size

void Annotations::createBins(Annotations::Tag tag) {
  // first set the useBins flag so that we return the binned annotations on
  // subsequent calls for annotations
  useBins = true;

  FrameAnnotation* fa;
  int min = INT_MAX;
  int max = INT_MIN;
  for (unsigned int i = 0; i < frameAnnotations.size(); i++) {
    fa = frameAnnotations[i];
    int loi = 0;
    switch (tag) {
    case Face:
      loi = fa->getFace().x;
      break;
    case LeftEye:
      loi = fa->getLeftIris().x;
      break;
    case RightEye:
      loi = fa->getRightIris().x;
      break;
    case Nose:
      loi = fa->getNose().x;
      break;
    default:
      loi = (fa->getLeftIris().x + fa->getRightIris().x) / 2;
      break;
    }
    if (min > loi)
      min = loi;
    if (max < loi)
      max = loi;
  }

  // min is taken to be half a bin width to the left of leftmost 
  // loi in the annotation data. Similarly for max
  min -= Globals::binWidth / 2;
  max += Globals::binWidth / 2;

  int width = max - min;
  int nBins = width / Globals::binWidth + 1;

  // create a counter for each bin and reset it
  int count[nBins];
  for (int i = 0; i < nBins; i++) {
    vector<FrameAnnotation*>* bin = new vector<FrameAnnotation*>();
    bins.push_back(bin);
    count[i] = 0;
  }

  // now iterate over all annotations and place them in their bin computed by their
  // distance from min, given our bin width. Simultaneously, compute the bin sizes
  for (unsigned int i = 0; i < frameAnnotations.size(); i++) {
    fa = frameAnnotations[i];
    int loi = (fa->getLeftIris().x + fa->getRightIris().x) / 2;
    int index = (loi - min) / Globals::binWidth;
    bins[index]->push_back(fa);
    count[index]++;
  }

  // Get the smallest bin size
  int sampleSize = INT_MAX;
  for (int i = 0; i < nBins; i++)
    if (sampleSize > count[i] && count[i])
      sampleSize = count[i];

  // We now create a set of sampleSize * nBins frame annotations in the unif
  // vector
  for (int i = 0; i < nBins; i++) {
    // for now pick the first sampleSize elements from each bin
    // a uniformly distributed pick will happen later if needed
    for (int j = 0; j < sampleSize; j++) {
      if (bins[i]->size()) {
	unif.push_back(bins[i]->back());
	bins[i]->pop_back();
      }
    }
    // Now that we are done with bin i, destroy it
    delete bins[i];
  }
}
