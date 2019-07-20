#include "opencv2/opencv.hpp"

using namespace cv;

int main(int, char**)
{
    VideoCapture cap(0); // open the default camera
    if(!cap.isOpened())  // check if we succeeded
        return -1;
    cap.set(CV_CAP_PROP_FRAME_WIDTH, 1920);
    cap.set(CV_CAP_PROP_FRAME_HEIGHT, 1080);

    for(;;)
    {
        Mat frame;
        cap >> frame; // get a new frame from camera
        // do any processing
        imwrite("webcam_output.png", frame);
        /* if(waitKey(30) >= 0) break;   // you can increase delay to 2 seconds here */
	break;
    }
    // the camera will be deinitialized automatically in VideoCapture destructor
    return 0;
}
