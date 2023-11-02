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

#include "cimnodes.h"
#include "ig_cimnodes.h"
#include "ig_common.h"
#include "ink_common.h"

#ifndef IM_COL32_R_SHIFT
#ifdef IMGUI_USE_BGRA_PACKED_COLOR
#define IM_COL32_R_SHIFT 16
#define IM_COL32_G_SHIFT 8
#define IM_COL32_B_SHIFT 0
#define IM_COL32_A_SHIFT 24
#define IM_COL32_A_MASK	 0xFF000000
#else
#define IM_COL32_R_SHIFT 0
#define IM_COL32_G_SHIFT 8
#define IM_COL32_B_SHIFT 16
#define IM_COL32_A_SHIFT 24
#define IM_COL32_A_MASK	 0xFF000000
#endif
#endif
#define IM_COL32( R, G, B, A )                                                                                         \
	( ( (ImU32)( A ) << IM_COL32_A_SHIFT ) | ( (ImU32)( B ) << IM_COL32_B_SHIFT ) |                                \
	  ( (ImU32)( G ) << IM_COL32_G_SHIFT ) | ( (ImU32)( R ) << IM_COL32_R_SHIFT ) )
#define IM_COL32_WHITE	     IM_COL32( 255, 255, 255, 255 )  // Opaque white = 0xFFFFFFFF
#define IM_COL32_BLACK	     IM_COL32( 0, 0, 0, 255 )	     // Opaque black
#define IM_COL32_BLACK_TRANS IM_COL32( 0, 0, 0, 0 )	     // Transparent black = 0x00000000


// ░█▀▀░▀█▀░█▄█░█▀█░█▀█░█▀▄░█▀▀░█▀▀
// ░█░░░░█░░█░█░█░█░█░█░█░█░█▀▀░▀▀█
// ░▀▀▀░▀▀▀░▀░▀░▀░▀░▀▀▀░▀▀░░▀▀▀░▀▀▀

void
cimCreateWidgets( t_cimnodes_data* self )
{
	(void)self;

	// First window
	igBegin( "Cimnodes Example", NULL, 0 );
	imnodes_BeginNodeEditor();

	imnodes_PushColorStyle( ImNodesCol_Link, IM_COL32( 127, 255, 0, 255 ) );
	imnodes_PushColorStyle( ImNodesCol_LinkSelected, IM_COL32( 127, 255, 127, 255 ) );
	imnodes_PushColorStyle( ImNodesCol_LinkHovered, IM_COL32( 127, 255, 127, 255 ) );

	imnodes_PushColorStyle( ImNodesCol_TitleBar, IM_COL32( 0, 255, 0, 255 ) );
	imnodes_BeginNode( 1 );
	imnodes_BeginNodeTitleBar();
	igText( "SVG", NULL );
	imnodes_EndNodeTitleBar();
	imnodes_PopColorStyle();

	// imnodes_BeginInputAttribute( 2, ImNodesPinShape_Circle );
	// igText( "Input" );
	// imnodes_EndInputAttribute();

	imnodes_PushColorStyle( ImNodesCol_Pin, IM_COL32( 255, 0, 0, 255 ) );
	imnodes_BeginOutputAttribute( 11, ImNodesPinShape_CircleFilled );
	imnodes_PopColorStyle();
	// igIndent( 40 );
	igText( "Geometry" );
	imnodes_EndOutputAttribute();
	imnodes_BeginOutputAttribute( 12, ImNodesPinShape_Circle );
	// igIndent( 40 );
	igText( "Job" );
	imnodes_EndOutputAttribute();

	imnodes_EndNode();
	imnodes_PopColorStyle();


	imnodes_PushColorStyle( ImNodesCol_TitleBar, IM_COL32( 255, 0, 0, 255 ) );
	imnodes_BeginNode( 2 );
	imnodes_BeginNodeTitleBar();
	igText( "Layer Split", NULL );
	imnodes_EndNodeTitleBar();

	imnodes_BeginInputAttribute( 13, ImNodesPinShape_Circle );
	igText( "Geometry" );
	imnodes_EndInputAttribute();

	imnodes_BeginOutputAttribute( 14, ImNodesPinShape_Circle );
	igIndent( 40 );
	igText( "Layer" );
	imnodes_EndOutputAttribute();
	imnodes_BeginOutputAttribute( 15, ImNodesPinShape_Circle );
	igIndent( 40 );
	igText( "Non-Layer" );
	imnodes_EndOutputAttribute();

	imnodes_EndNode();

	imnodes_PopColorStyle();  // Link
	imnodes_PopColorStyle();  // Link
	imnodes_PopColorStyle();  // Link

	imnodes_Link( 26, 11, 13 );
	imnodes_EndNodeEditor();

	// TODO: make the context static, dont create and destroy every frame
	igEnd();
}


// ░█▀▀░█░█░█▀▀░█▀█░▀█▀░░░█░░░█▀█░█▀█░█▀█
// ░█▀▀░▀▄▀░█▀▀░█░█░░█░░░░█░░░█░█░█░█░█▀▀
// ░▀▀▀░░▀░░▀▀▀░▀░▀░░▀░░░░▀▀▀░▀▀▀░▀▀▀░▀░░

bool
cimImGuiLoop( void* cim_data )
{
	t_cimnodes_data* self = (t_cimnodes_data*)cim_data;

	// Create the widgets
	cimCreateWidgets( self );

	// Let the ImGui event loop know if we want to continue or exit
	// (true to continue or false to exit)
	return !self->exit_event_loop;
}


int
main( int argc, char** argv )
{
	// We avoid globals by having a struct passed to the event loop
	t_cimnodes_data self = {
		.svg		  = NULL,
		.fh_script_output = NULL,
		.exit_event_loop  = false,
		.glfwdata	  = { .ctx	     = NULL,
				      .width	     = DEFAULT_WINDOW_WIDTH,
				      .height	     = DEFAULT_WINDOW_HEIGHT,
				      .imguiCallback = cimImGuiLoop },
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

	// Initialise graphics and imgui context
	openGLSetup( &self.glfwdata );

	// Initialise imnodes context
	self.inctx = imnodes_CreateContext();

	// Main event loop
	doEventLoop( &self.glfwdata, &self, NULL );

	// Destroy imnodes context
	imnodes_DestroyContext( self.inctx );

	// Deactivate graphics
	openGLShutdown( &self.glfwdata );

	// Destructors
	if ( self.svg != NULL ) {
		mxmlDelete( self.svg );
	}

	return EXIT_SUCCESS;
}
