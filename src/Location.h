#ifndef __LOCATION_H
#define __LOCATION_H

// Location.h
// This file contains the definition of the Location class, which extends the
// LocationBase class.
// We implement methods that are common to all location detection classes here.
// The workhorse methods belong here.
// Note: We abbreviate Location of Interest as LOI in the following
// The steps followed are expected to be -
// a. load filter data
// b. preprocess image data to generate post-FFT complex array
// c. compute product with a filter
// d. take inverse FFT to generate data in the real domain
// e. compose image data using inverse
// f. compute min/max values/locations (LOI as co-ordinates)

#include "Globals.h"
#include "LocationBase.h"
#include "Annotations.h"
#include "Filter.h"

class Location : public LocationBase {
 protected:
  Annotations::Tag xmlTag;  // the XML tag for this LOI extractor
  CvSize imgSize;           // the size of the input image
  IplImage* inputImg;       // the input image
  Filter* filter;           // an instance of a filter object
  fftw_complex* imageFFT;   // the preprocessed image data

  // image data after applying the filter. This is the image data that we
  // compute after a call to the method apply
  IplImage* postFilterImg;

  // past n locations for smoothing
  int pastLocationIndex;
  vector<CvPoint*> pastLocations;

  // compute PSR
  double computePSR(double max, CvPoint& location);

 public:
  // constructor that loads a filter from file
  Location(string outputDirName, Annotations::Tag tag, CvPoint& windowCenter);

  // constructor used to create location extractor with a pre-created filter
  Location(Filter* f);

  virtual ~Location();

  // main methods
  virtual void setImage(IplImage* image);
  virtual void setImage(fftw_complex* image);
  virtual fftw_complex* getPreprocessedImage(IplImage* image);
  virtual bool apply();
  virtual Filter* getFilter() {
    return filter;
  }
  virtual void setFilter(Filter* f) { 
    if (filter) delete filter;
    filter = f;
  }
  virtual void setWindowCenter(CvPoint& center) {
    if (filter)
      filter->setWindowCenter(center);
  }

  virtual double getMinValue(); 
  virtual double getMaxValue();
  virtual void getMinLocation(CvPoint& location, double& psr);
  virtual void getMaxLocation(CvPoint& location, double& psr);

  // test methods
  void printImageFFT(string filename) {
    ofstream fftFile;

    fftFile.open(filename.c_str());
    if (fftFile.good()) {
      fftFile << imgSize.height << " " << imgSize.width << endl;
      for (int i = 0; i < imgSize.height * ((imgSize.width / 2) + 1); i++) {
	fftFile << imageFFT[i][0] << " " << imageFFT[i][1] << endl;
      }
      fftFile.close();
    }
  }
};

#endif // __LOCATION_H
