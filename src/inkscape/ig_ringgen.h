#ifndef IG_RINGGEN
#define IG_RINGGEN

#include <mxml.h>
#include <stdbool.h>
#include <stdio.h>

#include "ig_common.h"

typedef struct
{
	mxml_node_t* svg;
	FILE*	     fh_script_output;	// Output from popen()
	bool	     exit_event_loop;
	t_glfw_data  glfwdata;
} t_ringgen_data;

bool rgImGuiLoop( void* rg_data );

#endif /* end of include guard: IG_RINGGEN */
