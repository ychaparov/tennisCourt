#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/video/background_segm.hpp"
#include "opencv2/video/tracking.hpp"
#include <iostream>
#include <sstream>
#include <vector>
#include <ctime>

using namespace cv;
using namespace std;

void processVideo(char*);
void writeFrameNumber(Mat, VideoCapture);

//global variables  --- That's probably the state of the class. One iteration is a poke.
vector<Mat> frames; // will contain last 'numberOfFrames' frames
const int numberOfFrames = 4;
vector<vector<Point2f> > setsOfIsolatedPoints;
const int maxCoordinateDiff = 40;
const int maxDistanceBetweenFrames = 50;
const int minDistanceBetweenFrames = 5;
vector<struct ballCandidate> ballCandidates;
Point2f ballPosition;
Mat frame; //current frame
Mat fgMask; //fg mask generated by MOG2 method
BackgroundSubtractorMOG2 pMOG2; //MOG2 Background subtractor
int keyboard;

struct ballCandidate {
  Point2f lastPosition;
  int xDiff;
  int yDiff;
};

//int main(int argc, char *argv[])
//{
//  //create gui windows
//  namedwindow("frame");
//
//  processvideo(argv[1]);
//
//  //destroy gui windows
//  destroyallwindows();
//  return exit_success;
//}

void updateFrames(Mat frame) {
  frames.insert(frames.begin(), frame);
  if (frames.size() > numberOfFrames) 
    frames.pop_back();
}

void updatesetsOfIsolatedPoints(vector<Point2f> isolatedPts) {
  setsOfIsolatedPoints.insert(setsOfIsolatedPoints.begin(), isolatedPts);
  if (setsOfIsolatedPoints.size() > numberOfFrames) {
    setsOfIsolatedPoints.pop_back();
  }
}

bool oppositeSigns(double a, double b) {
  return (a > 0 && b < 0) || (a < 0 && b > 0);
}

bool isSimilar(double a, double b) {
  if (abs(a) < 4 && abs(b) < 4) return true;
  if (oppositeSigns(a, b)) return false;
  // need something smarter here for the method to work...
  return abs(b / a) < 1.35 && abs(b / a) > 0.65;
}

bool matchesCurrentPath(struct ballCandidate candidate, Point2f point, int frameDifference) {
  cout << "check if " << point << " matches.." << endl;
  Point2f diff = point - candidate.lastPosition;
  if (!isSimilar(diff.x , frameDifference * candidate.xDiff)) return false;
  if (!isSimilar(diff.y , frameDifference * candidate.yDiff)) return false;
  //candidate.xdiff = diff.x;
  //candidate.ydiff = diff.y;
  //candidate.lastPosition = point;
  cout << "found match: " << point << endl;
  return true;
}

Point2f findCurrentPosition(ballCandidate candidate, int frameDifference) {
  for (Point2f p : setsOfIsolatedPoints[0]) {
    if (matchesCurrentPath(candidate, p, frameDifference)) return p;
  }
  return Point2f(0, 0);
}

void updateCurrentPosition(ballCandidate *candidate, Point2f currentPosition, int frameDifference) {
  (*candidate).xDiff = (currentPosition.x - (*candidate).lastPosition.x) / frameDifference;
  (*candidate).yDiff = (currentPosition.y - (*candidate).lastPosition.y) / frameDifference;
  (*candidate).lastPosition = currentPosition;
}

bool hasRightTrajectory(Point2f p) {
  cout << "check point " << p << endl;
  int xdiff = 0;
  int ydiff = 0;
  Point2f cur = p;
  bool found;
  for (int i = numberOfFrames - 2; i >= 0; --i) {
    vector<Point2f> isolatedPoints = setsOfIsolatedPoints[i];
    found = false;
    for (Point2f next : isolatedPoints) {
      if (norm(cur - next) < maxDistanceBetweenFrames && norm(cur - next) > minDistanceBetweenFrames) {
        cout << next << " matched!" << endl;
        if (xdiff == 0) xdiff = next.x - cur.x;
        else {
          if (!isSimilar(next.x - cur.x, xdiff)) return false;
          xdiff = next.x - cur.x;
        }
        if (ydiff == 0) ydiff = next.y - cur.y;
        else {
          if (!isSimilar(next.y - cur.y, ydiff)) return false;
          ydiff = next.y - cur.y;
        }
        cur = next;
        found = true;
        break;
      }
    }
    if (!found) return false;
  }
  return true;
}

ballCandidate recoverBallCandidate(Point2f p) {
  Point2f cur = p;
  vector<Point2f> isolatedPoints;
  int xdiff, ydiff;
  for (int i = numberOfFrames - 2; i >= 0; --i) {
    isolatedPoints = setsOfIsolatedPoints[i];
     for (Point2f next : isolatedPoints) {
       if (norm(cur - next) < maxDistanceBetweenFrames) {
         xdiff = next.x - cur.x;
         ydiff = next.y - cur.y;
         cur = next;
         break;
       }
     }
  }
  ballCandidate candidate;
  candidate.lastPosition = cur;
  candidate.xDiff = xdiff;
  candidate.yDiff = ydiff;
  return candidate;
}

vector<Point2f> getCentres(vector<vector<Point> > contours) {
  vector<Point2f> centres;
  for (vector<Point> contour : contours) {
      // for each controur take one point 'representing that contour' 
      Point2f center(0,0);
      int numberOfPoints = contour.size();
      for (Point point : contour) {
        center.x += point.x;
        center.y += point.y;
      }
      center.x = center.x / numberOfPoints;
      center.y = center.y / numberOfPoints;
      centres.push_back(center);
      //circle(frame, center, 4, Scalar(0, 255, 0), -1, 8);
  }
  return centres;
}

vector<Point2f> getIsolatedPoints(vector<vector<Point> > contours, vector<Point2f> centres) {
  vector<Point2f> isolatedPoints;
  for (int i = 0; i < contours.size(); ++i) {
      Point2f point = centres[i];
      int contourSize = contours[i].size();
      int j = 0;
      for (; j < centres.size(); ++j) {
        if (i != j) {
          if (norm(point - centres[j]) < 100) {
            break;
          }
        }
      }
    if (j == centres.size()) {
        // point is isolated
        isolatedPoints.push_back(point);
        //circle(frame, point, 4, Scalar(0, 255, 0), -1, 8);
        //cout << contourSize << endl;
        if (point.x < frame.rows && point.y < frame.cols) {
          //Mat part(frame, Range((int) point.x - 10, (int) point.x + 10), Range((int) point.y - 10, (int) point.y + 10));
          //cout << "Mat for point " << endl << part << endl;
        }
    }
  }
  return isolatedPoints;
}

void processVideo(char* videoFilename) {
  //create the capture object
  VideoCapture capture(videoFilename);

  if(!capture.isOpened()){
    //error in opening the video input
    cerr << "Unable to open video file: " << videoFilename << endl;
    exit(EXIT_FAILURE);
  }

  vector<vector<Point> > contours;
  int frameCounter = 0;
  int i = 0;
  int frameDifference = 1;
  int start = clock();
  //read input data. ESC or 'q' for quitting
  while( (char)keyboard != 'q' && (char)keyboard != 27 ){
    if ((char) keyboard == 's') {
      // skip 10 frames
      for (int i = 0; i < 10; ++i) {
         if (capture.read(frame))
          updateFrames(frame);
      }
    }

    //read the current frame
    if(!capture.read(frame)) {
      cerr << "Unable to read next frame." << endl;
      cerr << "Exiting..." << endl;
      break;
      exit(EXIT_FAILURE);
    }
    updateFrames(frame);

    if (i < 240) {
      ++i;
      continue;
    }

    ++frameCounter;
    if (frameCounter == 3000)
      break;

    //update the background model
    pMOG2.operator()(frame, fgMask, 0.01);

    //writeFrameNumber(frame, capture);
    //get the frame number and write it on the current frame
    
    // do an opening (erosion and dilation) on the mask
    erode(fgMask, fgMask, Mat());
    dilate(fgMask, fgMask, Mat()); 

    findContours(fgMask,contours,CV_RETR_EXTERNAL,CV_CHAIN_APPROX_NONE);
    drawContours(frame,contours,-1,cv::Scalar(0,0,255),0.1);

    
    vector<Point2f> centres = getCentres(contours);

    // check for isolated points
    vector<Point2f> isolatedPts = getIsolatedPoints(contours, centres);
    updatesetsOfIsolatedPoints(isolatedPts);

    int size = ballCandidates.size();
    
    //cout << "frame diff: " << frameDifference << endl;

    // check ballCandidates...
    if (ballCandidates.size() == 1) {
      ballCandidate *candidate = &ballCandidates[0];
      //cout << "ball at position: " << (*candidate).lastPosition << endl;
      Point2f currentPosition = findCurrentPosition(*candidate, frameDifference);
      //cout << "cur pos: " << currentPosition << endl;
      if (norm(currentPosition) != 0) {
        //cout << "draw a circle " << endl;
        circle(frame, currentPosition, 4, Scalar(0, 255, 0), -1, 8);
        updateCurrentPosition(candidate, currentPosition, frameDifference);
        frameDifference = 1;
      } else if (frameDifference > 3) {
        ballCandidates.clear();
        frameDifference = 1;
      }
      else
        ++frameDifference;
      // what if couldnt retrieve ball?
    } else if (ballCandidates.size() > 1) {
      for (int i = 0; i < ballCandidates.size(); ++i) {
        ballCandidate *candidate = &ballCandidates[i];
        Point2f currentPosition = findCurrentPosition(*candidate, frameDifference);
        if (norm(currentPosition) != 0)
          updateCurrentPosition(candidate, currentPosition, frameDifference);
        // again what if not found??
      }
    } else {
      // no ball candidates so far
      if (setsOfIsolatedPoints.size() == numberOfFrames) {
        vector<Point2f> startPoints = setsOfIsolatedPoints[numberOfFrames - 1];
        for (Point2f p : startPoints) {
          if (hasRightTrajectory(p)) {
            //cout << "right trajectory found!" << endl;
            ballCandidate candidate = recoverBallCandidate(p);
            ballCandidates.push_back(candidate);
          }
        }
      }
    }

    // prepare args for a call to calcOpticalFlow
    vector<uchar> status;
    vector<float> err;
    TermCriteria termcrit(TermCriteria::COUNT|TermCriteria::EPS,20,0.03);
    Size subPixWinSize(10,10), winSize(31,31);
    vector<Point2f> nextPoints;


    int size1 = centres.size();
    int size2 = nextPoints.size();

    vector<Point2f> ballCandidates;

    //show the current frame and the fg masks
    imshow("Frame", frame);
    //imshow("Background", back);
    //imshow("Mask", fgMask);

    //get the input from the keyboard
    keyboard = waitKey( 0 );
  }

  cout << (clock() - start) / CLOCKS_PER_SEC << endl;
  cout << "frame count " << frameCounter << endl;
  //delete capture object
  capture.release();
}

void writeFrameNumber(Mat frame, VideoCapture capture) {
  stringstream ss;
  rectangle(frame, cv::Point(10, 2), cv::Point(100,20),
                  cv::Scalar(255,255,255), -1);
  ss << capture.get(CV_CAP_PROP_POS_FRAMES);
  string frameNumberString = ss.str();
  putText(frame, frameNumberString.c_str(), cv::Point(15, 15),
          FONT_HERSHEY_SIMPLEX, 0.5 , cv::Scalar(0,0,0));
}

