#ifndef PSQL_H

#define PSQL_H

#include "libpq-fe.h"

typedef struct
{
	int	  num_rows;
	int	  num_columns;
	char**	  data;	   // This must be freed by user
	PGresult* result;  // So that we can free the result later
} t_db_query;

t_db_query SQLQueryToStruct( PGconn* conn, const char* sql );
PGconn*	   PSQLInit( const char* connection );
PGresult*  PSQLQuery( PGconn* conn, const char* sql_query );
void	   PSQLRelease( PGresult* result );
void	   PSQLExit( PGconn* connection );

#endif /* end of include guard: PSQL_H */
