#ifndef IG_MULTIPASS
#define IG_MULTIPASS

#include <mxml.h>
#include <stdbool.h>
#include <stdio.h>

#include "ig_common.h"

enum mp_constants
{
	MP_TEXT_BUFFER_MIN_SIZE = 1024 * 2,
	MP_TEXT_BUFFER_MAX_SIZE = 1024 * 3,
};

enum multipass_status
{
	MULTIPASS_INACTIVE	 = 0,
	MULTIPASS_GENERATE	 = 1,
	MULTIPASS_EXECUTE	 = 2,
	MULTIPASS_CANCEL	 = 3,
	MULTIPASS_DISPLAY_PAUSED = 4,
};

typedef struct
{
	t_glfw_data  glfwdata;
	FILE*	     fh_script_output;	// Output from popen()
	bool	     exit_event_loop;
	char*	     buf;	 // Malloc(ed) buffer for stdin
	int	     buf_size;	 // Current size of malloc(ed) buffer
	int	     idx_write;	 // Index of buffer for next write
	int	     status;
	int	     current_pass;
	int	     total_passes;
	mxml_node_t* svg;
	bool	     enable_imgui;
} t_multipass_data;

bool mpEventLoop( void* multipass_data );
bool mpImGuiLoop( void* multipass_data );
int  endMultipassScript( t_multipass_data* multipass_data );
int  pollMultipass( t_multipass_data* multipass_data );
int  startMultipassScript( const char* command, t_multipass_data* multipass_data );
void imgui_create_widgets( t_multipass_data* multipass_data );

#endif /* end of include guard: IG_MULTIPASS */
