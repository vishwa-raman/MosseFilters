#ifndef __ANNOTATIONS_H
#define __ANNOTATIONS_H

// Annotations.h
// This file contains the definition of class Annotations. It provides an interface
// to read and access the frames stored in an annotations file.

#include <string>
#include <vector>

#include "Globals.h"

// openCV stuff
#include <cv.h>
#include <highgui.h>

using namespace std;

// forward declaration
class FrameAnnotation;

class Annotations {
 private:
  // the directory containing the frames for a given annotations.xml file
  string framesDirectory;

  // the center of the face in zone 3 (straight ahead)
  CvPoint center;

  // the set of all annotations
  vector<FrameAnnotation*> frameAnnotations;

  // bins for annotations. We want to only return annotations that are uniformly
  // distributed across the image regions within which the LOIs occur. This 
  // ensures that the training is unbiased
  vector<vector<FrameAnnotation*>* > bins;

  // the set of annotations after binning. We will have min(bin sizes) * number of bins
  // annotations that will be placed in this vector
  vector<FrameAnnotation*> unif;

  // the min and max zone numbers
  int minZone;
  int maxZone;

  // the min and max locations of each LOI
  int minLeftEyeX;
  int maxLeftEyeX;
  int minRightEyeX;
  int maxRightEyeX;
  int minNoseX;
  int maxNoseX;

  // flag used to indicate we want binned annotations
  bool useBins;
  
 public:  
  enum Tag {
    Root,
    Frame,
    FrameNumber,
    LeftEye,
    RightEye,
    Nose,
    Zone,
    EndFrame,
    Face,
    Ignore
  };

  Annotations();
  ~Annotations();

  void readAnnotations(string& filename);
  CvPoint& getCenter() { return center; }
  string getFramesDirectory() { return framesDirectory; }
  int getNZones() { return maxZone - minZone + 1; }
  int getMinZone() { return minZone; }
  int getMaxZone() { return maxZone; }

  int getMinLeftEyeX() { return minLeftEyeX; }
  int getMaxLeftEyeX() { return maxLeftEyeX; }
  int getMinRightEyeX() { return minRightEyeX; }
  int getMaxRightEyeX() { return maxRightEyeX; }
  int getMinNoseX() { return minNoseX; }
  int getMaxNoseX() { return maxNoseX; }

  vector<FrameAnnotation*>& getFrameAnnotations() { 
    if (useBins)
      return unif;
    else
      return frameAnnotations;
  }
  void createBins(Annotations::Tag tag = Annotations::Ignore);

 private:
  Annotations::Tag getData(string str, CvPoint& point);
};

// Frame annotations class

class FrameAnnotation {
 private:
  int nFrame;
  CvPoint face;
  CvPoint leftIris;
  CvPoint rightIris;
  CvPoint nose;
  int zone;

 public:
  FrameAnnotation() {
    nFrame = 0;
    face.x = face.y = 0;
    leftIris.x = leftIris.y = rightIris.x = rightIris.y = nose.x = nose.y = 0;
    zone = 0;
  }
  FrameAnnotation(int frame, CvPoint& f, CvPoint& l, CvPoint& r, CvPoint& n, int z) {
    nFrame = frame;

    face.x = f.x; face.y = f.y;
    leftIris.x = l.x; leftIris.y = l.y;
    rightIris.x = r.x; rightIris.y = r.y;
    nose.x = n.x; nose.y = n.y;

    zone = z;
  }
  FrameAnnotation(FrameAnnotation* fa) {
    nFrame = fa->getFrameNumber();
    setFace(fa->getFace());
    setLeftIris(fa->getLeftIris());
    setRightIris(fa->getRightIris());
    setNose(fa->getNose());
    zone = fa->getZone();
  }

  int getFrameNumber() { return nFrame; }
  CvPoint& getFace() { return face; }
  CvPoint& getLeftIris() { return leftIris; }
  CvPoint& getRightIris() { return rightIris; }
  CvPoint& getNose() { return nose; }
  int getZone() { return zone; }

  CvPoint& getLOI(Annotations::Tag tag) {
    switch (tag) {
    case Annotations::Face:
      return getFace();
    case Annotations::LeftEye:
      return getLeftIris();
    case Annotations::RightEye:
      return getRightIris();
    case Annotations::Nose:
      return getNose();
    default: {
      string err = "FrameAnnotation::getLOI. Unknown tag.";
      throw(err);
    }
    }
  }

  // set functions
  void setFace(CvPoint& point) {
    face.x = point.x;
    face.y = point.y;
  }
  void setLeftIris(CvPoint& point) {
    leftIris.x = point.x;
    leftIris.y = point.y;
  }
  void setRightIris(CvPoint& point) {
    rightIris.x = point.x;
    rightIris.y = point.y;
  }
  void setNose(CvPoint& point) {
    nose.x = point.x;
    nose.y = point.y;
  }

  // print functions
  void print() {
    cout << "Face: (" << face.x << ", " << face.y << ") ";
    cout << "Left: (" << leftIris.x << ", " << leftIris.y << ") "; 
    cout << "Right: (" << rightIris.x << ", " << rightIris.y << ") "; 
    cout << "Nose: (" << nose.x << ", " << nose.y << ")" << endl; 
  }
};

#endif
