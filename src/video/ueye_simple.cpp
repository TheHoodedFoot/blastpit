//===========================================================================//
//                                                                           //
//  Copyright (C) 2006 - 2018                                                //
//  IDS Imaging Development Systems GmbH                                     //
//  Dimbacher Str. 6-8                                                       //
//  D-74182 Obersulm, Germany                                                //
//                                                                           //
//  The information in this document is subject to change without notice     //
//  and should not be construed as a commitment by IDS Imaging Development   //
//  Systems GmbH. IDS Imaging Development Systems GmbH does not assume any   //
//  responsibility for any errors that may appear in this document.          //
//                                                                           //
//  This document, or source code, is provided solely as an example          //
//  of how to utilize IDS software libraries in a sample application.        //
//  IDS Imaging Development Systems GmbH does not assume any responsibility  //
//  for the use or reliability of any portion of this document or the        //
//  described software.                                                      //
//                                                                           //
//  General permission to copy or modify, but not for profit, is hereby      //
//  granted, provided that the above copyright notice is included and        //
//  reference made to the fact that reproduction privileges were granted     //
//  by IDS Imaging Development Systems GmbH.                                 //
//                                                                           //
//  IDS Imaging Development Systems GmbH cannot assume any responsibility    //
//  for the use or misuse of any portion of this software for other than     //
//  its intended diagnostic purpose in calibrating and testing IDS           //
//  manufactured cameras and software.                                       //
//                                                                           //
//===========================================================================//


/// Developer Note: I tried to keep it as simple as possible.
/// Therefore there are no functions asking for the newest driver software or freeing memory beforehand, etc.
///	The sole purpose of this program is to show one of the simplest ways to interact with an IDS camera via the uEye API.
/// (XS/XC Cameras are not supported)
/// This program was tested in Qt5 on Ubuntu 16.04 and with the IDS Software Suite 4.90

#include <ueye.h>
#include <iostream>

using namespace std;

HIDS hCam = 0;	// 0 for the next available camera. 1-254 to access by ID
SENSORINFO sInfo;
HWND hWndDisplay;

char* pcImageMemory;
int DisplayWidth, DisplayHeight;

int
main()
{
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	// Starts the driver and establishes the connection to the camera
	is_InitCamera(&hCam, hWndDisplay);

	// You can query information about the sensor type used in the camera
	is_GetSensorInfo(hCam, &sInfo);

	// Saving the information about the max. image proportions in variables
	DisplayWidth = sInfo.nMaxWidth;
	DisplayHeight = sInfo.nMaxHeight;

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	// Need to find out the memory size of the pixel and the colour mode
	int nColorMode;
	int nBitsPerPixel = 24;

	if (sInfo.nColorMode == IS_COLORMODE_BAYER) {
		// For color camera models use RGB24 mode
		nColorMode = IS_CM_BGR8_PACKED;
		nBitsPerPixel = 24;
	} else if (sInfo.nColorMode == IS_COLORMODE_CBYCRY) {
		// For CBYCRY camera models use RGB32 mode
		nColorMode = IS_CM_BGRA8_PACKED;
		nBitsPerPixel = 32;
	} else {
		// For monochrome camera models use Y8 mode
		nColorMode = IS_CM_MONO8;
		nBitsPerPixel = 24;
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	IS_RECT rectAOI;

	rectAOI.s32X = 100;
	rectAOI.s32Y = 100;
	rectAOI.s32Height = 100;
	rectAOI.s32Width = 100;

	int nRet = is_AOI(hCam, IS_AOI_IMAGE_SET_AOI, (void*)&rectAOI, sizeof(rectAOI));


	int nMemoryId;

	// Assigns a memory for the image and sets it active
	is_AllocImageMem(hCam, rectAOI.s32Width, rectAOI.s32Height, nBitsPerPixel, &pcImageMemory, &nMemoryId);
	is_SetImageMem(hCam, pcImageMemory, nMemoryId);

	// Acquires a single image from the camera
	is_FreezeVideo(hCam, IS_WAIT);

	// Parameter definition for saving the image file
	IMAGE_FILE_PARAMS ImageFileParams;
	ImageFileParams.pwchFileName = L"./TestImage.bmp";  /// <-- Insert name and location of the image
	ImageFileParams.pnImageID = NULL;
	ImageFileParams.ppcImageMem = NULL;
	ImageFileParams.nQuality = 0;
	ImageFileParams.nFileType = IS_IMG_BMP;

	// Saves the image file
	if (is_ImageFile(hCam, IS_IMAGE_FILE_CMD_SAVE, (void*)&ImageFileParams, sizeof(ImageFileParams)) == IS_SUCCESS) {
		cout << "An Image was saved" << endl;
	} else {
		cout << "something went wrong" << endl;
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	// Releases an image memory that was allocated
	is_FreeImageMem(hCam, pcImageMemory, nMemoryId);

	// Disables the hCam camera handle and releases the data structures and memory areas taken up by the uEye camera
	is_ExitCamera(hCam);

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	return 0;
}
