#include <iostream>
#include <vector>

#include "opencv2/imgcodecs.hpp"
#include "opencv4/opencv2/core.hpp"
#include "opencv4/opencv2/highgui.hpp"
#include "opencv4/opencv2/imgcodecs.hpp"
#include "opencv4/opencv2/imgproc.hpp"
#include "opencv4/opencv2/videoio/legacy/constants_c.h"

#include "ueye_opencv.h"

using namespace cv;

const char* windowName = "Ueye Capture";  // Name of the window

static void
CannyThreshold( int level, void* image )
{  //

	const int ratio	      = 3;
	const int kernel_size = 3;

	Mat src, src_gray;
	Mat dst, detected_edges;
	Mat conv = ( (Mat*)image )->clone();

	/// Create a greyscale copy
	Mat greyscale;
	cvtColor( conv, greyscale, COLOR_BGR2GRAY );

	dst.create( greyscale.size(), greyscale.type() );

	/// Reduce noise with a kernel 3x3
	blur( greyscale, detected_edges, Size( 3, 3 ) );

	/// Canny detector
	Canny( detected_edges, detected_edges, level, level * ratio, kernel_size );

	/// Using Canny's output as a mask, we display our result
	dst = Scalar::all( 0 );
	conv.copyTo( dst, detected_edges );

	Mat displayImg;
	cv::resize( dst, displayImg, cv::Size(), 0.5, 0.5 );
	imshow( windowName, displayImg );
}

int
UeyeToOpenCV( unsigned char* bitmap, int width, int height )
{  // Create OpenCV Mat header pointing to image data

	int	  lowThreshold	   = 0;
	const int max_lowThreshold = 100;

	Mat image;

	// If bitmap is NULL, load a sample image
	if ( !bitmap ) {
		image = imread( "/tmp/ueye_capture.jpg", cv::IMREAD_COLOR );
	} else {
		image = Mat( width, height, CV_8UC3, bitmap, width * 3 );
	}

	// std::vector *png_conversion;
	// cv::imencode("png",

	// https://docs.opencv.org/master/d3/d63/classcv_1_1Mat.html#a51615ebf17a64c968df0bf49b4de6a3a
	namedWindow( windowName );  // Create a window

	/// Create a Trackbar for user to enter threshold
	createTrackbar( "Min Threshold:", windowName, &lowThreshold, max_lowThreshold, CannyThreshold, (void*)&image );

	waitKey( 0 );

	return true;
}

void
UeyeCloseWindow()
{
	destroyWindow( windowName );  // destroy the created window
};


// To encode an opencv image to a memory buffer instead of a file, use
// imencode()
