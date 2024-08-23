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
// #include "libpq-fe.h"

#include "ig_common.h"
#include "ig_database.h"
#include "ink_common.h"
#include "psql.h"


// ░▀█▀░█▄█░█▀▀░█░█░▀█▀░░░█▀▄░▀█▀░█▀█░█▀▀░█▀▀░█▀▀░█▀█
// ░░█░░█░█░█░█░█░█░░█░░░░█▀▄░░█░░█░█░█░█░█░█░█▀▀░█░█
// ░▀▀▀░▀░▀░▀▀▀░▀▀▀░▀▀▀░░░▀░▀░▀▀▀░▀░▀░▀▀▀░▀▀▀░▀▀▀░▀░▀

// Constants for all windows
ImVec2 ig2Empty = { 0, 0 };
ImVec2 vertGap	= { 0, 20 };


void
dbTextWindow( t_database_data* self )
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
dbSearchWindow( t_database_data* self )
{
	(void)self;

	igBegin( "Search", NULL, 0 );

	igButton( "Limit by customer", ig2Empty );
	igButton( "Limit by category", ig2Empty );
	igButton( "Limit by date", ig2Empty );

	static int slider = 1;
	igSliderInt( "Record", &slider, 1, 1000, NULL, 0 );

	igEnd();
}

void
dbImageWindow( t_database_data* self )
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
dbDatabaseWindow( t_database_data* self )
{
	// Main window. Create this last so that it is active.

	ImVec2 tbSize = { 0, igGetTextLineHeightWithSpacing() * 8 };

	igBegin( "Database", NULL, 0 );

	// Customer
	static char db_job_description[1024] = "Lorem ipsum";
	static char db_filename[256]	     = "";
	static int  cust_idx		     = 0;
	const char* cust_preview	     = *( self->customers.data + cust_idx * 2 );

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

	igInputTextMultiline( "Job Description", db_job_description, 1024, tbSize, 0, NULL, NULL );

	static float quantity = 1.0f;
	igInputFloat( "Quantity", &quantity, 1.0f, 1.0f, "%.0f", 0 );
	static float price = 0.0f;
	igInputFloat( "Price", &price, 1.0f, 1.0f, "%.2f", 0 );

	igInputText( "Filename", db_filename, 256, 0, NULL, NULL );

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
dbCreateWidgets( t_database_data* self )
{
	if ( self->customers.data == NULL ) {
		self->customers =
			SQLQueryToStruct( self->conn, "SELECT name, filepath FROM customers ORDER BY name ASC" );
	}

	if ( self->fonts.data == NULL ) {
		self->fonts = SQLQueryToStruct( self->conn,
						"SELECT font_name, inkscape_spec FROM fonts ORDER BY font_name ASC" );
	}

	dbDatabaseWindow( self );
	dbTextWindow( self );
	dbSearchWindow( self );
	dbImageWindow( self );
}


// ░█▀▀░█░█░█▀▀░█▀█░▀█▀░░░█░░░█▀█░█▀█░█▀█
// ░█▀▀░▀▄▀░█▀▀░█░█░░█░░░░█░░░█░█░█░█░█▀▀
// ░▀▀▀░░▀░░▀▀▀░▀░▀░░▀░░░░▀▀▀░▀▀▀░▀▀▀░▀░░

bool
dbEventLoop( void* db_data )
{
	t_database_data* self  = (t_database_data*)db_data;
	self->job.result       = NULL;
	self->customers.result = NULL;
	self->fonts.result     = NULL;

	if ( !self->conn ) {
        // Don't spam the connection check
        /* if ( time(NULL) > self->last_db_check + DB_SPAM_DELAY */
		self->conn	     = PSQLInit( "host = 192.168.1.20, dbname = engraving" );
		self->customers.data = NULL;
	}

	// Create the widgets
	dbCreateWidgets( self );

	// Let the ImGui event loop know if we want to continue or exit
	// (true to continue or false to exit)
	return !self->exit_event_loop;
}

int
main( int argc, char** argv )
{
	// We avoid globals by having a struct passed to the event loop
	t_database_data self = {
		.exit_event_loop  = false,
		.fh_script_output = NULL,
		.svg		  = NULL,
		.conn		  = NULL,

		.glfwdata = { .ctx	     = NULL,
			      .width	     = DEFAULT_WINDOW_WIDTH,
			      .height	     = DEFAULT_WINDOW_HEIGHT,
			      .imguiCallback = dbEventLoop },
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
