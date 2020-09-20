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

#include <ueye.h>
#include <iostream>

#include "blastpit.h"
#include "mongoose.h"

using namespace std;

HIDS	   hCam = 0;  // 0 for the next available camera. 1-254 to access by ID
SENSORINFO sInfo;
HWND	   hWndDisplay;

int   nMemoryId;
char *pcImageMemory;
int   DisplayWidth, DisplayHeight;

int should_exit = false;

// Base 64 Encoding (https://nachtimwald.com/2017/11/18/base64-encode-and-decode-in-c/)
size_t
b64_encoded_size(size_t inlen)
{
	size_t ret;

	ret = inlen;
	if (inlen % 3 != 0)
		ret += 3 - (inlen % 3);
	ret /= 3;
	ret *= 4;

	return ret;
}

const char b64chars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

char *
b64_encode(const unsigned char *in, size_t len)
{
	char * out;
	size_t elen;
	size_t i;
	size_t j;
	size_t v;

	if (in == NULL || len == 0)
		return NULL;

	elen	  = b64_encoded_size(len);
	out	  = (char *)malloc(elen + 1);
	out[elen] = '\0';

	for (i = 0, j = 0; i < len; i += 3, j += 4) {
		v = in[i];
		v = i + 1 < len ? v << 8 | in[i + 1] : v << 8;
		v = i + 2 < len ? v << 8 | in[i + 2] : v << 8;

		out[j]	   = b64chars[(v >> 18) & 0x3F];
		out[j + 1] = b64chars[(v >> 12) & 0x3F];
		if (i + 1 < len) {
			out[j + 2] = b64chars[(v >> 6) & 0x3F];
		} else {
			out[j + 2] = '=';
		}
		if (i + 2 < len) {
			out[j + 3] = b64chars[v & 0x3F];
		} else {
			out[j + 3] = '=';
		}
	}

	return out;
}

void
snapshot()
{
	// Acquires a single image from the camera
	int retval = is_FreezeVideo(hCam, IS_WAIT);
	if (retval == IS_TRANSFER_ERROR) {
		cout << "Transfer error in is_FreezeVideo" << endl;
		return;
	}
	if (retval != IS_SUCCESS) {
		cout << "Error capturing video (is_FreezeVideo)" << endl;
		cout << "Return value: " << retval << endl;
		return;
	}

	// Parameter definition for saving the image file
	IMAGE_FILE_PARAMS ImageFileParams;
	ImageFileParams.pwchFileName = L"/tmp/ueye_capture.png";  /// <-- Insert name and location of the image
	ImageFileParams.pnImageID    = NULL;
	ImageFileParams.ppcImageMem  = NULL;
	ImageFileParams.nQuality     = 100;
	ImageFileParams.nFileType    = IS_IMG_PNG;

	// Saves the image file
	if (is_ImageFile(hCam, IS_IMAGE_FILE_CMD_SAVE, (void *)&ImageFileParams, sizeof(ImageFileParams)) ==
	    IS_SUCCESS) {
		cout << "An Image was saved" << endl;
		// should_exit = true;
	} else {
		cout << "something went wrong" << endl;
	}
}

void
messageReceivedCallback(void *ev_data, void *object)
{
	(void)object;

	struct websocket_message *wm = (struct websocket_message *)ev_data;

	// If message = kPhoto get photo and send
	(void)wm;
	(void)ev_data;
	cout << "ueye_simple: messageReceivedCallback called" << endl;
	snapshot();

	// Convert to PNG

	// Encode to base64
	char *encoded_image = b64_encode((const unsigned char *)pcImageMemory, (size_t)1280 * 1024 * 3);

	// Send as XML

	// Cleanup
	free(encoded_image);
}

int
main()
{
	const char *server = "ws://192.168.1.40:8000";

	t_Blastpit *client = blastpitNew();
	registerCallback(client, &messageReceivedCallback);
	int result = connectToServer(client, server, 1000);
	if (result == kConnectionFailure) {
		fprintf(stderr, "Could not connect to server.\n");
		exit(1);
	}

	// Starts the driver and establishes the connection to the camera
	is_InitCamera(&hCam, NULL);

	UINT nRange[3];
	ZeroMemory(nRange, sizeof(nRange));
	// Get pixel clock range
	INT nRet = is_PixelClock(hCam, IS_PIXELCLOCK_CMD_GET_RANGE, (void *)nRange, sizeof(nRange));
	if (nRet == IS_SUCCESS) {
		UINT nMin = nRange[0];
		UINT nMax = nRange[1];
		UINT nInc = nRange[2];
		cout << "Pixel clock min: " << nMin << endl;
		cout << "Pixel clock max: " << nMax << endl;
	}

	UINT nPixelClockDefault;
	nRet = is_PixelClock(hCam, IS_PIXELCLOCK_CMD_GET_DEFAULT, (void *)&nPixelClockDefault,
			     sizeof(nPixelClockDefault));

	if (nRet == IS_SUCCESS) {
		// Set this pixel clock
		cout << "Setting pixel clock" << endl;
		UINT nPixelClock = 7;
		nRet = is_PixelClock(hCam, IS_PIXELCLOCK_CMD_SET, (void *)&nPixelClock, sizeof(nPixelClock));
		if (nRet != IS_SUCCESS)
			cout << "Setting clock failed with retval: " << nRet;
	} else {
		cout << "Error getting pixel clock data" << endl;
	}

	// You can query information about the sensor type used in the camera
	is_GetSensorInfo(hCam, &sInfo);

	is_SetHardwareGain(hCam, 100, IS_IGNORE_PARAMETER, IS_IGNORE_PARAMETER, IS_IGNORE_PARAMETER);
	// is_SetHWGainFactor(hCam, IS_SET_MASTER_GAIN_FACTOR, 100);

	// Saving the information about the max. image proportions in variables
	DisplayWidth  = sInfo.nMaxWidth;
	DisplayHeight = sInfo.nMaxHeight;

	// Need to find out the memory size of the pixel and the colour mode
	int nColorMode;
	int nBitsPerPixel = 24;

	nColorMode = IS_CM_BGR8_PACKED;

	IS_RECT rectAOI;

	rectAOI.s32X	  = 500;
	rectAOI.s32Y	  = 400;
	rectAOI.s32Width  = 320;
	rectAOI.s32Height = 240;

	nRet = is_AOI(hCam, IS_AOI_IMAGE_SET_AOI, (void *)&rectAOI, sizeof(rectAOI));

	// Assigns a memory for the image and sets it active
	int retval = is_AllocImageMem(hCam, 1280, 1024, nBitsPerPixel, &pcImageMemory, &nMemoryId);
	if (retval != IS_SUCCESS) {
		cout << "Error allocating image memory" << endl;
		exit(1);
	}

	is_SetImageMem(hCam, pcImageMemory, nMemoryId);

	// Event loop
	while (!should_exit) {
		pollMessages(client);
		usleep(100000);
	}

	// Releases an image memory that was allocated
	is_FreeImageMem(hCam, pcImageMemory, nMemoryId);

	// Disables the hCam camera handle and releases the data structures and memory areas taken up by the uEye camera
	is_ExitCamera(hCam);

	return 0;
}
