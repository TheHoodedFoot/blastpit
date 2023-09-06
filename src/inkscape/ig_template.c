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

// PostgreSQL
#include "libpq-fe.h"
#include "psql.h"

// Tracy
#ifdef TRACY_ENABLE
// #include "Tracy.hpp"
#include "TracyC.h"
#else
// Undefine Tracy Macros if tracy is disabled
#define ZoneScoped
#define TracyCAlloc( a, b )
#define TracyCFree( a )
#define TracyCPlot( a, b )
#define TracyCZone( a, b )
#define TracyCZoneEnd( a )
#define TracyCFrameMark
#endif

#include "cimnodes.h"
#include "cimplot.h"
#include "ig_common.h"
#include "ig_template.h"
#include "ink_common.h"


// ░▀█▀░█▄█░█▀▀░█░█░▀█▀░░░█▀▄░▀█▀░█▀█░█▀▀░█▀▀░█▀▀░█▀█
// ░░█░░█░█░█░█░█░█░░█░░░░█▀▄░░█░░█░█░█░█░█░█░█▀▀░█░█
// ░▀▀▀░▀░▀░▀▀▀░▀▀▀░▀▀▀░░░▀░▀░▀▀▀░▀░▀░▀▀▀░▀▀▀░▀▀▀░▀░▀

// Constants for all windows
ImVec2 ig2Empty = { 0, 0 };
ImVec2 vertGap	= { 0, 20 };


void
tmpl_RingWindow( t_template_data* self )
{
	(void)self;

	igBegin( "Ring Generator", NULL, 0 );

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

	// Generate SVG Button
	igPushID_Int( 1 );
	ImColor but_red_normal, but_red_hover, but_red_active;
	ImColor_HSV( &but_red_normal, 0.0f, 0.6f, 0.6f, 1.0f );
	ImColor_HSV( &but_red_hover, 0.0f, 0.7f, 0.7f, 1.0f );
	ImColor_HSV( &but_red_active, 0.0f, 0.8f, 0.8f, 1.0f );
	igPushStyleColor_Vec4( ImGuiCol_Button, but_red_normal.Value );
	igPushStyleColor_Vec4( ImGuiCol_ButtonHovered, but_red_hover.Value );
	igPushStyleColor_Vec4( ImGuiCol_ButtonActive, but_red_active.Value );

	igButton( "Modify SVG", ig2Empty );

	igPopStyleColor( 3 );  // This must match the number of pushes above
	igPopID();

	igEnd();
}

void
tmpl_TextWindow( t_template_data* self )
{
	(void)self;

	igBegin( "Text", NULL, 0 );

	static int  font_idx	 = 0;
	const char* font_preview = *( self->fonts.data + font_idx * 2 );

	// Font
	if ( igBeginCombo( "Font", font_preview, 0 ) ) {
		for ( int i = 0; i < self->fonts.num_rows; i++ ) {

			const bool is_selected = ( font_idx == i );
			if ( igSelectable_Bool( *( self->fonts.data + ( i * 2 ) ), is_selected, 0, ig2Empty ) ) {
				font_idx = i;
				if ( is_selected ) {
					igSetItemDefaultFocus();
				}
			}
		}
		igEndCombo();
	}

	// Text
	static char svg_text[256] = "Lorem ipsum";

	igInputText( "Text", svg_text, 256, 0, NULL, 0 );
	igDummy( vertGap );

	// Generate SVG Button
	igButton( "Create SVG", ig2Empty );

	igEnd();
}

void
tmpl_SearchWindow( t_template_data* self )
{
	(void)self;

	igBegin( "Search", NULL, 0 );

	igButton( "Limit by customer", ig2Empty );
	igButton( "Limit by category", ig2Empty );
	igButton( "Limit by date", ig2Empty );

	static int slider = 1;
	igSliderInt( "Record", &slider, 1, 1000, NULL, 0 );

	bool dummy = 0;
	ImPlot_ShowDemoWindow( &dummy );
	//
	// uint16_t   bar_data[11] = {0,1,2,3,4,5,6,7,8,9,10};
	//
	// if (ImPlot_BeginPlot("My Plot", ig2Empty, 0)) {
	//    	    // ImPlot_PlotBarsG("My Bar Plot", bar_data, 11);
	//    	    ImPlot_PlotBars_U16PtrInt("plot", bar_data, 11, 1, 0, 0, 0 ,0);
	//    	    ImPlot_EndPlot();
	// }

	igEnd();
}

void
tmpl_ImageWindow( t_template_data* self )
{
	(void)self;

	static int    my_image_width   = 0;
	static int    my_image_height  = 0;
	static GLuint my_image_texture = 0;

	if ( my_image_texture == 0 ) {
		bool ret = LoadTextureFromFile(
			BUILD_DIR "../res/img/MyImage01.jpg", &my_image_texture, &my_image_width, &my_image_height );
		(void)ret;
	}

	igBegin( "OpenGL Texture Text", NULL, 0 );
	igText( "pointer = %p", &my_image_texture );
	igText( "size = %d x %d", my_image_width, my_image_height );
	ImVec2 img_dims	  = { my_image_width, my_image_height };
	ImVec2 imv2_zero  = { 0.0f, 0.0f };
	ImVec2 imv2_unity = { 1.0f, 1.0f };
	ImVec4 imv4_unity = { 1.0f, 1.0f, 1.0f, 1.0f };
	igImage( (void*)(intptr_t)my_image_texture, img_dims, imv2_zero, imv2_unity, imv4_unity, imv4_unity );
	igEnd();
}

void
tmpl_DatabaseWindow( t_template_data* self )
{
	// Main window. Create this last so that it is active.

	ImVec2 tbSize = { 0, igGetTextLineHeightWithSpacing() * 8 };

	igBegin( "Database", NULL, 0 );

	// Customer
	static char tmpl_job_description[1024] = "Lorem ipsum";
	static char tmpl_filename[256]	       = "";
	static int  cust_idx		       = 0;
	const char* cust_preview	       = *( self->customers.data + cust_idx * 2 );

	if ( igBeginCombo( "Customer", cust_preview, 0 ) ) {
		for ( int i = 0; i < self->customers.num_rows; i++ ) {

			const bool is_selected = ( cust_idx == i );
			if ( igSelectable_Bool( *( self->customers.data + ( i * 2 ) ), is_selected, 0, ig2Empty ) ) {

				cust_idx = i;

				if ( is_selected ) {
					igSetItemDefaultFocus();
				}
			}
		}
		igEndCombo();
	}

	igInputTextMultiline( "Job Description", tmpl_job_description, 1024, tbSize, 0, NULL, NULL );

	static float quantity = 1.0f;
	igInputFloat( "Quantity", &quantity, 1.0f, 1.0f, "%.0f", 0 );
	static float price = 0.0f;
	igInputFloat( "Price", &price, 1.0f, 1.0f, "%.2f", 0 );

	igInputText( "Filename", tmpl_filename, 256, 0, NULL, NULL );

	static int job_id = 0;	// Zero indicates unsaved record
	//
	igInputInt( "ID", &job_id, 1, 10, 0 );
	// generateRemoteFilename( mp_customer, mp_filename, (void*)sds_mp_windows_filename );
	// igInputText( "Windows", sds_mp_windows_filename, 1024, ImGuiInputTextFlags_ReadOnly, NULL, NULL );

	igDummy( vertGap );

	// Generate SVG Button
	if ( igButton( "Create Database Entry", ig2Empty ) ) {
		if ( self->job.result != NULL ) {
			// There is an existing result, so free it
			PSQLRelease( self->job.result );
		}

		self->job = SQLQueryToStruct(
			self->conn,
			"INSERT INTO big_jobs (customer, description) VALUES ('bob', 'bobby') RETURNING id;" );
		printf( "%s\n", *self->job.data );
		job_id = atoi( *self->job.data );
	}

	igEnd();
}

void
tmpl_CreateWidgets( t_template_data* self )
{
	tmpl_DatabaseWindow( self );
	tmpl_RingWindow( self );
	tmpl_TextWindow( self );
	tmpl_SearchWindow( self );
	tmpl_ImageWindow( self );
}


// ░█▀▀░█░█░█▀▀░█▀█░▀█▀░░░█░░░█▀█░█▀█░█▀█
// ░█▀▀░▀▄▀░█▀▀░█░█░░█░░░░█░░░█░█░█░█░█▀▀
// ░▀▀▀░░▀░░▀▀▀░▀░▀░░▀░░░░▀▀▀░▀▀▀░▀▀▀░▀░░

bool
tmpl_EventLoop( void* tmpl_data )
{
	TracyCZone( eventloop, true )

		t_template_data* self = (t_template_data*)tmpl_data;

	// Create the widgets
	tmpl_CreateWidgets( self );

	TracyCZoneEnd( eventloop )

		// Let the ImGui event loop know if we want to continue or exit
		// (true to continue or false to exit)
		return !self->exit_event_loop;
}


// ░█▄█░█▀█░▀█▀░█▀█
// ░█░█░█▀█░░█░░█░█
// ░▀░▀░▀░▀░▀▀▀░▀░▀

int
main( int argc, char** argv )
{
	TracyCZone( main, true )

		// We avoid globals by having a struct passed to the event loop
		t_template_data self = {
			.exit_event_loop  = false,
			.glfwdata	  = { .ctx	     = NULL,
					      .width	     = DEFAULT_WINDOW_WIDTH,
					      .height	     = DEFAULT_WINDOW_HEIGHT,
					      .imguiCallback = tmpl_EventLoop },
			.svg		  = NULL,
			.fh_script_output = NULL,
			.conn		  = NULL,
			.customers	  = { .result = NULL },
			.fonts		  = { .result = NULL },
			.job		  = { .result = NULL },
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

	// Initialise imnodes and implot context
	self.inctx = imnodes_CreateContext();
	self.ipctx = ImPlot_CreateContext();

	// For now, just run the queries once before the loop
	if ( !self.conn ) {
		self.conn = PSQLInit( "host = hilly, dbname = engraving" );
	}
	if ( self.customers.data == NULL ) {
		self.customers =
			SQLQueryToStruct( self.conn, "SELECT name, filepath FROM customers ORDER BY name ASC" );
	}
	if ( self.fonts.data == NULL ) {
		self.fonts = SQLQueryToStruct( self.conn,
					       "SELECT font_name, inkscape_spec FROM fonts ORDER BY font_name ASC" );
	}


	// Main event loop
	doEventLoop( &self.glfwdata, &self, NULL );

	// Destroy imnodes and implot context
	ImPlot_DestroyContext( self.ipctx );
	imnodes_DestroyContext( self.inctx );

	// Deactivate graphics
	openGLShutdown( &self.glfwdata );

	// Destructors
	if ( self.svg != NULL ) {
		mxmlDelete( self.svg );
	}
	if ( self.fonts.data ) {
		free( self.fonts.data );
	}
	if ( self.fonts.result ) {
		PSQLRelease( self.fonts.result );
	}
	if ( self.customers.data ) {
		free( self.customers.data );
	}
	if ( self.customers.result ) {
		PSQLRelease( self.customers.result );
	}
	if ( self.job.data ) {
		free( self.job.data );
	}
	if ( self.job.result ) {
		PSQLRelease( self.job.result );
	}
	PSQLExit( self.conn );

	TracyCZoneEnd( main )

		return EXIT_SUCCESS;
}
