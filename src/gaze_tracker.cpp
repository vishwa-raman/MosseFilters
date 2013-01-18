/*
 * driver_gaze.cpp
 *
 * A ROS node that performs gaze tracking using the library.
 *
 */

#include "gaze_tracker.h"

int s_imageIndex = 0;

namespace enc = sensor_msgs::image_encodings;
using namespace std;

// static instance of RosGaze that is used to process images 
// received from the USB camera at runtime. Initialized in main
static RosGaze* pRosGaze;

// the head orientation
double RosGaze::orientation;

// The following callback is called whenever a new image is
// published

void RosGaze::usbCameraCallback(const sensor_msgs::ImageConstPtr& image) {
  // convert the ROS image to an opencv image
  cv_bridge::CvImagePtr cvPtr;
  try {
    cvPtr = cv_bridge::toCvCopy(image, enc::BGR8);
  } catch (cv_bridge::Exception& e) {
    // conversion error handled
    ROS_ERROR("driver_gaze::usbCameraCallback::cv_bridge exception: %s",
	      e.what());
  }

  // Get the opencv image
  IplImage cvImage = cvPtr->image;

  // publish gaze
  pRosGaze->broadcastGaze(&cvImage);
}

// The following is the imu callback. The imu is used to get head
// orientation and hence ground truth, when we do have the magnometer
// attached to the driver's head (with bolts may I add)

void RosGaze::imuCallback(const sensor_msgs::Imu& msg) {
  double x = msg.orientation.x;
  double y = msg.orientation.y;
  double z = msg.orientation.z;
  double w = msg.orientation.w;

  tf::Quaternion q(x, y, z, w);
  double yaw, pitch, roll;
  btMatrix3x3(q.asBt()).getEulerYPR(yaw, pitch, roll);

  RosGaze::orientation = roll / M_PI * 180;
}

RosGaze::RosGaze(int argc, char** argv) {
  // data structures for smoothing the gaze. We keep track of the last
  // s_circularBufferSize sectors as determined by the gaze tracker 
  // and keep count of the number of occurences of each sector in that
  // buffer. When the number of sectors goes beyond a certain threshold
  // and it is different from the previous sector we broadcast, we 
  // broadcast the new sector
  index = 0;
  lastSector = 3;
  orientation = 0;

  for (int i = 0; i <= s_numZones; i++)
    counts[i] = 0;
  for (int i = 0; i < s_circularBufferSize; i++)
    circularBuffer[i] = 0;

  //
  // Setup ROS Publisher
  //
  ros::init(argc, argv, "driver_gaze");

  ros::NodeHandle nh("~");

  nh.getParam("model_dir", sModelDir);

  showImage = false;
  nh.getParam("show_image", showImage);

  readGroundTruth = false;
  nh.getParam("read_ground_truth", readGroundTruth);

  if (readGroundTruth)
    imuSub = nh.subscribe("/imu/data", 10, RosGaze::imuCallback);

  // create and initialize the ros broadcast message object
  odomTrans.header.frame_id = "/car";
  odomTrans.child_frame_id = "/gaze";

  odomTrans.transform.translation.x = 0;
  odomTrans.transform.translation.y = 0;
  odomTrans.transform.translation.z = 0;
  odomTrans.transform.rotation.x = 0;
  odomTrans.transform.rotation.y = 0;

  odomBroadcaster = new tf::TransformBroadcaster();

  pub = nh.advertise<driver_gaze::gaze_sector>("/driver_gaze", 10);

  // construct the sector message
  msg.value = lastSector;
  msg.confidence = 1.0;
  msg.orientation = RosGaze::orientation;

  // publish the sector
  pub.publish(msg);

  cout << "Models directory = " << sModelDir << endl;
  cout << "Show Images = " << showImage << endl;

  // Init Gaze Tracker (Model Path = sModelDir) //
  tracker = 0;
  try {
    tracker = new GazeTracker((const char*)sModelDir.c_str(), 
			      false /* no online */);
  } catch (string err) {
    cout << err << endl;
    cerr << "[GAZE >>>>>>>>>>>>>>>>>>>>>>>] " << err << endl;
    exit(0);
  }

  if (showImage)
    cvNamedWindow("window", CV_WINDOW_NORMAL | CV_WINDOW_AUTOSIZE);

  // initialize font and add text
  cvInitFont(&font, CV_FONT_HERSHEY_SIMPLEX, 1.0, 1.0, 0, 3, CV_AA);
}

RosGaze::~RosGaze() {
  delete tracker;
}

void RosGaze::broadcastGaze(IplImage* frame) {
  if (frame) {
    // Compute sector number and confidence
    IplImage* image = cvCloneImage(frame);

    //    char buffer[256];
    //    sprintf(buffer, "/home/cesar/data/frames_vishwa-08-09-12/frame_%d.png", s_imageIndex++);
    //    cvSaveImage(buffer, image);

    try {
      FrameAnnotation fa;

      double confidence;
      int sector = tracker->getZone(image, confidence, fa);

      if (showImage) {
	cvCircle(image, fa.getLOI(Annotations::LeftEye), 5, 
		 cvScalar(0, 255, 255, 0), 2, 8, 0);
	cvCircle(image, fa.getLOI(Annotations::RightEye), 5, 
		 cvScalar(255, 255, 0, 0), 2, 8, 0);
	cvCircle(image, fa.getLOI(Annotations::Nose), 5, 
		 cvScalar(255, 0, 255, 0), 2, 8, 0);
      }

      // apply smoothing
      if (counts[circularBuffer[index]])
	counts[circularBuffer[index]]--;
      counts[sector] = (counts[sector] < s_circularBufferSize)? 
	counts[sector] + 1 : counts[sector];

      // if the number of occurences of sector is beyond threshold
      // and sector is different from what we broadcast before,
      // then broadcase sector
      if (counts[sector] > s_threshold && 
	  sector != lastSector && confidence > 0) {
	cout << "Broadcasting " << sector << " (" << confidence << ")." << endl;
	lastSector = sector;
      }
	
      // construct the sector message
      msg.value = lastSector;
      msg.confidence = confidence;
      msg.orientation = RosGaze::orientation;

      // publish the sector
      pub.publish(msg);

      // construct the transform
      odomTrans.header.stamp = ros::Time::now();
      odomTrans.transform.rotation.w = -cos(s_angles[lastSector - 1] * M_PI / 360);
      odomTrans.transform.rotation.z = sin(s_angles[lastSector - 1] * M_PI / 360);

      // send the transform
      odomBroadcaster->sendTransform(odomTrans);

      circularBuffer[index] = sector;
      index = (index + 1) % s_circularBufferSize;

      if (showImage) {
	char buffer[8];
	sprintf(buffer, "%d", lastSector);
	cvPutText(image, buffer, cvPoint(580, 440), &font, 
		  cvScalar(255, 255, 255, 0));
	
	cvShowImage("window", image);
	cvWaitKey(1);
      }
      cvReleaseImage(&image);
    } catch (string err) {
      cout << err << endl;
    }
  } else {
    cout << "NULL frame!" << endl;
  }
}

// main function that simply registers the callback for the usb camera
// and spins

int main(int argc, char **argv) {
  pRosGaze = new RosGaze(argc, argv);

  ros::init(argc, argv, "image_processor");
  ros::NodeHandle nh;

  image_transport::ImageTransport it(nh);
  image_transport::Subscriber sub = it.subscribe("camera/image_raw", 1, 
						 RosGaze::usbCameraCallback);

  ros::spin();
}
