#ifndef BALL_FINDER_H__
#define BALL_FINDER_H__
#include <opencv2/core/core.hpp>
#include <opencv2/video/background_segm.hpp>
#include <vector>

struct ballCandidate {
  cv::Point2f lastPosition;
  double xDiff;
  double yDiff;
};

struct person {
  cv::Point2f feet[2];
  cv::Point2f head;
  cv::Point2f hands[2]; 
};

struct object {
  cv::Point2f top, bottom, right, left;
};

// Might be refactored into general object detector. Or some
// common code from ball & person detector.
class BallFinder {
public:
  BallFinder() : frameDifference(-1), playersConsistency(0) {}
  bool addFrame(const cv::Mat &frame, cv::Point2f &ballpos, std::vector<object> &players);
  // just for testing
  int mymain(int argc, char *argv[]);

private:
  void updateFrames(const cv::Mat &frame);
  void updatesetsOfIsolatedPoints(std::vector<cv::Point2f> isolatedPts);
  bool oppositeSigns(double a, double b);
  bool isSimilar(double a, double b);
  bool matchesCurrentPath(struct ballCandidate candidate, cv::Point2f point, int frameDifference);
  cv::Point2f findCurrentPosition(ballCandidate candidate, int frameDifference);
  int lowDistanceBetweenContours(int i, int j);
  std::vector< std::vector<cv::Point> > getRepresentatives();
  void printContour(std::vector<cv::Point> contour);
  // we will see if this works
  std::vector<ballCandidate> findBallCandidates(cv::Point2f p);
  bool hasRightTrajectory(cv::Point2f p);
  ballCandidate recoverBallCandidate(cv::Point2f p);
  std::vector<cv::Point2f> getCentres();
  std::vector<cv::Point2f> getIsolatedPoints();
  void updateCurrentPosition(ballCandidate *candidate, cv::Point2f currentPosition, int frameDifference);
  bool findBall(cv::Point2f &ballpos);
  void findPlayers(std::vector<object> &players);
  bool updatePlayerCandidate(object &candidate);
  std::vector<object> findPlayerCandidates();
  bool isClose(object&, cv::Point2f&);
  bool isInside(object&, cv::Point2f&);
  bool isNearby(object&, cv::Point2f&);
  void updateObject(object&, cv::Point2f&);

  std::vector<cv::Mat> frames; // will contain last 'numberOfFrames' frames
  std::vector< std::vector<cv::Point2f> > setsOfIsolatedPoints;
  std::vector< std::vector<cv::Point> > contours;
  std::vector< std::vector<cv::Point> > representatives;
  std::vector<cv::Point2f> centres;
  static const int numberOfFrames = 4; // must be at least 2 (recommended at least 3)
  static const int maxCoordinateDiff = 40;
  static const int maxDistanceBetweenFrames = 50;
  static const int minDistanceBetweenFrames = 2;
  static const int playersConsistencyThreshold = 4;
  static const int isolationThreshold = 35;
  static const int representativeFrequency = 15;
  static const int ballContourSizeThreshold = 40;
  static const int minPlayerSize = 10;
  std::vector<struct ballCandidate> ballCandidates;
  std::vector<struct object> playerCandidates;

  int playersConsistency;
  cv::Mat fgMask; //fg mask generated by MOG2 method
  cv::BackgroundSubtractorMOG2 pMOG2; //MOG2 Background subtractor
  int frameDifference;
  cv::Mat frame;
};

std::vector<cv::Vec3f> getCircles(cv::Mat img);
bool getCirclesVerify(cv::Mat img);
bool getCirclesTest(cv::Mat *img);

#endif
