MosseFilters
============

Implementation of MOSSE filters with SVM classification for driver in-car head orientation.

## Problem statement
Given frames from a USB camera, mounted on the inside of the windshield of a car, that capture the driver, determine if the driver head orientation is in one of the following classes,

1. Driver window
2. Left of center
3. Straight ahead
4. Right of center
5. Passenger window

## Implementation
We have implemented a head orientation classifier based on the following papers,

* Visual Object Tracking using Adaptive Correlation Filters. D. S. Bolme et al., Computer Vision and Pattern Recognition. June 2010.
* Average of Synthetic Exact Filters. D. S. Bolme et al., Computer Vision and Pattern Recognition. June 2009.

*The technology is patented by the authors and hence the use of this software can only be for academic purposes.*

We have implemented MOSSE filters to detect the location of the irises and the nose in a given frame. Using these locations, we compute a feature vector that is then classified by an SVM classifier. For SVM training and classification, we use SVM-Light (http://svmlight.joachims.org).

The code is organized into two directories,

1. src, which contains the sources to build a tracking library called libtrack.a. This library contains code for both training the MOSSE filters and the SVM models, and for runtime classification. 
2. utils, which contains a number of files used to build a set of executables that use the tracking library to provide training and classification functionalities.

### Training and classification
The training data for MOSSE filters is expected to contain frames annotated with the locations of the irises, the nose and a rough face center. The annotations are expected to be in a file called annotations.xml in each training data directory. A snippet of the file is shown below,

    <?xml version="1.0"?>
    <annotations dir="/home/vishwa/work/data/frames_shadows" center="360,260">
      <frame>
        <frameNumber>5</frameNumber>
        <face>282,330</face>
        <leftEye>256,275</leftEye>
        <rightEye>250,226</rightEye>
        <nose>315,223</nose>
        <zone>4</zone>
      </frame>
      <frame>
        <frameNumber>10</frameNumber>
        <face>293,326</face>
        <leftEye>256,280</leftEye>
        <rightEye>251,231</rightEye>
        <nose>319,231</nose>
        <zone>4</zone>
      </frame>
      ...
      ...
    </annotations>

where, the directory is expected to contain frames from the video with the
following filenames *'frame_%d.png'%frameNumber*. Each video frame is a
640x480 image file with the zone indicating the class as discussed above. The locations of the points of interest form the rest of the annotation per frame.

Given these annotations, we build MOSSE filters for the irises and the nose. Using these locations as input we extract a set of features such as the distance between the irises, the area of the triangle formed by the irises and the nose etc., and use them to learn an SVM model. 

The SVM-Light trainer is used as a separate process after generating all the data sets required for training. But for runtime classification, we build the SVM-Light classifier into the final classifier executable. 

*The dependencies are OpenCV, OpenMP, and fftw.*

We use openMP based parallelism where possible to accelerate training and classification.

The code is written in C++ and has been deployed in a ROS based prototype multi-modal in-car intelligent agent for driver interactions. The head orientation is one of the modalities that is used to realize a context aware dialog manager.

For questions please send mail to: vishwa.raman@west.cmu.edu

