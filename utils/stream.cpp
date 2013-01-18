// stream.cpp
// Code that tests gaze tracking with video streaming

#include "GazeTracker.h"

using namespace std;
using namespace cv;

static int s_circularBufferSize = 10;
static int s_numZones = 5;
static int s_threshold = 5;

int main(int argc, char** argv) {
  if (argc < 2) {
    cout << "Usage: stream <outputDirectory>." << endl;
    return -1;
  }

  // data structures for smoothing the gaze. We keep track of the last
  // s_circularBufferSize sectors as determined by the gaze tracker 
  // and keep count of the number of occurences of each sector in that
  // buffer. When the number of sectors goes beyond a certain threshold
  // and it is different from the previous sector we broadcast, we 
  // broadcast the new sector

  int index = 0;
  int lastSector = -1;
  int circularBuffer[s_circularBufferSize];
  int counts[s_numZones];

  for (int i = 0; i < s_numZones; i++)
    counts[i] = 0;
  for (int i = 0; i < s_circularBufferSize; i++)
    circularBuffer[i] = 0;

  string outputDir = argv[1];
  GazeTracker tracker(outputDir, false);

  cvNamedWindow("window", CV_WINDOW_NORMAL | CV_WINDOW_AUTOSIZE);

  CvCapture* capture = 0;
  IplImage* frame = 0;

  if (!(capture = cvCreateCameraCapture(-1))) {
    cout << "Cannot initialize camera!" << endl;
  }
  // set the wanted image size from the camera
  cerr << "WIDTH=" << cvGetCaptureProperty(capture, CV_CAP_PROP_FRAME_WIDTH) << endl;
  cerr << "HEIGHT=" << cvGetCaptureProperty(capture, CV_CAP_PROP_FRAME_HEIGHT) << endl;
  cvSetCaptureProperty (capture, CV_CAP_PROP_FRAME_WIDTH, 640);
  cvSetCaptureProperty (capture, CV_CAP_PROP_FRAME_HEIGHT, 480);

  // initialize font and add text
  CvFont font;
  cvInitFont(&font, CV_FONT_HERSHEY_SIMPLEX, 1.0, 1.0, 0, 3, CV_AA);

  while (true) {
    int grab = cvGrabFrame(capture);
    if (!grab)
      break;

    frame = cvRetrieveFrame(capture);
    frame = cvQueryFrame(capture);

    if (!frame)
      break;

    IplImage* image = cvCloneImage(frame);

    cvShowImage("window", image);
    try {
      FrameAnnotation fa;

      double confidence;
      int sector = tracker.getZone(image, confidence, fa);

      cvCircle(image, fa.getLOI(Annotations::LeftEye), 5, cvScalar(0, 255, 255, 0), 2, 8, 0);
      cvCircle(image, fa.getLOI(Annotations::RightEye), 5, cvScalar(255, 255, 0, 0), 2, 8, 0);
      cvCircle(image, fa.getLOI(Annotations::Nose), 5, cvScalar(255, 0, 255, 0), 2, 8, 0);

      // apply smoothing
      if (counts[circularBuffer[index]])
	counts[circularBuffer[index]]--;
      counts[sector] = (counts[sector] < s_circularBufferSize)? counts[sector] + 1 : counts[sector];

      // if the number of occurences of sector is beyond threshold
      // and sector is different from what we broadcast before,
      // then broadcase sector
      if (counts[sector] > s_threshold && sector != lastSector && confidence > 0) {
	cout << "Broadcasting " << sector << " (" << confidence << ")." << endl;
	lastSector = sector;
      }

      circularBuffer[index] = sector;
      index = (index + 1) % s_circularBufferSize;

      char buffer[8];
      sprintf(buffer, "%d", lastSector);
      cvPutText(image, buffer, cvPoint(580, 440), &font, cvScalar(255, 255, 255, 0));

      cvShowImage("window", image);
      cvWaitKey(1);
    } catch (string err) {
      cout << err << endl;
    }

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

