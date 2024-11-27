#include "psql.h"
#include "libpq-fe.h"
#include <stdlib.h>

// ░█▀█░█▀█░█▀▀░▀█▀░█▀▀░█▀▄░█▀▀░█▀▀░▄▀▄░█░░
// ░█▀▀░█░█░▀▀█░░█░░█░█░█▀▄░█▀▀░▀▀█░█\█░█░░
// ░▀░░░▀▀▀░▀▀▀░░▀░░▀▀▀░▀░▀░▀▀▀░▀▀▀░░▀\░▀▀▀

// USE localtime TO GET DATE

t_db_query SQLQueryToStruct( PGconn* conn, const char* sql )
{
	// Takes a string query which returns a single column and returns a struct containing the number of records and
	// an array of pointers to the record strings const int (*array)[]
	t_db_query query;

	query.result = PSQLQuery( (PGconn*)conn, (const char*)sql );

	query.num_rows	  = PQntuples( query.result );
	query.num_columns = PQnfields( query.result );
	query.data	  = (char**)calloc( query.num_rows * query.num_columns + 1, 8 );

	// Store the pointers to strings returned from PostreSQL to avoid calling PQ functions unneccessarily
	for ( int i = 0; i < query.num_rows; i++ ) {
		for ( int j = 0; j < query.num_columns; j++ ) {
			*( query.data + j + ( i * query.num_columns ) ) = PQgetvalue( query.result, i, j );
		}
	}

	return query;
}

PGconn* PSQLInit( const char* connection )
{  // Creates the initial connection to the database

	PGconn* conn = PQconnectdb( connection );

	/* Check to see that the backend connection was successfully made */
	if ( PQstatus( conn ) != CONNECTION_OK ) {
		fprintf( stderr, "Connection to database failed: %s", PQerrorMessage( conn ) );
		PQfinish( conn );
		return NULL;
	}

	return conn;
}

PGresult* PSQLQuery( PGconn* conn, const char* sql_query )
{  // Sends a query and returns a result

	PGresult* res = PQexec( conn, sql_query );
	if ( PQresultStatus( res ) != PGRES_TUPLES_OK ) {
		fprintf( stderr, "Fetch rows failed: %s", PQerrorMessage( conn ) );
		PQclear( res );
		PQfinish( conn );
		return NULL;
	}

	return res;
}

void PSQLRelease( PGresult* result )
{  // Releases the result

	PQclear( result );
}

void PSQLExit( PGconn* connection )
{  // Shuts down the connection to the database

	PQfinish( connection );
}
