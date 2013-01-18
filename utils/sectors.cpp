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

typedef struct {
  int frameN;

  CvPoint left;
  CvPoint right;
  CvPoint nose;
  int zone;

} LocationsT;

typedef enum {
  Left = 0,
  Right = 1,
  Nose = 2,
  Zone = 3
} LocationT;

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
      "Usage. annot <framesDirectory> <output> <nthframe> [startFrame] [nframesoptional]" << endl;
    return 0;
  }

  DIR* dir;
  struct dirent *ent;

  string inputDirectory = argv[1];
  string outputFileName = argv[2];
  int nthFrame = atoi(argv[3]);
  int startFrame = 0;
  bool mark = false;
  vector<int> frameNumbers;

  int zone = -1;
  if (argc > 4) {
    if (strcmp(argv[4], "-mark"))
      startFrame = atoi(argv[4]);
    else {
      zone = atoi(argv[5]);
      mark = true;

      // now read all the frame numbers in the frames directory into a vector
      dir = opendir(inputDirectory.c_str());
      if (dir != NULL) {
	/* print all the files and directories within directory */
	while ((ent = readdir (dir)) != NULL) {
	  string fileName = ent->d_name;
	  char* str = (char*)fileName.c_str();
	  if (fileName.find("frame") != string::npos) {
	    int frameNumber;
	    sscanf(str, "frame_%d.png", &frameNumber);
	    frameNumbers.push_back(frameNumber);
	  }
	}
	closedir(dir);
	startFrame = 0;
	nthFrame = 1;
      }
    }
  }

  cvNamedWindow((const char*)windowName.c_str(), CV_WINDOW_NORMAL | CV_WINDOW_AUTOSIZE);

  for (int i = 0; ; i += nthFrame) {
    if (i > startFrame) {
      // get every nthframe

      string fileName = inputDirectory + "/frame_";
      char buffer[32];
      if (mark)
	sprintf(buffer, "%d.png", frameNumbers[i]);
      else
	sprintf(buffer, "%d.png", i);
      string str = buffer;
      fileName += str;

      image = cvLoadImage((const char*)fileName.c_str());
      if (!image)
	break;

      if (!mark) {
	// show the image in a window
	cvShowImage((const char*)windowName.c_str(), image);
	zone = cvWaitKey(0) - '0';
	if (!zone)
	  continue;
	if (zone == 9)
	  break;
      }

      // the globals left, right and nose now have the locations of interest
      LocationsT* location = new LocationsT;

      location->frameN = (mark)? frameNumbers[i] : i;
      location->left.x = 0;
      location->left.y = 0;
      location->right.x = 0;
      location->right.y = 0;
      location->nose.x = 0;
      location->nose.y = 0;
      location->zone = zone;
      
      locations.push_back(location);
    }
  }

  // check if the input directory exists, or else bail
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
