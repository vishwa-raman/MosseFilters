// annotate.cpp
// This file contains the code for annotation. We take as input the name of a video
// file, the name of an output file for annotations and the distance in number of 
// frames between frames we want annotated. We then present frames and expect the
// users to click on locations of interest. For now we are a bit clunky and 
// expect users to click on the left iris, then the right iris and finally the tip of
// the nose

#include "stdio.h"
#include "stdlib.h"
#include "limits.h"
#include "float.h"
#include <sys/types.h>
#include <dirent.h>
#include <unistd.h>
#include <iostream>
#include <fstream>

// The standard OpenCV headers
#include <cv.h>
#include <highgui.h>
#include <opencv2/opencv.hpp>

using namespace std;

IplImage* img0 = 0;
IplImage* img1 = 0;

static CvPoint s_face;
static CvPoint s_left;
static CvPoint s_right;
static CvPoint s_nose;

typedef struct {
  int frameN;

  CvPoint face;
  CvPoint left;
  CvPoint right;
  CvPoint nose;
  int zone;

} LocationsT;

typedef enum {
  Face = 0,
  Left = 1,
  Right = 2,
  Nose = 3,
  Zone = 4
} LocationT;

static bool s_states[4] = {false, false, false, false};

void mouseHandler(int event, int x, int y, int flags, void* param)
{
  switch(event) {
    /* left button down */
  case CV_EVENT_LBUTTONDOWN:
    fprintf(stdout, "Left button down (%d, %d).\n", x, y);
    if (!s_states[Face]) {
      s_face.x = x; 
      s_face.y = y;
      s_states[Face] = true;
    } else if (!s_states[Left]) {
      s_left.x = x; 
      s_left.y = y;
      s_states[Left] = true;
    } else if (!s_states[Right]) {
      s_right.x = x;
      s_right.y = y;
      s_states[Right] = true;
    } else if (!s_states[Nose]) {
      s_nose.x = x;
      s_nose.y = y;
      s_states[Nose] = true;
    }
    break;
       
    /* right button down */
  case CV_EVENT_RBUTTONDOWN:
    fprintf(stdout, "Right button down. Resetting all locations");
    s_states[Face] = s_states[Left] = s_states[Right] = s_states[Nose] = false;
    break;
       
    /* mouse move */
  case CV_EVENT_MOUSEMOVE:
    /* draw a rectangle */
    if (img1)
      cvReleaseImage(&img1);
    img1 = cvCloneImage(img0);
    cvCircle(img1, cvPoint(x, y), 5, cvScalar(255, 255, 255, 0), 2, 8, 0);
    cvShowImage("image", img1);
    break;
  }
}

// The main function
int main(int argc, char* argv[])
{
  IplImage* image;

  static string windowName = "annotate";

  vector<LocationsT*> locations;

  if (argc > 1 && !strcmp(argv[1], "-help")) {
    cout << 
      "Usage. annot <video> -save <frameFileNamePrefix>" << endl;
    cout << "or.." << endl;
    cout << 
      "Usage. annotate <framesDirectory> <output> <nthframe> [startFrame] [nframesoptional]" << endl;
    return 0;
  }

  string inputDirectory = argv[1];
  string outputFileName = argv[2];
  int nthFrame = atoi(argv[3]);
  unsigned int startFrame = 0;
  unsigned int nFrames = INT_MAX;

  if (argc > 4)
    startFrame = atoi(argv[4]);
  if (argc > 5)
    nFrames = atoi(argv[5]);

  cvNamedWindow((const char*)windowName.c_str(), CV_WINDOW_NORMAL | CV_WINDOW_AUTOSIZE);
  cvNamedWindow("image", CV_WINDOW_NORMAL | CV_WINDOW_AUTOSIZE);
  cvSetMouseCallback((const char*)windowName.c_str(), mouseHandler, NULL);

  for (unsigned int i = 0; locations.size() < nFrames; i += nthFrame) {
    if (i > startFrame) {
      // get every nthframe

      string fileName = inputDirectory + "/frame_";
      char buffer[32];
      sprintf(buffer, "%d.png", i);
      string str = buffer;
      fileName += str;

      image = cvLoadImage((const char*)fileName.c_str());
      if (!image)
	break;

      img0 = image;

      // show the image in a window
      cvShowImage((const char*)windowName.c_str(), image);
      int zone = cvWaitKey(0) - '0';

      if (s_states[Face] && s_states[Left] && s_states[Right] && s_states[Nose]) {    
	// the globals left, right and nose now have the locations of interest
	LocationsT* location = new LocationsT;

	location->frameN = i;
	location->face.x = s_face.x;
	location->face.y = s_face.y;
	location->left.x = s_left.x;
	location->left.y = s_left.y;
	location->right.x = s_right.x;
	location->right.y = s_right.y;
	location->nose.x = s_nose.x;
	location->nose.y = s_nose.y;
	location->zone = zone;

	locations.push_back(location);
      }
      s_states[Face] = s_states[Left] = s_states[Right] = s_states[Nose] = false;
    }
  }

  // check if the input directory exists, or else bail
  DIR* dir;
  dir = opendir(inputDirectory.c_str());
  if (dir == NULL) {
    string err = "The directory " + inputDirectory + " does not exist. Bailing out.";
    throw (err);
  }
  closedir(dir);

  // get absolute path of the input directory
  char fullPath[PATH_MAX + 1];
  string fullPathName = realpath((const char*)inputDirectory.c_str(), fullPath);

  ofstream file;
  file.open((const char*)outputFileName.c_str());
  if (file.good()) {
    file << "<?xml version=\"1.0\"?>" << endl;
    file << "<annotations dir=\"" + fullPathName + "\">" << endl;
    for (unsigned int i = 0; i < locations.size(); i++) {
      file << "  <frame>" << endl;
      file << "    <frameNumber>" << locations[i]->frameN << "</frameNumber>" << endl;
      file << "    <face>" << locations[i]->face.y << "," << 
	locations[i]->face.x << "</face>" << endl;
      file << "    <leftEye>" << locations[i]->left.y << "," << 
	locations[i]->left.x << "</leftEye>" << endl;
      file << "    <rightEye>" << locations[i]->right.y << "," << 
	locations[i]->right.x << "</rightEye>" << endl;
      file << "    <nose>" << locations[i]->nose.y << "," << 
	locations[i]->nose.x << "</nose>" << endl;
      file << "    <zone>" << locations[i]->zone << "</zone>" << endl;
      file << "  </frame>" << endl;
    }
    file << "</annotations>" << endl;
  }
  file.close();

  cvDestroyWindow((const char*)windowName.c_str());
}
