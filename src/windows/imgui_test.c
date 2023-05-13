#include <assert.h>
#include <limits.h>
#include <math.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#include <cimgui.h>
#include <cimgui_impl.h>

#include <GL/gl3w.h>
#include <GLFW/glfw3.h>
#include <imgui_impl_glfw.h>

// Bitmap image
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define WINDOW_WIDTH  1200
#define WINDOW_HEIGHT 800

static void
error_callback( int e, const char* d )
{
	printf( "Error %d: %s\n", e, d );
}

/* Platform */
static GLFWwindow*   win;
struct ImGuiContext* ctx;
struct ImGuiIO*	     io;

// Simple helper function to load an image into a OpenGL texture with common settings
bool
LoadTextureFromFile( const char* filename, GLuint* out_texture, int* out_width, int* out_height )
{
	// Load from file
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

void
gui_init()
{
	// IMGUI_CHECKVERSION();
	ctx = igCreateContext( NULL );
	io  = igGetIO();

	const char* glsl_version = "#version 330 core";
	ImGui_ImplGlfw_InitForOpenGL( win, true );
	ImGui_ImplOpenGL3_Init( glsl_version );

	// Setup style
	igStyleColorsDark( NULL );
}

void
gui_terminate()
{
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	igDestroyContext( ctx );
}

void
gui_render()
{
	igRender();
	ImGui_ImplOpenGL3_RenderDrawData( igGetDrawData() );
}

void
gui_update()
{
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	igNewFrame();

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
	ImVec2	    ig2Empty	 = { 0, 0 };
	ImVec2	    vertGap	 = { 0, 20 };

	igBegin( "Ring Generator", NULL, 0 );

	if ( igBeginCombo( "Template", cust_preview, 0 ) ) {
		for ( int i = 0; i < (int)sizeof( customers ) / (int)sizeof( customers[0] ); i++ ) {

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
		for ( int i = 0; i < (int)sizeof( fonts ) / (int)sizeof( fonts[0] ); i++ ) {

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

	// Customer
	static char customer[256] = "Miscellaneous";
	static char filename[256] = "Generic ring";

	igInputText( "Customer", customer, 256, 0, NULL, 0 );
	igInputText( "Filename", filename, 256, 0, NULL, 0 );
	igDummy( vertGap );
	// Generate SVG Button
	igButton( "Generate SVG", ig2Empty );
	igDummy( vertGap );

	// Demo Window Enable/Disable
	static bool show_demo_window = false;
	igCheckbox( "Demo Window", &show_demo_window );
	igSameLine( 0, 0 );
	static bool show_image_window = false;
	igCheckbox( "Image Window", &show_image_window );

	igEnd();

	// window2
	if ( show_image_window ) {
		static int    my_image_width   = 0;
		static int    my_image_height  = 0;
		static GLuint my_image_texture = 0;

		if ( my_image_texture == 0 ) {
			LoadTextureFromFile(
				BUILD_DIR "MyImage01.jpg", &my_image_texture, &my_image_width, &my_image_height );
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

	// Demo window
	if ( show_demo_window ) {
		igShowDemoWindow( &show_demo_window );
	}
}

int
main( int argc, char** argv )
{

	(void)argc;
	(void)argv;

	/* GLFW */
	glfwSetErrorCallback( error_callback );
	if ( !glfwInit() ) {
		fprintf( stdout, "[GFLW] failed to init!\n" );
		exit( 1 );
	}

	glfwWindowHint( GLFW_CONTEXT_VERSION_MAJOR, 3 );
	glfwWindowHint( GLFW_CONTEXT_VERSION_MINOR, 3 );

	win = glfwCreateWindow( WINDOW_WIDTH, WINDOW_HEIGHT, "ImGui", NULL, NULL );
	glfwMakeContextCurrent( win );
	bool err = gl3wInit();
	if ( err ) {
		fprintf( stderr, "Failed to initialize OpenGL loader!\n" );
		return 1;
	}

	gui_init();

	// glfwSetWindowSizeCallback(win, onResize);
	int width, height;
	glfwGetWindowSize( win, &width, &height );

	/* OpenGL */
	glViewport( 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT );


	while ( !glfwWindowShouldClose( win ) ) {
		/* Input */
		glfwPollEvents();

		gui_update();

		/* Draw */
		glfwGetWindowSize( win, &width, &height );
		glViewport( 0, 0, width, height );
		glClear( GL_COLOR_BUFFER_BIT );
		glClearColor( 0.0, 0.0, 0.0, 0.0 );

		gui_render();

		glfwSwapBuffers( win );
	}

	gui_terminate();
	glfwTerminate();
}

/*

 To move widgets, get the current cursor position and change it:

	// ImVec2 pos;
	// igGetCursorScreenPos( &pos ); pos.x += 20;
	// igSetCursorScreenPos( pos );

*/
