#ifndef IG_CIMNODES
#define IG_CIMNODES

#include <mxml.h>
#include <stdbool.h>
#include <stdio.h>

#include "ig_common.h"

#include "cimnodes.h"

typedef struct
{
	mxml_node_t*	svg;
	FILE*		fh_script_output;  // Output from popen()
	bool		exit_event_loop;
	t_glfw_data	glfwdata;
	ImNodesContext* inctx;	// State for imnodes
} t_cimnodes_data;

bool cimImGuiLoop( void* cim_data );

#endif /* end of include guard: IG_CIMNODES */
