#include <assert.h>
#include <fcntl.h>
#include <limits.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <time.h>
#include <unistd.h>

// Cimgui
#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#include <cimgui.h>
#include <cimgui_impl.h>

// OpenGL
#include <GL/gl3w.h>
#include <GLFW/glfw3.h>
#include <imgui_impl_glfw.h>

// Blastpit
#include <mxml.h>
#include <sds.h>

#include "ig_common.h"
#include "ig_ringgen.h"
#include "ink_common.h"


int
main( int argc, char** argv )
{
	// We avoid globals by having a struct passed to the event loop
	t_ringgen_data self = {
		.exit_event_loop	= false,
		.fh_script_output	= NULL,
		.glfwdata.ctx		= NULL,
		.glfwdata.height	= DEFAULT_WINDOW_HEIGHT,
		.glfwdata.imguiCallback = rgImGuiLoop,
		.glfwdata.width		= DEFAULT_WINDOW_WIDTH,
		.svg			= NULL,
	};

	// Parse the command line options
	int opt;
	while ( ( opt = getopt( argc, argv, "w:h:n" ) ) != -1 ) {
		switch ( opt ) {
			case 'n':
				fprintf( stderr, "Display deactivated\n" );
				self.glfwdata.imguiCallback = NULL;
				break;
			case 'w':
				self.glfwdata.width = atoi( optarg );
				break;
			case 'h':
				self.glfwdata.height = atoi( optarg );
				break;
			default:
				fprintf( stderr,
					 "Usage: %s [-n(o display)] [-w width] [-h height] [file...]\n",
					 argv[0] );
				exit( EXIT_FAILURE );
		}
	}

	// Parse Inkscape SVG to obtain customer and filename
	if ( argc == optind + 1 ) {
		self.svg = FileToXML( argv[optind] );
	}

	// Initialise graphics
	openGLSetup( &self.glfwdata );

	// Main event loop
	doEventLoop( &self.glfwdata, &self, NULL );

	// Deactivate graphics
	openGLShutdown( &self.glfwdata );

	// Destructors
	if ( self.svg != NULL ) {
		mxmlDelete( self.svg );
	}

	return EXIT_SUCCESS;
}
