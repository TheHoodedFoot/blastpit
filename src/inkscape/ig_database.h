#ifndef IG_DATABASE
#define IG_DATABASE

#include <mxml.h>
#include <stdbool.h>
#include <stdio.h>

#include "libpq-fe.h"  // PostgreSQL

#include "ig_common.h"
#include "psql.h"

typedef struct
{
	mxml_node_t* svg;
	FILE*	     fh_script_output;	// Output from popen()
	bool	     exit_event_loop;
	t_glfw_data  glfwdata;
	PGconn*	     conn;
	t_db_query   customers;
	t_db_query   fonts;
	t_db_query   job;
} t_database_data;

int  testPSQL( void );
bool dbEventLoop( void* rg_data );

#endif /* end of include guard: IG_DATABASE */
