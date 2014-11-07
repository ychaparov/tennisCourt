#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include "ballFinder.h"
#include <iostream>
#include <cstdio>
#include "utils.h"
#include <ctime>
#include "video_writer.h"

#ifdef ITC_SERVER
#include <pylon/PylonIncludes.h>
#endif

#include "frame_grabber.h"
#include "file_frame_grabber.h"
// #include "camera_frame_grabber.h"

using namespace cv;
using namespace std;

void fun() {
  FileFrameGrabber ffg("TennisCourt/TestData/cut1.avi");
  writeVideoToFile("video.mpeg", &ffg, 5);
}
int main(int argc, char *argv[]) {
  fun();
  return 0;
  FileFrameGrabber ffg("TennisCourt/TestData/cut1.avi");
  
  
  cv::Mat ff(ffg.getMat());
  // Setup output video
  cv::VideoWriter output_cap(argv[2], 
    //CV_FOURCC('F','F','V','1'),
    CV_FOURCC('H','F','Y','U'),
    // -1,
    30,
    ff.size());

  if (!output_cap.isOpened())
  {
     std::cout << "!!! Output video could not be opened" << std::endl;
       return 0;
  }
  
  
  // Loop to read from input and write to output
  cv::Mat frame;
  
  while (true)
  {       
      if (!ffg.getNextFrame(&frame))             
                break;
     std::cout << "FSZ " << frame.size() << std::endl;
                  
        output_cap.write(frame);
  }
  return 0;
  /*VideoCapture vc("S:\\video.avi");
  Mat frame;
  namedWindow("edges",1);
  while (vc.read(frame)) {
    imshow("edges", frame);
    waitKey();
  }*/
  return 0;
}

/*
  CameraFrameGrabber cfg(0);

  CGrabResultPtr ptrGrabResult;
  namedWindow("CV_Image", WINDOW_AUTOSIZE);
  Mat cv_img(cfg.getMat());

	    int frames = 0;
	    int frames2 = 0;
	    printf("GRABBING STARTED AT %d\n", time(nullptr));
	    clock_t st = clock();
	    vector<Mat> video;
      while (cfg.getNextFrame(&cv_img)) {
				  ++frames;
				  if (clock() - st > 10 * CLOCKS_PER_SEC)
					  break;
  		    imshow("CV_Image", cv_img);
          video.push_back(cv_img.clone());
		  }

	    printf("%d, %d frames grabed in 5 seconds\n", frames, frames2);
	    printf("GRABBING FINISHED AT %d\n", time(nullptr));

	    int i = 0;
	    while (true)
	    {
		    cout << "Showing frame " << i << "\n";
		    imshow("CV_Image", video[i]);
		    char c = waitKey(0);
		    if (c == 'k')
			    break;
		    else if (c == 'n' && i + 1 < (int) video.size())
			    ++i;
		    else if (c == 'p' && i > 0)
			    --i;
	    }

      return 0;
}
*/
/*    // The exit code of the sample application.
    int exitCode = 0;

    // Automagically call PylonInitialize and PylonTerminate to ensure 
    // the pylon runtime system is initialized during the lifetime of this object.
    Pylon::PylonAutoInitTerm autoInitTerm;


    CGrabResultPtr ptrGrabResult;
    namedWindow("CV_Image", WINDOW_AUTOSIZE);
    try
    {
	    DeviceInfoList_t dl;
	    CTlFactory::GetInstance().EnumerateDevices(dl);
	    cout << dl.size() << " cameras found\n";
	    CInstantCamera camera(CTlFactory::GetInstance().CreateDevice(dl[0]));
      cout << "Using device " << camera.GetDeviceInfo().GetModelName() << endl;
	    camera.Open();
	    //CInstantCamera camera2(CTlFactory::GetInstance().CreateDevice(dl[1]));
	    //cout << "Using device " << camera2.GetDeviceInfo().GetModelName() << endl;
	    //camera2.Open();

	    GenApi::CIntegerPtr width(camera.GetNodeMap().GetNode("Width"));
	    GenApi::CIntegerPtr height(camera.GetNodeMap().GetNode("Height"));
	    Mat cv_img(width->GetValue(), height->GetValue(), CV_8UC3);

	    camera.StartGrabbing();
//	    camera2.StartGrabbing();
	    CPylonImage image;
	    CImageFormatConverter fc;
	    fc.OutputPixelFormat = PixelType_RGB8packed;

	    int frames = 0;
	    int frames2 = 0;
	    printf("GRABBING STARTED AT %d\n", time(nullptr));
	    clock_t st = clock();
	    vector<Mat> video;
	    while(camera.IsGrabbing()){
		    camera.RetrieveResult( 5000, ptrGrabResult, TimeoutHandling_ThrowException);
		    if (ptrGrabResult->GrabSucceeded()){
				    ++frames;
				    if (clock() - st > 10 * CLOCKS_PER_SEC)
					    break;
				    fc.Convert(image, ptrGrabResult);

				    cv_img = cv::Mat(ptrGrabResult->GetHeight(), ptrGrabResult->GetWidth(), CV_8UC3, (uint8_t*)image.GetBuffer());
				    video.push_back(cv_img.clone());
		    }

	    }
	    printf("%d, %d frames grabed in 5 seconds\n", frames, frames2);
	    printf("GRABBING FINISHED AT %d\n", time(nullptr));

	    int i = 0;
	    while (true)
	    {
		    cout << "Showing frame " << i << "\n";
		    imshow("CV_Image", video[i]);
		    char c = waitKey(0);
		    if (c == 'k')
			    break;
		    else if (c == 'n' && i + 1 < (int) video.size())
			    ++i;
		    else if (c == 'p' && i > 0)
			    --i;
	    }
    }

    catch (GenICam::GenericException &e)
    {
	    // Error handling.
	    cerr << "An exception occurred." << endl
	    << e.GetDescription() << endl;
	    exitCode = 1;
    }

    // Comment the following two lines to disable waiting on exit
    cerr << endl << "Press Enter to exit." << endl;
    while( cin.get() != '\n');

    return exitCode;
}
*/
