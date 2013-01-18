// test.cpp
// Code that tests the various pieces of functionality we have for gaze tracking

#include <unistd.h>
#include <ios>
#include <iostream>
#include <fstream>
#include <string>

#include "GazeTracker.h"

using namespace std;

static int s_circularBufferSize = 10;
static int s_numZones = 5;
static int s_threshold = 5;

void processMemUsage(double& vm_usage, double& resident_set) {
  using std::ios_base;
  using std::ifstream;
  using std::string;

  vm_usage     = 0.0;
  resident_set = 0.0;

  // 'file' stat seems to give the most reliable results
  //
  ifstream stat_stream("/proc/self/stat",ios_base::in);

  // dummy vars for leading entries in stat that we don't care about
  //
  string pid, comm, state, ppid, pgrp, session, tty_nr;
  string tpgid, flags, minflt, cminflt, majflt, cmajflt;
  string utime, stime, cutime, cstime, priority, nice;
  string O, itrealvalue, starttime;

  // the two fields we want
  //
  unsigned long vsize;
  long rss;

  stat_stream >> pid >> comm >> state >> ppid >> pgrp >> session >> tty_nr
	      >> tpgid >> flags >> minflt >> cminflt >> majflt >> cmajflt
	      >> utime >> stime >> cutime >> cstime >> priority >> nice
	      >> O >> itrealvalue >> starttime >> vsize >> rss; // don't care about the rest

  stat_stream.close();

  // in case x86-64 is configured to use 2MB pages
  long page_size_kb = sysconf(_SC_PAGE_SIZE) / 1024; 
  vm_usage     = vsize / 1024.0;
  resident_set = rss * page_size_kb;
}


int main(int argc, char** argv) {
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

  if (argc < 3) {
    cout << 
      "Usage: test <outputDirectory> <testSetDirectory> [startFrame] [EndFrame] [Step]." << endl;
    return -1;
  }

  string outputDir = argv[1];
  string testDir = argv[2];
  bool train = false;
  if (argc > 3) {
    train = (!strcmp(argv[3], "-t"))? true : train;
  }

  GazeTracker tracker(outputDir, false /* online */);

  int initial = 500;
  int final = 12000 ;
  int step = 1;
  if (argc > 5) {
    initial = atoi(argv[4]);
    final = atoi(argv[5]);
  }
  if (argc > 6)
    step = atoi(argv[6]);

  string ianjul2 = "/home/vishwa/work/data/frames_ianjul2";
  string antoine = "/home/vishwa/work/data/frames/frames_antoine";
  string vishwa = "/home/vishwa/work/data/frames/frames_vishwa";
  string vishwa2 = "/home/vishwa/work/data/frames-aug-28";

  if (train) { 
   try {
      tracker.addFrameSet(testDir);
      tracker.addFrameSet(ianjul2);
      tracker.addFrameSet(antoine);
      tracker.addFrameSet(vishwa);
      tracker.addFrameSet(vishwa2);
      tracker.createFilters();

      tracker.addTrainingSet(testDir);
      tracker.addTrainingSet(ianjul2);
      tracker.addTrainingSet(antoine);
      tracker.addTrainingSet(vishwa);
      tracker.addTrainingSet(vishwa2);
      tracker.train();
    } catch (string err) {
      cout << err << endl;
    }
  }

//  tracker.showAnnotations();
//  return 0;
 
  cvNamedWindow("window", CV_WINDOW_NORMAL | CV_WINDOW_AUTOSIZE);
  //  cvNamedWindow("roi", CV_WINDOW_NORMAL | CV_WINDOW_AUTOSIZE);

  string prefix = testDir + "/frame_";

  // initialize font and add text
  CvFont font;
  cvInitFont(&font, CV_FONT_HERSHEY_SIMPLEX, 1.0, 1.0, 0, 3, CV_AA);

  try {
    for (int i = initial; i < final; i += step) {
      char buffer[256];
      sprintf(buffer, "%d.png", i);
      string filename = prefix + buffer;
      IplImage* image = cvLoadImage((const char*)filename.c_str());
      if (image) {
	FrameAnnotation fa;

	double confidence;
	int sector = tracker.getZone(image, confidence, fa);

	cvCircle(image, fa.getLOI(Annotations::LeftEye), 5, cvScalar(0, 255, 255, 0), 2, 8, 0);
	cvCircle(image, fa.getLOI(Annotations::RightEye), 5, cvScalar(255, 255, 0, 0), 2, 8, 0);
	cvCircle(image, fa.getLOI(Annotations::Nose), 5, cvScalar(255, 0, 255, 0), 2, 8, 0);

	//	CvPoint offset;
	//	IplImage* roi = GazeTracker::roiFunction(image, fa, offset);
	//	cvShowImage("roi", roi);
	//	cvReleaseImage(&roi);

	// apply smoothing
	if (counts[circularBuffer[index]])
	  counts[circularBuffer[index]]--;
	counts[sector] = (counts[sector] < s_circularBufferSize)?
	  counts[sector] + 1 : counts[sector];

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

	cvReleaseImage(&image);

	if (!(i % 100)) { 
	  double vm, rss;
	  processMemUsage(vm, rss);
	  cout << "VM: " << vm << "; RSS: " << rss << endl;
	}
      }
    }
  } catch (string err) {
    cout << err << endl;
  }

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

