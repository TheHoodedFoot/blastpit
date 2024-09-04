#ifndef IG_DATABASE
#define IG_DATABASE

#include <mxml.h>
#include <stdbool.h>
#include <stdio.h>
#include <time.h>

#include "libpq-fe.h"  // PostgreSQL

#include "ig_common.h"
#include "psql.h"

typedef struct
{
	bool	     exit_event_loop;
	FILE*	     fh_script_output;	// Output from popen()
	mxml_node_t* svg;
	PGconn*	     conn;
	time_t	     last_db_check;  // Time of last connection check
	t_glfw_data  glfwdata;
	t_db_query   customers;
	t_db_query   fonts;
	t_db_query   job;
} t_database_data;

int  testPSQL( void );
bool dbEventLoop( void* rg_data );

#endif /* end of include guard: IG_DATABASE */
