// capture.cpp
// Code that tests gaze tracking with video streaming

#include "GazeTracker.h"

using namespace std;
using namespace cv;

int main(int argc, char** argv) {
  if (argc < 3) {
    cout << "Usage: capture <outputDirectory> <nFrames>." << endl;
    return -1;
  }

  string outputDir = argv[1];
  int nFrames = atoi(argv[2]);

  cvNamedWindow("window", CV_WINDOW_NORMAL | CV_WINDOW_AUTOSIZE);

  CvCapture* capture = 0;
  IplImage* frame = 0;

  if (!(capture = cvCreateCameraCapture(-1))) {
    cout << "Cannot initialize camera!" << endl;
  }

  int index = 1;
  while (nFrames) {
    nFrames--;

    int grab = cvGrabFrame(capture);
    if (!grab)
      break;

    frame = cvRetrieveFrame(capture);
    frame = cvQueryFrame(capture);

    if (!frame)
      break;

    IplImage* image = cvCloneImage(frame);

    cvShowImage("window", image);
    
    char buffer[256];
    sprintf(buffer, "/frame_%d.png", index++);
    string filename = outputDir + buffer;
    cvSaveImage((const char*)filename.c_str(), image);

    cvReleaseImage(&image);
  }

  cvReleaseCapture(&capture);

  return 0;
}

#if 0
int main(int argc, char** argv) {
  Annotations annotations;

  string filename = argv[1];
  annotations.readAnnotations(filename);
  vector<FrameAnnotation*> frameAnnotations = annotations.getFrameAnnotations();
  for (int i = 0; i < frameAnnotations.size(); i++) {
    FrameAnnotation* fa = frameAnnotations[i];
    cout << "Frame = " << fa->getFrameNumber() << endl;
    cout << "  Left = (" << fa->getLeftIris().y << ", " << fa->getLeftIris().x << ")" << endl;
    cout << "  Right = (" << fa->getRightIris().y << ", " << fa->getRightIris().x << ")" << endl;
    cout << "  Nose = (" << fa->getNose().y << ", " << fa->getNose().x << ")" << endl;
  }
  return 0;
}
#endif

