//===========================================================================//
//																																					 //
//	Copyright (C) 2006 - 2018
////	IDS Imaging Development Systems GmbH
////	Dimbacher Str. 6-8
////	D-74182 Obersulm, Germany
////
//																																					 //
//	The information in this document is subject to change without notice		 //
//	and should not be construed as a commitment by IDS Imaging Development	 //
//	Systems GmbH. IDS Imaging Development Systems GmbH does not assume any	 //
//	responsibility for any errors that may appear in this document.					 //
//																																					 //
//	This document, or source code, is provided solely as an example					 //
//	of how to utilize IDS software libraries in a sample application.				 //
//	IDS Imaging Development Systems GmbH does not assume any responsibility	 //
//	for the use or reliability of any portion of this document or the				 //
//	described software.
////
//																																					 //
//	General permission to copy or modify, but not for profit, is hereby			 //
//	granted, provided that the above copyright notice is included and				 //
//	reference made to the fact that reproduction privileges were granted		 //
//	by IDS Imaging Development Systems GmbH.
////
//																																					 //
//	IDS Imaging Development Systems GmbH cannot assume any responsibility		 //
//	for the use or misuse of any portion of this software for other than		 //
//	its intended diagnostic purpose in calibrating and testing IDS					 //
//	manufactured cameras and software.
////
//																																					 //
//===========================================================================//

#include <getopt.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "blastpit.h"
#include "ueye.h"
#include "ueye_opencv.h"

#define kBITSPERPIXEL 24

// HWND		 hWndDisplay;

// int	 nMemoryId;
// char *pcImageMemory;

// int should_exit = false;

struct cameraInfo
{
	t_Blastpit* client;
	HIDS	    hCam;
	char*	    pcImageMemory;
	int	    nMemoryId;
};

int verbose_flag = 0;
int test_flag	 = 0;

void printUsage()
{
	printf( "Usage: ueye [-p pixelclock] [-f framerate] [-e exposure] <server>\n\n"
		"e.g. ueye -p 7 -e 1500 ws://127.0.0.1:8000\n" );
}

// Base 64 Encoding (https://nachtimwald.com/2017/11/18/base64-encode-and-decode-in-c/)
size_t b64_encoded_size( size_t inlen )
{
	size_t ret;

	ret = inlen;
	if ( inlen % 3 != 0 ) {
		ret += 3 - ( inlen % 3 );
	}
	ret /= 3;
	ret *= 4;

	return ret;
}


char* b64_encode( const unsigned char* in, size_t len )
{
	const char b64chars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

	char*  out;
	size_t elen;
	size_t i;
	size_t j;
	size_t v;

	if ( in == NULL || len == 0 ) {
		return NULL;
	}

	elen	  = b64_encoded_size( len );
	out	  = (char*)malloc( elen + 1 );
	out[elen] = '\0';

	for ( i = 0, j = 0; i < len; i += 3, j += 4 ) {
		v = in[i];
		v = i + 1 < len ? v << 8 | in[i + 1] : v << 8;
		v = i + 2 < len ? v << 8 | in[i + 2] : v << 8;

		out[j]	   = b64chars[( v >> 18 ) & 0x3F];
		out[j + 1] = b64chars[( v >> 12 ) & 0x3F];
		if ( i + 1 < len ) {
			out[j + 2] = b64chars[( v >> 6 ) & 0x3F];
		} else {
			out[j + 2] = '=';
		}
		if ( i + 2 < len ) {
			out[j + 3] = b64chars[v & 0x3F];
		} else {
			out[j + 3] = '=';
		}
	}

	return out;
}

void snapshot( HIDS hCam, char* bitmap )
{  // Acquires a single image from the camera

	int retval = is_FreezeVideo( hCam, IS_WAIT );
	if ( retval == IS_TRANSFER_ERROR ) {
		fprintf( stderr, "Transfer error in is_FreezeVideo\n" );
		return;
	}
	if ( retval != IS_SUCCESS ) {
		fprintf( stderr, "Error capturing video (is_FreezeVideo)\n" );
		fprintf( stderr, "Return value: \n" );
		return;
	}
}

void messageReceivedCallback( void* ev_data, void* object )
{
	(void)object;

	struct websocket_message* wm = (struct websocket_message*)ev_data;

	// If message = kPhoto get photo and send
	(void)wm;
	(void)ev_data;
	printf( "ueye_simple: messageReceivedCallback called" );

	// snapshot();

	// Convert to PNG

	// Encode to base64
	// char *encoded_image = b64_encode((const unsigned char *)pcImageMemory, (size_t)1280 * 1024 * 3);

	// Send as XML
	// QueueReplyPayload(blast, id, retval, encoded_image);
	// BpUploadQueuedMessages(blast);

	// Cleanup
	// free(encoded_image);

	// If message = kOpenCV get photo and launch opencv processing
}

void InitialiseServer( char* serverip, struct cameraInfo* camera )
{
	camera->client = blastpitNew();
	registerCallback( camera->client, &messageReceivedCallback );
	registerObject( camera->client, camera );
	int result = connectToServer( camera->client, serverip, 1000 );
	if ( result == kConnectionFailure ) {
		fprintf( stderr, "Could not connect to server at address %s.\n", serverip );
		exit( 1 );
	}
}

int InitialiseCamera( struct cameraInfo* camera, unsigned int pixelclock, double framerate, double exposure )
{
	// Starts the driver and establishes the connection to the camera
	is_InitCamera( &camera->hCam, NULL );

	// You can query information about the sensor type used in the camera
	SENSORINFO sInfo;
	is_GetSensorInfo( camera->hCam, &sInfo );

	UINT nPixelClockRange[3];

	// Get pixel clock range
	INT nRet = is_PixelClock(
		camera->hCam, IS_PIXELCLOCK_CMD_GET_RANGE, (void*)nPixelClockRange, sizeof( nPixelClockRange ) );
	if ( nRet == IS_SUCCESS ) {
		UINT nMin = nPixelClockRange[0];
		UINT nMax = nPixelClockRange[1];
		UINT nInc = nPixelClockRange[2];
		if ( verbose_flag ) {
			printf( "Pixel clock min: %d\n", nMin );
			printf( "Pixel clock max: %d\n", nMax );
		}
		if ( !pixelclock ) {
			pixelclock = nMin;
		}
	}

	// Set this pixel clock
	if ( verbose_flag ) {
		printf( "Setting pixel clock to %d\n", pixelclock );
	}
	nRet = is_PixelClock( camera->hCam, IS_PIXELCLOCK_CMD_SET, (void*)&pixelclock, sizeof( pixelclock ) );
	if ( nRet != IS_SUCCESS ) {
		printf( "Setting pixel clock failed with retval: %d\n", nRet );
		exit( 1 );
	}

	// Find the colour mode
	INT mode;
	nRet = is_SetColorMode( camera->hCam, IS_GET_COLOR_MODE );
	fprintf( stderr, "is_SetColorMode: %d\n", nRet );

	// Assigns a memory for the image and sets it active
	int retval = is_AllocImageMem( camera->hCam,
				       sInfo.nMaxWidth,
				       sInfo.nMaxHeight,
				       kBITSPERPIXEL,
				       &camera->pcImageMemory,
				       &camera->nMemoryId );
	if ( retval != IS_SUCCESS ) {
		printf( "Error allocating image memory\n" );
		exit( 1 );
	}

	is_SetImageMem( camera->hCam, camera->pcImageMemory, camera->nMemoryId );

	double frmin, frmax, frrange;
	is_GetFrameTimeRange( camera->hCam, &frmin, &frmax, &frrange );
	if ( verbose_flag ) {
		printf( "Framerate: (min) %f, (max) %f, (interval) %f\n", frmin, frmax, frrange );
	}

	if ( !framerate ) {
		framerate = frmin;
	}

	nRet = is_SetFrameRate( camera->hCam, framerate, NULL );

	double params[3];
	is_Exposure( camera->hCam, IS_EXPOSURE_CMD_GET_EXPOSURE_RANGE, &params, sizeof( params ) );
	if ( verbose_flag ) {
		printf( "Exposure range: %f - %f in %f steps\n", params[0], params[1], params[2] );
	}

	if ( !exposure ) {
		exposure = params[1];
	}

	nRet = is_Exposure( camera->hCam, IS_EXPOSURE_CMD_SET_EXPOSURE, &exposure, sizeof( exposure ) );
	nRet = is_Exposure( camera->hCam, IS_EXPOSURE_CMD_GET_EXPOSURE, &exposure, sizeof( exposure ) );
	if ( verbose_flag ) {
		fprintf( stderr, "Getting exposure is %f \n", exposure );
	}

	// Discard inital (corrupt) frame
	is_FreezeVideo( camera->hCam, IS_WAIT );

	return true;
}

int TerminateCamera( struct cameraInfo* camera )
{
	// Releases the image memory that was allocated
	is_FreeImageMem( camera->hCam, camera->pcImageMemory, camera->nMemoryId );

	// Disables the hCam camera handle and releases the data structures and memory areas taken up by the uEye camera
	is_ExitCamera( camera->hCam );

	return 0;
}

int main( int argc, char** argv )
{
	int c;

	unsigned int pixelclock = 0;
	double	     framerate	= 0;
	double	     exposure	= 0;

	while ( 1 ) {
		static struct option long_options[] = { /* These options set a flag. */
							{ "verbose", no_argument, &verbose_flag, 1 },
							{ "test", no_argument, &test_flag, 1 },
							/* These options don't set a flag.
								 We distinguish them by their indices. */
							{ "pixelclock", required_argument, 0, 'p' },
							{ "framerate", required_argument, 0, 'f' },
							{ "exposure", required_argument, 0, 'e' },
							{ 0, 0, 0, 0 }
		};
		/* getopt_long stores the option index here. */
		int option_index = 0;

		c = getopt_long( argc, argv, "p:f:e:", long_options, &option_index );

		/* Detect the end of the options. */
		if ( c == -1 ) {
			break;
		}

		switch ( c ) {
			case 0:
				/* If this option set a flag, do nothing else now. */
				if ( long_options[option_index].flag != 0 ) {
					break;
				}
				printf( "option %s", long_options[option_index].name );
				if ( optarg ) {
					printf( " with arg %s", optarg );
				}
				printf( "\n" );
				break;

			case 'p':
				pixelclock = atof( optarg );
				fprintf( stderr, "Found pixel clock: %s\n", optarg );
				break;

			case 'f':
				framerate = atof( optarg );
				fprintf( stderr, "Found framerate: %s\n", optarg );
				break;

			case 'e':
				exposure = atof( optarg );
				fprintf( stderr, "Found exposure: %s\n", optarg );
				break;

			case '?':
				printUsage();
				break;
		}
	}

	/* Instead of reporting ‘--verbose’
		 and ‘--brief’ as they are encountered,
		 we report the final status resulting from them. */
	if ( verbose_flag ) {
		puts( "verbose flag is set" );
	}

	// We should have a single argument left, the server address.
	if ( !( optind < argc ) ) {
		printUsage();
		exit( 1 );
	} else {
		struct cameraInfo camera = { NULL, 0, NULL, 0 };
		InitialiseServer( argv[optind], &camera );

		if ( !test_flag ) {
			InitialiseCamera( &camera, pixelclock, framerate, exposure );
		}

		// Event loop here
		if ( !test_flag ) {
			snapshot( camera.hCam, camera.pcImageMemory );
		}

		UeyeToOpenCV( (unsigned char*)camera.pcImageMemory, 1280, 1024 );

		if ( !test_flag ) {
			TerminateCamera( &camera );
		}

		disconnectFromServer( camera.client );
	}

	exit( 0 );
}

// We can send a message, either using Python or from wscli, that requests
// ueye to do something, like capture an image or change a setting.

// Ueye needs to be able to parse the incoming message, so parser.cpp must
// become a general C parser that gets called when a message is received
// and returns a struct that contains the command plus any data

//
