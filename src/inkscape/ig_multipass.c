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
#include "ig_multipass.h"
#include "ink_common.h"


// ░▀█▀░█▄█░█▀▀░█░█░▀█▀░░░█▄█░█░█░█░░░▀█▀░▀█▀░█▀█░█▀█░█▀▀░█▀▀
// ░░█░░█░█░█░█░█░█░░█░░░░█░█░█░█░█░░░░█░░░█░░█▀▀░█▀█░▀▀█░▀▀█
// ░▀▀▀░▀░▀░▀▀▀░▀▀▀░▀▀▀░░░▀░▀░▀▀▀░▀▀▀░░▀░░▀▀▀░▀░░░▀░▀░▀▀▀░▀▀▀

void
mpCreateWidgets( t_multipass_data* self )
{
	ImVec2 ig2Empty = { 0, 0 };
	ImVec2 vertGap	= { 0, 20 };

	igBegin( "Multipass Ring", NULL, 0 );

	// Customer
	static char mp_customer[256] = "Miscellaneous";
	static char mp_filename[256] = "Generic ring";
	// static char mp_windows_filename[1024] = "src/inkscape/multipass_generate.py -i
	// \"Z:\\\\drawings\\\\2022\\\\ANDREW_MACGOWAN\\\\Hamlet_pen_band.VLM\" --passes 4";

	igInputText( "Customer", mp_customer, 256, 0, NULL, NULL );
	igInputText( "Filename", mp_filename, 256, 0, NULL, NULL );

	sds sds_mp_windows_filename = generateRemoteFilename( mp_customer, mp_filename );
	igInputText( "Windows", sds_mp_windows_filename, 1024, ImGuiInputTextFlags_ReadOnly, NULL, NULL );

	static int num_passes = 10;
	igInputInt( "Number of passes", &num_passes, 1, 1, 0 );
	if ( num_passes < 1 ) {
		num_passes = 1;
	}
	static int start_pass = 0;
	igInputInt( "Starting pass", &start_pass, 1, 1, 0 );
	if ( start_pass < 0 ) {
		start_pass = 0;
	}
	if ( start_pass > num_passes - 1 ) {
		start_pass = num_passes - 1;
	}

	igDummy( vertGap );

	// Generate SVG Button
	ImVec2 horizGap = { 10, 0 };
	if ( self->status == MULTIPASS_INACTIVE ) {
		if ( igButton( "Generate Multipass", ig2Empty ) ) {
			sds command	  = sdsnew( "multipass_generate.py -i " );
			command		  = sdscatsds( command, sds_mp_windows_filename );
			command		  = sdscat( command, " --passes " );
			sds str_numpasses = sdsfromlonglong( (long)num_passes );
			command		  = sdscatsds( command, str_numpasses );
			sdsfree( str_numpasses );

			startMultipassScript( command, self );
			self->status = MULTIPASS_GENERATE;
			sdsfree( command );
		}
		igSameLine( 0, 0 );
		igDummy( horizGap );
		igSameLine( 0, 0 );

		// igPushStyleVar_Vec2(ImGuiStyleVar_ItemSpacing, horizGap);
		igPushID_Int( 1 );
		ImColor but_red_normal, but_red_hover, but_red_active;
		ImColor_HSV( &but_red_normal, 0.0f, 0.6f, 0.6f, 1.0f );
		ImColor_HSV( &but_red_hover, 0.0f, 0.7f, 0.7f, 1.0f );
		ImColor_HSV( &but_red_active, 0.0f, 0.8f, 0.8f, 1.0f );
		igPushStyleColor_Vec4( ImGuiCol_Button, but_red_normal.Value );
		igPushStyleColor_Vec4( ImGuiCol_ButtonHovered, but_red_hover.Value );
		igPushStyleColor_Vec4( ImGuiCol_ButtonActive, but_red_active.Value );
		if ( igButton( "Execute Multipass", ig2Empty ) ) {
			sds command	  = sdsnew( "multipass_execute.py -i " );
			command		  = sdscatsds( command, sds_mp_windows_filename );
			command		  = sdscat( command, " --passes " );
			sds str_numpasses = sdsfromlonglong( (long)num_passes );
			command		  = sdscatsds( command, str_numpasses );
			command		  = sdscat( command, " --begin " );
			sds str_startpass = sdsfromlonglong( (long)start_pass );
			command		  = sdscatsds( command, str_startpass );
			sdsfree( str_numpasses );

			startMultipassScript( command, self );
			self->status = MULTIPASS_GENERATE;
			sdsfree( command );
		}

		igPopStyleColor( 3 );  // This must match the number of pushes above
		igPopID();
	} else {
		if ( self->status == MULTIPASS_DISPLAY_PAUSED ) {
			if ( igButton( "Close", ig2Empty ) ) {
				self->status = MULTIPASS_INACTIVE;
			}
		} else {
			if ( igButton( "Cancel", ig2Empty ) ) {
				self->status = MULTIPASS_CANCEL;
				endMultipassScript( self );
			}
		}
	}

	sdsfree( sds_mp_windows_filename );

	// igPopStyleVar(1);

	igDummy( vertGap );

	// Progress Bar
	if ( self->status != MULTIPASS_INACTIVE && self->buf != NULL ) {
		static ImGuiInputTextFlags flags     = ImGuiInputTextFlags_AllowTabInput;
		ImVec2			   tbox_size = { igGET_FLT_MIN(), igGetTextLineHeight() * 16 };

		igInputTextMultiline( "##source", self->buf, strlen( self->buf ), tbox_size, flags, NULL, NULL );
		// To continually display the last entry, we have to
		// open a child window (ImGui #1523)
		igBeginChild_Str( "##source", ig2Empty, true, 0 );
		igSetScrollHereY( 1.0f );
		igEndChild();

		float div = (float)self->current_pass / (float)self->total_passes;
		if ( div > 0 ) {
			ImVec2 ig_progbarsize = { -1.0f, 0.0f };
			char   pbar_buf[64];

			snprintf( pbar_buf, 64, "Pass %d of %d\n", self->current_pass, self->total_passes );
			igProgressBar( (float)( (float)self->current_pass / (float)self->total_passes ),
				       ig_progbarsize,
				       pbar_buf );
		}
	}

	igEnd();
}


// ░█▄█░█░█░█░░░▀█▀░▀█▀░█▀█░█▀█░█▀▀░█▀▀
// ░█░█░█░█░█░░░░█░░░█░░█▀▀░█▀█░▀▀█░▀▀█
// ░▀░▀░▀▀▀░▀▀▀░░▀░░▀▀▀░▀░░░▀░▀░▀▀▀░▀▀▀

int
startMultipassScript( const char* command, t_multipass_data* self )
{  // Execute the script

	/* Open the command for reading. */
	self->fh_script_output = popen( command, "r" );
	if ( self->fh_script_output == NULL ) {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
		perror( __FUNCTION__ );
#pragma GCC diagnostic pop
		return ( false );
	}

	// Set file handle to nonblocking
	// int fd	  = fileno( self->fh_script_output );
	// int flags = fcntl( fd, F_GETFL, 0 );
	// flags |= O_NONBLOCK;
	// fcntl( fd, F_SETFL, flags );

	// Allocate initial buffer
	self->buf	= malloc( MP_TEXT_BUFFER_MAX_SIZE );
	self->buf_size	= MP_TEXT_BUFFER_MAX_SIZE;
	self->idx_write = 0;

	return true;
}

int
mpParseFilename( t_multipass_data* self )
{  // Obtain the filename from the SVG

	(void)self;

	// Obtain customer and filename from XML

	// Move ImGui into separate thread
	return false;
}

int
pollMultipass( t_multipass_data* self )
{  // Get output from running script

	if ( self->fh_script_output == NULL ) {
		return false;
	}

	// Read the output a line at a time
	char* get_stdin =
		fgets( self->buf + self->idx_write, self->buf_size - self->idx_write, self->fh_script_output );
	if ( get_stdin == NULL ) {
		endMultipassScript( self );
		return false;
	}

	// Parse the data to obtain the currently processed pass
	int result = sscanf( self->buf + self->idx_write, "(%d/%d)", &self->current_pass, &self->total_passes );
	(void)result;

	// Output to the text box
	self->idx_write = strlen( self->buf );
	if ( self->idx_write > MP_TEXT_BUFFER_MIN_SIZE ) {
		self->idx_write = 0;
	}

	return true;
}

int
endMultipassScript( t_multipass_data* self )
{  // Close file handle and clean up

	pclose( self->fh_script_output );
	self->fh_script_output = NULL;
	self->status	       = MULTIPASS_DISPLAY_PAUSED;

	return true;
}


// ░█▀▀░█░█░█▀▀░█▀█░▀█▀░░░█░░░█▀█░█▀█░█▀█
// ░█▀▀░▀▄▀░█▀▀░█░█░░█░░░░█░░░█░█░█░█░█▀▀
// ░▀▀▀░░▀░░▀▀▀░▀░▀░░▀░░░░▀▀▀░▀▀▀░▀▀▀░▀░░

bool
mpEventLoop( void* multipass_data )
{
	t_multipass_data* self = (t_multipass_data*)multipass_data;

	// If multipass is active, check stdin
	if ( self->status == MULTIPASS_GENERATE || self->status == MULTIPASS_EXECUTE ) {
		pollMultipass( self );
	}

	return !self->exit_event_loop;
}

bool
mpImGuiLoop( void* multipass_data )
{
	t_multipass_data* self = (t_multipass_data*)multipass_data;

	// Create the widgets
	mpCreateWidgets( self );

	// Let the ImGui event loop know if we want to continue or exit
	// (true to continue or false to exit)
	return !self->exit_event_loop;
}

// #include <stdio.h>
// #include <stdlib.h>
// #include <pthread.h>

// void *print_message_function( void *ptr );

// main()
// {
//      pthread_t thread1, thread2;
//      char *message1 = "Thread 1";
//      char *message2 = "Thread 2";
//      int  iret1, iret2;

//     /* Create independent threads each of which will execute function */

//      iret1 = pthread_create( &thread1, NULL, print_message_function, (void*) message1);
//      iret2 = pthread_create( &thread2, NULL, print_message_function, (void*) message2);

//      /* Wait till threads are complete before main continues. Unless we  */
//      /* wait we run the risk of executing an exit which will terminate   */
//      /* the process and all threads before the threads have completed.   */

//      pthread_join( thread1, NULL);
//      pthread_join( thread2, NULL);

//      printf("Thread 1 returns: %d\n",iret1);
//      printf("Thread 2 returns: %d\n",iret2);
//      exit(0);
// }

// void *print_message_function( void *ptr )
// {
//      char *message;
//      message = (char *) ptr;
//      printf("%s \n", message);
// }


int
main( int argc, char** argv )
{
	// We avoid globals by passing a struct to the event loop
	t_multipass_data self = {
		.glfwdata	  = { .ctx	     = NULL,
				      .width	     = DEFAULT_WINDOW_WIDTH,
				      .height	     = DEFAULT_WINDOW_HEIGHT,
				      .imguiCallback = mpImGuiLoop },
		.fh_script_output = NULL,
		.status		  = MULTIPASS_INACTIVE,
		.svg		  = NULL,
	};

	// Parse the command line options
	int opt;
	while ( ( opt = getopt( argc, argv, "w:h:t" ) ) != -1 ) {
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
	doEventLoop( &self.glfwdata, &self, mpEventLoop );

	// Deactivate graphics
	openGLShutdown( &self.glfwdata );

	// Destructors
	if ( self.svg != NULL ) {
		mxmlDelete( self.svg );
	}

	return EXIT_SUCCESS;
}
