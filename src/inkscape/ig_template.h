// This header is specific to the program, however all of the
// structs will contain the glfwdata and exit_event_loop

#ifndef IG_TEMPLATE
#define IG_TEMPLATE

#include <mxml.h>
#include <stdbool.h>
#include <stdio.h>

// Common
#include "ig_common.h"

#include "libpq-fe.h"  // PostgreSQL
#include "psql.h"

typedef struct
{
	// Common
	bool	    exit_event_loop;
	t_glfw_data glfwdata;

	// App-specific
	mxml_node_t*	svg;
	FILE*		fh_script_output;  // Output from popen()
	PGconn*		conn;
	t_db_query	customers;
	t_db_query	fonts;
	t_db_query	job;
	ImNodesContext* inctx;	// State for imnodes
} t_template_data;

int  testPSQL( void );
bool dbEventLoop( void* rg_data );

#endif /* end of include guard: IG_TEMPLATE */
