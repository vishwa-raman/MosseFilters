// accuracy.cpp
// Code that computes the accuracy of filters and the classifier

#include "GazeTracker.h"

using namespace std;

int main(int argc, char** argv) {
  if (argc < 3) {
    cout << "Usage: accuracy <modelsDirectory> <trainingDirectory>" << endl;
    return -1;
  }

  string modelsDirectory = argv[1];
  string trainingDirectory = argv[2];

  try {
    GazeTracker tracker(modelsDirectory, false /* online */);

    // print out error for each filter
/*    double lError = tracker.getFilterAccuracy(trainingDirectory, 
  					      Annotations::LeftEye, Classifier::OneNorm);
    double rError = tracker.getFilterAccuracy(trainingDirectory, 
  					      Annotations::RightEye, Classifier::OneNorm);
    double nError = tracker.getFilterAccuracy(trainingDirectory, 
  					      Annotations::Nose, Classifier::OneNorm);

    cout << "Filter Error (L, R, N) = (" << lError << ", " << rError << ", " << nError << ")" << endl;
*/
    // print out the percentage of miss-classifications for the classifier
    pair<double, string> error = tracker.getClassifierAccuracy(trainingDirectory);

    cout << "Classifier Error = " << error.first << "%" << endl;
    cout << "Classifier message = " << error.second << endl;
  } catch (string err) {
    cout << err << endl;
  }

  return 0;
}
