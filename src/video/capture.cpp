#include "opencv2/opencv.hpp"
#include "opencv2/videoio/legacy/constants_c.h"
#include <iostream>
#include <string>
#include <sys/stat.h>

using namespace cv;
using namespace std;

inline bool
exists( const std::string& name )
{
	struct stat buffer;
	return ( stat( name.c_str(), &buffer ) == 0 );
}

int
main( int, char** )
{
	// TODO: CAPTURE_DEVICE needs to be the description found in
	// /dev/v4l/by-id, which we need to convert here using
	// readlink -f to find the actual /dev/videox path used below
	VideoCapture cap( CAPTURE_DEVICE, CV_CAP_V4L );
	if ( !cap.isOpened() )	// check if we succeeded
		return -1;
	cap.set( CV_CAP_PROP_FRAME_WIDTH, CAPTURE_X_RESOLUTION );
	cap.set( CV_CAP_PROP_FRAME_HEIGHT, CAPTURE_Y_RESOLUTION );

	String windowName = "Microscope Capture";  // Name of the window
	namedWindow( windowName );		   // Create a window

	for ( ;; ) {
		Mat frame;
		cap >> frame;  // get a new frame from camera

		if ( frame.empty() ) {
			cout << "Could not open or find the image" << endl;
			cin.get();  // wait for any key press
			return -1;
		}

		Mat colourframe = frame;
		// change the color image to grayscale image
		cvtColor( frame, frame, COLOR_BGR2GRAY );

		Mat dst, hist;
		int histsize = 8;
		calcHist( &frame, 1, 0, Mat(), hist, 1, &histsize, 0 );

		long highest = histsize - 1;
		for ( int i = 0; i < histsize; i++ ) {
			if ( (long)hist.at<float>( i ) > (long)hist.at<float>( highest ) ) {
				highest = i;
			}
		}

#ifdef IGNORE_DARK_IMAGES
		if ( !highest ) {
			/* cout << "This image is too dark and would be rejected." << endl; */
		} else {
#endif
			imshow( windowName, colourframe );  // Show our image inside the created window.
			int key;
			if ( ( key = waitKey( 1 ) ) == 32 ) {
				cout << "Key: " << key << endl;

				std::string filename;

				for ( int i = 0;; i++ ) {
					filename = "/home/user/Pictures/Microscope/" CAMERA_NAME "-";
					filename += std::to_string( i );
					filename += ".jpg";
					if ( !exists( filename ) )
						break;
				}

				cout << "next file: " << filename << endl;


				imwrite( filename, colourframe );
				destroyWindow( windowName );  // destroy the created window

				return 0;
			}
			if ( key == 27 )
				return -1;
#ifdef IGNORE_DARK_IMAGES
		}
#endif
	}

	// the camera will be deinitialized automatically in VideoCapture destructor
}


/* Compile with <CC> <FILENAME> $(pkg-config opencv --libs) */
