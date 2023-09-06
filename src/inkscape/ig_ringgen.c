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


// ░▀█▀░█▄█░█▀▀░█░█░▀█▀░░░█▀▄░▀█▀░█▀█░█▀▀░█▀▀░█▀▀░█▀█
// ░░█░░█░█░█░█░█░█░░█░░░░█▀▄░░█░░█░█░█░█░█░█░█▀▀░█░█
// ░▀▀▀░▀░▀░▀▀▀░▀▀▀░▀▀▀░░░▀░▀░▀▀▀░▀░▀░▀▀▀░▀▀▀░▀▀▀░▀░▀

void
rgCreateWidgets( t_ringgen_data* self )
{
	(void)self;

	ImVec2 ig2Empty = { 0, 0 };
	ImVec2 vertGap	= { 0, 20 };

	// First window
	const char* customers[] = {
		"No Template", "G.H. Moore", "Mitchel & Co.", "Rivermounts", "Ashes Memorial Jewellery"
	};
	const char* fonts[] = {
		"Times New Roman Bold Italic", "Monotype Corsiva", "Helvetica", "England Hand", "Arial"
	};

	static int  cust_idx	 = 0;
	static int  font_idx	 = 0;
	const char* cust_preview = customers[cust_idx];
	const char* font_preview = fonts[font_idx];

	// Customer
	static char rg_customer[256] = "Miscellaneous";
	static char rg_filename[256] = "Generic ring";
	// sds	    sds_rg_windows_filename = sdsempty();

	igBegin( "Ring Generator", NULL, 0 );

	if ( igBeginCombo( "Template", cust_preview, 0 ) ) {
		for ( int i = 0; i < (int)( sizeof( customers ) / sizeof( customers[0] ) ); i++ ) {

			const bool is_selected = ( cust_idx == i );
			if ( igSelectable_Bool( customers[i], is_selected, 0, ig2Empty ) ) {

				cust_idx = i;

				if ( is_selected ) {
					igSetItemDefaultFocus();
				}
			}
		}
		igEndCombo();
	}

	igDummy( vertGap );

	// Ring diameter
	static float diameter = 20.0f;
	igInputFloat( "Diameter", &diameter, 0.1f, 1.0f, "%.1f", 0 );
	static float width = 6.0f;
	igInputFloat( "Width", &width, 0.1f, 1.0f, "%.1f", 0 );

	// Internal or extenal engraving
	static int radio = 0;
	igRadioButton_IntPtr( "Inside", &radio, 0 );
	igSameLine( 0, 0 );
	igRadioButton_IntPtr( "Outside", &radio, 1 );
	igDummy( vertGap );

	// Font
	if ( igBeginCombo( "Font", font_preview, 0 ) ) {
		for ( int i = 0; i < (int)( sizeof( fonts ) / sizeof( fonts[0] ) ); i++ ) {

			const bool is_selected = ( font_idx == i );
			ImVec2	   ig2Empty    = { 0, 0 };

			if ( igSelectable_Bool( fonts[i], is_selected, 0, ig2Empty ) ) {

				font_idx = i;

				if ( is_selected ) {
					igSetItemDefaultFocus();
				}
			}
		}
		igEndCombo();
	}

	// Text
	static char ring_text[256] = "";

	igInputText( "Ring Text", ring_text, 256, 0, NULL, 0 );
	igDummy( vertGap );

	igInputText( "Customer", rg_customer, 256, 0, NULL, NULL );
	igInputText( "Filename", rg_filename, 256, 0, NULL, NULL );
	igDummy( vertGap );

	static bool makeDatabase = true;
	igCheckbox( "Create database entry", &makeDatabase );
	igDummy( vertGap );

	// Generate SVG Button
	igButton( "Generate SVG", ig2Empty );
	igDummy( vertGap );

	// generateRemoteFilename( mp_customer, mp_filename, (void*)sds_mp_windows_filename );
	// igInputText( "Windows", sds_mp_windows_filename, 1024, ImGuiInputTextFlags_ReadOnly, NULL, NULL );

	igEnd();
}


// ░█▀▀░█░█░█▀▀░█▀█░▀█▀░░░█░░░█▀█░█▀█░█▀█
// ░█▀▀░▀▄▀░█▀▀░█░█░░█░░░░█░░░█░█░█░█░█▀▀
// ░▀▀▀░░▀░░▀▀▀░▀░▀░░▀░░░░▀▀▀░▀▀▀░▀▀▀░▀░░

bool
rgImGuiLoop( void* rg_data )
{
	t_ringgen_data* self = (t_ringgen_data*)rg_data;

	// Create the widgets
	rgCreateWidgets( self );

	// Let the ImGui event loop know if we want to continue or exit
	// (true to continue or false to exit)
	return !self->exit_event_loop;
}


int
main( int argc, char** argv )
{
	// We avoid globals by having a struct passed to the event loop
	t_ringgen_data self = {
		.glfwdata	  = { .ctx	     = NULL,
				      .width	     = DEFAULT_WINDOW_WIDTH,
				      .height	     = DEFAULT_WINDOW_HEIGHT,
				      .imguiCallback = rgImGuiLoop },
		.fh_script_output = NULL,
		.svg		  = NULL,
		.exit_event_loop  = false,
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
