#ifndef IG_COMMON
#define IG_COMMON

#include "GLFW/glfw3.h"
#include <stdbool.h>

enum constants
{
	DEFAULT_WINDOW_WIDTH  = 1200,
	DEFAULT_WINDOW_HEIGHT = 800,
};

typedef struct
{
	GLFWwindow*	     win;
	struct ImGuiContext* ctx;  // Segfault if not null on openGLSetup()
	struct ImGuiIO*	     io;
	int		     width;
	int		     height;
	bool		     ( *imguiCallback )( void* );
} t_glfw_data;

bool LoadTextureFromFile( const char* filename, GLuint* out_texture, int* out_width, int* out_height );
void openGLSetup( t_glfw_data* glfwdata );
void doEventLoop( t_glfw_data* glfwdata, void* self, bool ( *headlessCallback )( void* ) );
void glfw_error_callback( int e, const char* d );
void gui_init( GLFWwindow* win, struct ImGuiContext* ctx, struct ImGuiIO* io );
void gui_terminate( struct ImGuiContext* ctx );
void imgui_render( GLFWwindow** win, int* width, int* height );
void openGLShutdown( t_glfw_data* glfwdata );

#endif /* end of include guard: IG_COMMON */
