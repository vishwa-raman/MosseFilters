#ifndef __ONLINEFILTER_H
#define __ONLINEFILTER_H

// OnlineFilter.h
// This file contains the definition of the OnlineFilter class, which extends the
// Filter class. In the online filter, as opposed to the base Filter class which 
// is an offline filter, we build a perfect filter for a given starting image
// and then update it as we track the LOIs. For each new image, the update is a 
// convex combination of the filter computed for previous images and the one
// computed for the new image. The implementation is based on Bolme 2010.

#include "Filter.h"

class OnlineFilter : public Filter {
 private:
  double learningRate;              // the learning rate for the online update
  FrameAnnotation fa;               // a frame annotation object

 public:
  // Constructor used to construct a filter
  OnlineFilter(string outputDirectory, Annotations::Tag xmlTag, CvSize size, 
	       double gaussianSpread, double learningRate, 
	       CvPoint& windowCenter);

  virtual ~OnlineFilter();

  // method to apply a filter to an image array
  virtual IplImage* apply(fftw_complex* imageFFT);

 protected:
  void onlineUpdate(fftw_complex* imageFFT, CvPoint& location);
};

#endif // __ONLINEFILTER_H
