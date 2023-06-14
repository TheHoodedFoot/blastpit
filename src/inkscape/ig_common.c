#include <assert.h>
#include <ctype.h>
#include <fcntl.h>
#include <limits.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>

// Cimgui
#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#include <cimgui.h>
#include <cimgui_impl.h>

// OpenGL
#include <GL/gl3w.h>
#include <GLFW/glfw3.h>
#include <imgui_impl_glfw.h>

// Bitmap image
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "ig_common.h"


// ░▀█▀░█▄█░█▀█░█▀▀░█▀▀░░░█░█░▀█▀░▀█▀░█░░░▀█▀░▀█▀░▀█▀░█▀▀░█▀▀
// ░░█░░█░█░█▀█░█░█░█▀▀░░░█░█░░█░░░█░░█░░░░█░░░█░░░█░░█▀▀░▀▀█
// ░▀▀▀░▀░▀░▀░▀░▀▀▀░▀▀▀░░░▀▀▀░░▀░░▀▀▀░▀▀▀░▀▀▀░░▀░░▀▀▀░▀▀▀░▀▀▀

bool
LoadTextureFromFile( const char* filename, GLuint* out_texture, int* out_width, int* out_height )
{  // Simple helper function to load an image into a OpenGL texture with common settings

	int	       image_width  = 0;
	int	       image_height = 0;
	unsigned char* image_data   = stbi_load( filename, &image_width, &image_height, NULL, 4 );
	if ( image_data == NULL ) {
		return false;
	}

	// Create a OpenGL texture identifier
	GLuint image_texture;
	glGenTextures( 1, &image_texture );
	glBindTexture( GL_TEXTURE_2D, image_texture );

	// Setup filtering parameters for display
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
	glTexParameteri( GL_TEXTURE_2D,
			 GL_TEXTURE_WRAP_S,
			 GL_CLAMP_TO_EDGE );  // This is required on WebGL for non power-of-two textures
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );	// Same

	// Upload pixels into texture
#if defined( GL_UNPACK_ROW_LENGTH ) && !defined( __EMSCRIPTEN__ )
	glPixelStorei( GL_UNPACK_ROW_LENGTH, 0 );
#endif
	glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, image_width, image_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image_data );
	stbi_image_free( image_data );

	*out_texture = image_texture;
	*out_width   = image_width;
	*out_height  = image_height;

	return true;
}


// ░█▀▀░█░░░█▀▀░█░█
// ░█░█░█░░░█▀▀░█▄█
// ░▀▀▀░▀▀▀░▀░░░▀░▀

void
openGLSetup( t_glfw_data* glfwdata )
{  // Our Imgui OpenGL setup routine

	const int win_width  = glfwdata->width;
	const int win_height = glfwdata->height;

	/* GLFW */
	glfwSetErrorCallback( glfw_error_callback );
	if ( !glfwInit() ) {
		fprintf( stderr, "[GFLW] failed to init!\n" );
		exit( EXIT_FAILURE );
	}

	glfwWindowHint( GLFW_CONTEXT_VERSION_MAJOR, 3 );
	glfwWindowHint( GLFW_CONTEXT_VERSION_MINOR, 3 );

	glfwdata->win = glfwCreateWindow( win_width, win_height, "ImGui", NULL, NULL );
	if ( !glfwdata->win ) {
		fprintf( stderr, "glfwCreateWindow failed.\n" );
		exit( EXIT_FAILURE );
	}

	glfwMakeContextCurrent( glfwdata->win );
	bool err = gl3wInit();
	if ( err ) {
		fprintf( stderr, "Failed to initialize OpenGL loader!\n" );
		exit( EXIT_FAILURE );
	}

	gui_init( glfwdata->win, glfwdata->ctx, glfwdata->io );

	// glfwSetWindowSizeCallback(win, onResize);
	glfwGetWindowSize( glfwdata->win, &glfwdata->width, &glfwdata->height );

	/* OpenGL */
	glViewport( 0, 0, win_width, win_height );
}

void
openGLShutdown( t_glfw_data* glfwdata )
{  // Our Imgui OpenGL shutdown routine

	gui_terminate( glfwdata->ctx );
	glfwTerminate();
}

void
glfw_error_callback( int e, const char* d )
{
	printf( "Error %d: %s\n", e, d );
}


// ░▀█▀░█▄█░█▀▀░█░█░▀█▀░░░█▀▀░█▀█░█▄█░█▄█░█▀█░█▀█
// ░░█░░█░█░█░█░█░█░░█░░░░█░░░█░█░█░█░█░█░█░█░█░█
// ░▀▀▀░▀░▀░▀▀▀░▀▀▀░▀▀▀░░░▀▀▀░▀▀▀░▀░▀░▀░▀░▀▀▀░▀░▀

void
gui_init( GLFWwindow* win, struct ImGuiContext* ctx, struct ImGuiIO* io )
{
	// IMGUI_CHECKVERSION();
	ctx = igCreateContext( NULL );
	io  = igGetIO();

	(void)ctx;  // Squash warning about unused variables
	(void)io;

	const char* glsl_version = "#version 330 core";
	ImGui_ImplGlfw_InitForOpenGL( win, true );
	ImGui_ImplOpenGL3_Init( glsl_version );

	// Setup style
	igStyleColorsDark( NULL );
}

void
gui_terminate( struct ImGuiContext* ctx )
{
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	igDestroyContext( ctx );
}

void
imgui_render( GLFWwindow** win, int* width, int* height )
{
	/* Draw */
	glfwGetWindowSize( *win, width, height );
	glViewport( 0, 0, *width, *height );
	glClear( GL_COLOR_BUFFER_BIT );
	glClearColor( 0.0, 0.0, 0.0, 0.0 );

	igRender();
	ImGui_ImplOpenGL3_RenderDrawData( igGetDrawData() );

	glfwSwapBuffers( *win );
}


// ░▀█▀░█▄█░█▀▀░█░█░▀█▀░░░█▀▀░█░█░█▀▀░█▀█░▀█▀░░░█░░░█▀█░█▀█░█▀█
// ░░█░░█░█░█░█░█░█░░█░░░░█▀▀░▀▄▀░█▀▀░█░█░░█░░░░█░░░█░█░█░█░█▀▀
// ░▀▀▀░▀░▀░▀▀▀░▀▀▀░▀▀▀░░░▀▀▀░░▀░░▀▀▀░▀░▀░░▀░░░░▀▀▀░▀▀▀░▀▀▀░▀░░

void
doEventLoop( t_glfw_data* glfwdata, void* callback_data, bool ( *headlessCallback )( void* ) )
{  // Warning: Ensure that the headless callback contains no ImGui functions

	while ( headlessCallback || glfwdata->imguiCallback ) {
		if ( headlessCallback != NULL ) {
			if ( !( *headlessCallback )( callback_data ) ) {
				// Headless has exited, so clear its pointer
				headlessCallback = NULL;
			}
		}

		if ( glfwdata->imguiCallback != NULL ) {

			// ImGui prepare
			glfwPollEvents();
			ImGui_ImplOpenGL3_NewFrame();
			ImGui_ImplGlfw_NewFrame();
			igNewFrame();

			// Our ImGui widget callback
			if ( !( *glfwdata->imguiCallback )( callback_data ) ) {
				// ImGui has exited, so clear its pointer
				glfwdata->imguiCallback = NULL;
			} else {
				// Render the widgets
				imgui_render( &glfwdata->win, &glfwdata->width, &glfwdata->height );
			}

			if ( glfwWindowShouldClose( glfwdata->win ) ) {
				// Window has been closed, so exit event loop
				glfwdata->imguiCallback = NULL;

				// TODO: This should allow the headless to continue,
				// but for now it just causes the imgui window to stay open
				// and prevents the program from exiting,
				// so we set the headlessCallback to NULL to exit
				headlessCallback = NULL;
			}
		}
	}
}

// Threads
// https://www.cs.cmu.edu/afs/cs/academic/class/15492-f07/www/pthreads.html
