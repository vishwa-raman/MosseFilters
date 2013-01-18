// OnlineFilter.cpp
// This file contains the implementation of class OnlineFilter. This is an extension
// of the Filter class and is used to build online filters that automatically update
// the filter during each apply operation. We use this to do online tracking of the
// LOIs.

#include "OnlineFilter.h"

// Class construction and destruction

OnlineFilter::OnlineFilter(string output, Annotations::Tag tag, CvSize size, 
			   double spread, double rate, CvPoint& windowCenter)
  : Filter(output, tag, windowCenter), learningRate(rate) {
  gaussianSpread = spread;
}

OnlineFilter::~OnlineFilter() {
}

// onlineUpdate
// Method used to update terms used to create a filter from an image file
// and a location of interest. This method differs from the one in the base
// Filter class in that here we use the LOIs as identified by the filter we
// have built using past images to find LOIs in a fresh image. These LOIs are
// then used to update the filter for the next fresh image.

void OnlineFilter::onlineUpdate(fftw_complex* fftImage, CvPoint& location) {
  // create a gaussian around the location centered at the location
  CvSize sizeOfGaussian = {gaussianSpread, gaussianSpread}; // size and sd as per the MATLAB code
  double sd = gaussianSpread / 2.0;
  fftw_complex* gaussian = createGaussian(location, sizeOfGaussian, sd);

  // find the complex conjugate of fftImage
  int nElements = imgSize.height * ((imgSize.width / 2) + 1);
  fftw_complex* fftImageC = getBuffer();
  for (int i = 0; i < nElements; i++) {
    fftImageC[i][0] = fftImage[i][0];
    fftImageC[i][1] = -fftImage[i][1];
  }

  // Compute numerator and denominator terms for the filter update
  fftw_complex* num = convolve(gaussian, fftImageC);
  fftw_complex* den = convolve(fftImage, fftImageC);

  // accumulate into filter wide numerator and denominator terms
  // this is where we differ from the offline case. Here we take a
  // convex combination of the filter terms we have learned from
  // past images with the terms we have computed for this image

  for(int i = 0; i < imgSize.height; i++) {
    for(int j = 0; j < imgSize.width / 2 + 1; j++) {
      int ij = i * (imgSize.width / 2 + 1) + j;
      mosseNum[ij][0] = 
	learningRate * num[ij][0] + (1 - learningRate) * mosseNum[ij][0];
      mosseNum[ij][1] = 
	learningRate * num[ij][1] + (1 - learningRate) * mosseNum[ij][1];

      mosseDen[ij][0] = 
	learningRate * den[ij][0] + (1 - learningRate) * mosseDen[ij][0];
      mosseDen[ij][1] = 
	learningRate * den[ij][1] + (1 - learningRate) * mosseDen[ij][1];
    }
  }
}

// apply
// This method is used to apply a filter passed as input to an image
// The filter is a complex array. The image FFT is also a complex array. The
// method takes the filter and the image FFT as inputs and takes their element-wise
// product. The steps taken are,
// a. Take element-wise complex product of the fft and filter
// b. Compute inverse FFT to transform data from complex to real domain
// c. Compose an image object using the IFFT data and return it

IplImage* OnlineFilter::apply(fftw_complex* fft) {
  // first check if we have done the preprocessing step
  if (!fft) {
    string err = "OnlineFilter::apply. fft object is NULL.";
    throw (err);
  }

  // In the online case, the filter should first be created before use. The terms
  // required to compute the filter evolve as new images are processed. But the
  // filter as a function of these terms should be created before each application.
  // Defer to the base class..

  create();
  if (!filter) {
    string err = "OnlineFilter::apply. Invalid filter (NULL).";
    throw (err);
  }

  // copy the preprocessed image fft for use in the subsequent online update
  fftw_complex* fftCopy = getBuffer();
  int nElements = imgSize.height * ((imgSize.width / 2) + 1);
  memcpy(fftCopy, fft, (sizeof(fftw_complex) * nElements));

  // now take product of the fft data and the filter data
  convolve(fft, filter, fftBuffer /* output store */);

  // now get the inverse FFT and perform an online update
  IplImage* postFilterImg = computeInvFFT();

  // finally do an online update of the filter using the LOI and the
  // input image
  double min;
  double max;
  CvPoint minLoc;
  CvPoint maxLoc;
  
  // get max location
  cvMinMaxLoc(postFilterImg, &min, &max, &minLoc, &maxLoc);
  double scale = 1.0 / max;
  cvConvertScale(postFilterImg, postFilterImg, scale, 0.0);

  // set the location of the nose if this is the nose filter
  fa.setFace(maxLoc);

  // update
  onlineUpdate(fftCopy, maxLoc);

  return postFilterImg;
}
