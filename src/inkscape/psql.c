#include "psql.h"
#include "libpq-fe.h"
#include <stdlib.h>

// ░█▀█░█▀█░█▀▀░▀█▀░█▀▀░█▀▄░█▀▀░█▀▀░▄▀▄░█░░
// ░█▀▀░█░█░▀▀█░░█░░█░█░█▀▄░█▀▀░▀▀█░█\█░█░░
// ░▀░░░▀▀▀░▀▀▀░░▀░░▀▀▀░▀░▀░▀▀▀░▀▀▀░░▀\░▀▀▀

// USE localtime TO GET DATE

t_db_query
SQLQueryToStruct( PGconn* conn, const char* sql )
{
	// Takes a string query which returns a single column and returns a struct containing the number of records and
	// an array of pointers to the record strings const int (*array)[]
	t_db_query query;

	query.result = PSQLQuery( (PGconn*)conn, (const char*)sql );

	query.num_rows	  = PQntuples( query.result );
	query.num_columns = PQnfields( query.result );
	// printf( "num_rows: %d, num_columns: %d", query.num_rows, query.num_columns );
	// printf( "nmemb: %d, size: %lu\n", query.num_rows * query.num_columns, sizeof( const char* ) );
	query.data = (char**)calloc( query.num_rows * query.num_columns + 1, 8 );
	// printf( "data start: %p\n", query.data );
	// printf( "data should end: %p\n", query.data + ( query.num_rows * query.num_columns * 8 ) );

	// Store the pointers to strings returned from PostreSQL to avoid calling PQ functions unneccessarily
	for ( int i = 0; i < query.num_rows; i++ ) {
		for ( int j = 0; j < query.num_columns; j++ ) {
			// printf( "query.data = %p, i = %d, j = %d, j * s = %d, i * s * query.num_columns = %d, total =
			// "
			// 	"%p\n",
			// 	query.data,
			// 	i,
			// 	j,
			// 	j * s,
			// 	i * s * query.num_columns,
			//
			// 	( query.data + j + ( i * query.num_columns ) ) );
			// printf( " j * s = %d, i * s * query.num_columns = %d\n", j * s, i * s * query.num_columns );
			*( query.data + j + ( i * query.num_columns ) ) = PQgetvalue( query.result, i, j );
		}
	}

	// Test the above
	// for ( int i = 0; i < query.num_rows; i++ ) {
	// 	for ( int j = 0; j < query.num_columns; j++ ) {
	// 		// 		// *( query.data + i + ( j * query.num_columns ) ) = PQgetvalue( result, i, j );
	// 		// 		// printf( "(%p) %-30s", PQgetvalue( result, i, j ), PQgetvalue( result, i, j )
	// 		// );
	// 		printf( "(%p) %-30s",
	// 			*( query.data + ( j * s ) + ( i * s * query.num_columns ) ),
	// 			*( query.data + ( j * s ) + ( i * s * query.num_columns ) ) );
	// 		// 			*( query.data + i + ( j * query.num_columns ) ),
	// 		// 			*( query.data + i + ( j * query.num_columns ) ) );
	// 	}
	// 	printf( "\n" );
	// }

	return query;
}

PGconn*
PSQLInit( const char* connection )
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

PGresult*
PSQLQuery( PGconn* conn, const char* sql_query )
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

void
PSQLRelease( PGresult* result )
{  // Releases the result

	PQclear( result );
};

void
PSQLExit( PGconn* connection )
{  // Shuts down the connection to the database

	PQfinish( connection );
}


// int
// testPSQL( void )
// {
// 	const char* conninfo;
// 	PGconn*	    conn;
// 	PGresult*   res;
// 	int	    nFields;
// 	int	    i, j;
//
// 	conninfo = "host = hilly, dbname = engraving";
//
// 	/* Make a connection to the database */
// 	conn = PQconnectdb( conninfo );
//
// 	/* Check to see that the backend connection was successfully made */
// 	if ( PQstatus( conn ) != CONNECTION_OK ) {
// 		fprintf( stderr, "Connection to database failed: %s", PQerrorMessage( conn ) );
// 		PQfinish( conn );
// 		return -1;
// 	}
//
// 	/*
// 	 * Fetch rows from pg_database, the system catalog of databases
// 	 */
// 	res = PQexec( conn, "SELECT name, filepath FROM customers ORDER BY name ASC" );
// 	if ( PQresultStatus( res ) != PGRES_TUPLES_OK ) {
// 		fprintf( stderr, "Fetch rows failed: %s", PQerrorMessage( conn ) );
// 		PQclear( res );
// 		PQfinish( conn );
// 		return -1;
// 	}
//
// 	nFields = PQnfields( res );
//
// 	// Calculate and allocate enough memory for the array
//
// 	/* next, print out the rows */
// 	for ( i = 0; i < PQntuples( res ); i++ ) {
// 		for ( j = 0; j < nFields; j++ ) {
// 			printf( "%-30s", PQgetvalue( res, i, j ) );
// 		}
// 		printf( "\n" );
// 	}
//
// 	PQclear( res );
//
// 	/* close the connection to the database and cleanup */
// 	PQfinish( conn );
//
// 	return 0;
// }
