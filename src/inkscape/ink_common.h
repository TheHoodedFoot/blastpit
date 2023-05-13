#ifndef INK_COMMON
#define INK_COMMON

#include "GLFW/glfw3.h"
#include "ig_common.h"
#include <libpq-fe.h>
#include <mxml.h>
#include <sds.h>
#include <stdbool.h>

typedef struct
{
	sds customer;
	sds filename;
} t_laserdata;

int	     getCurrentDirectory( void );
int	     postgresTest( void );
mxml_node_t* FileToXML( const char* filename );
void	     doEventLoop( t_glfw_data* glfwdata, void* self, bool ( *headlessCallback )( void* ) );
void	     dumpXML( const char* filename );
sds	     generateRemoteFilename( const char* customer, const char* filename );
void	     pg_exit_nicely( PGconn* conn );

#endif /* end of include guard: INK_COMMON */
