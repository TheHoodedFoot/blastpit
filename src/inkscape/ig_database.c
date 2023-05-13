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

#include "ig_common.h"
#include "ig_database.h"
#include "psql.h"


// ░▀█▀░█▄█░█▀▀░█░█░▀█▀░░░█▀▄░▀█▀░█▀█░█▀▀░█▀▀░█▀▀░█▀█
// ░░█░░█░█░█░█░█░█░░█░░░░█▀▄░░█░░█░█░█░█░█░█░█▀▀░█░█
// ░▀▀▀░▀░▀░▀▀▀░▀▀▀░▀▀▀░░░▀░▀░▀▀▀░▀░▀░▀▀▀░▀▀▀░▀▀▀░▀░▀

// Constants for all windows
ImVec2 ig2Empty = { 0, 0 };
ImVec2 vertGap	= { 0, 20 };


void
dbRingWindow( t_database_data* self )
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
	dbRingWindow( self );
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
		self->conn	     = PSQLInit( "host = hilly, dbname = engraving" );
		self->customers.data = NULL;
	}

	// Create the widgets
	dbCreateWidgets( self );

	// Let the ImGui event loop know if we want to continue or exit
	// (true to continue or false to exit)
	return !self->exit_event_loop;
}
