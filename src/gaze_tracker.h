#ifndef __GAZE_TRACKER_H
#define __GAZE_TRACKER_H

#include <cstdlib>
#include "ros/ros.h"
#include <tf/transform_broadcaster.h>
#include <message_filters/subscriber.h>
#include <message_filters/time_synchronizer.h>
#include <image_transport/image_transport.h>
#include <cv_bridge/cv_bridge.h>
#include <sensor_msgs/image_encodings.h>
#include <opencv2/imgproc/imgproc.hpp>

#include <LinearMath/btQuaternion.h>
#include <LinearMath/btTransform.h>

#include <tf/transform_broadcaster.h>
#include <sensor_msgs/Imu.h>

#include "driver_gaze/gaze_sector.h"
#include "GazeTracker.h"

using namespace sensor_msgs;

#define s_circularBufferSize 10
#define s_numZones 5
#define s_threshold 7

// angles corresponding to each gaze sector. We take the
// vector that bisects each gaze region and take its angle
// Straight ahead is always north and has an angle of 0

static double s_angles[5] = {
  -90, -45, 0, 45, 90
};

// GAZE_Sector: 
// 1 (drivers side window)
// 2 (left)
// 3 (center)
// 4 (right)
// 5 (passengers side window)]

class RosGaze {
 public:
  // head orientation angle
  static double orientation;

  // Construction and destruction
  RosGaze(int argc, char** argv);
  virtual ~RosGaze();

  // gaze broadcaster
  void broadcastGaze(IplImage* frame);

  // the imu callback
  static void imuCallback(const sensor_msgs::Imu& msg);

  // usb camera callback
  static void usbCameraCallback(const sensor_msgs::ImageConstPtr& image);

 private:
  int index;
  int lastSector;
  int circularBuffer[s_circularBufferSize];
  int counts[s_numZones + 1];

  geometry_msgs::TransformStamped odomTrans;
  tf::TransformBroadcaster* odomBroadcaster;
  driver_gaze::gaze_sector msg;
  ros::Publisher pub;
  ros::Subscriber imuSub;

  string sModelDir;
  bool showImage;
  bool readGroundTruth;

  // the gaze tracker object
  GazeTracker* tracker;

  CvFont font;
};

#endif
