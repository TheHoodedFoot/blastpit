#ifndef IG_RINGGEN
#define IG_RINGGEN

#include <mxml.h>
#include <stdbool.h>
#include <stdio.h>

#include "ig_common.h"

typedef struct
{
	t_glfw_data  glfwdata;
	FILE*	     fh_script_output;	// Output from popen()
	mxml_node_t* svg;
	bool	     exit_event_loop;
} t_ringgen_data;

bool rgImGuiLoop( void* rg_data );

#endif /* end of include guard: IG_RINGGEN */
