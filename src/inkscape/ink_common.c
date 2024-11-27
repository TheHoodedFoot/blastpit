#include <assert.h>
#include <ctype.h>
#include <fcntl.h>
#include <limits.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/ioctl.h>
#include <time.h>
#include <unistd.h>

// Blastpit
#include <mxml.h>
#include <sds.h>

#include "ink_common.h"


// ░█░█░▀█▀░▀█▀░█░░░▀█▀░▀█▀░▀█▀░█▀▀░█▀▀
// ░█░█░░█░░░█░░█░░░░█░░░█░░░█░░█▀▀░▀▀█
// ░▀▀▀░░▀░░▀▀▀░▀▀▀░▀▀▀░░▀░░▀▀▀░▀▀▀░▀▀▀

void convertToUppercase( sds sds_lowercase )
{
	char* s = sds_lowercase;
	while ( *s ) {
		*s = toupper( (unsigned char)*s );
		s++;
	}
}

void spacesToUnderscores( sds sds_with_spaces )
{
	char* s = sds_with_spaces;
	while ( *s ) {
		if ( *s == ' ' ) {
			*s = '_';
		}
		s++;
	}
}

sds generateRemoteFilename( const char* customer, const char* filename )
{  // Generates a Windows filename compatible with Blastpit

	// Get the current year
	time_t	  t	   = time( NULL );
	struct tm tm	   = *localtime( &t );
	int	  year	   = tm.tm_year + 1900;
	sds	  sds_year = sdsfromlonglong( year );

	// Convert customer to uppercase
	sds sds_upper_customer = sdsnew( customer );
	convertToUppercase( sds_upper_customer );
	spacesToUnderscores( sds_upper_customer );
	sds sds_filename = sdsnew( filename );
	spacesToUnderscores( sds_filename );

	// Combine into filename
	sds sds_out_string = sdsnew( "\"Z:\\\\drawings\\\\" );
	sds_out_string	   = sdscatsds( sds_out_string, sds_year );
	sds_out_string	   = sdscat( sds_out_string, "\\\\" );
	sds_out_string	   = sdscatsds( sds_out_string, sds_upper_customer );
	sds_out_string	   = sdscat( sds_out_string, "\\\\" );
	sds_out_string	   = sdscatsds( sds_out_string, sds_filename );
	sds_out_string	   = sdscat( sds_out_string, ".VLM\"" );

	sdsfree( sds_year );
	sdsfree( sds_upper_customer );
	sdsfree( sds_filename );

	return sds_out_string;
}


// ░█░█░█▄█░█░░
// ░▄▀▄░█░█░█░░
// ░▀░▀░▀░▀░▀▀▀

void dumpXML( const char* filename )
{

	FILE* infile;

	infile = fopen( filename, "r" );
	if ( infile == NULL ) {
		fprintf( stderr, "Failed to open input file (%s).\n", filename );
		exit( 1 );
	}
	char*	linebuf = NULL;	 // malloc(ed) buffer returned by getline()
	size_t	bufsize;	 // Current size of allocated memory
	ssize_t numchars;	 // Count of characters read

	while ( ( numchars = getline( &linebuf, &bufsize, infile ) ) > 0 ) {
		fprintf( stdout, "%s", linebuf );
	}

	fclose( infile );

	if ( linebuf != NULL ) {
		free( linebuf );
	}
}

mxml_node_t* FileToXML( const char* filename )
{  // Convert the SVG to XML

	FILE* svg_in;

	svg_in = fopen( filename, "r" );

	if ( svg_in == NULL ) {
		fprintf( stderr, "Failed to open input file (%s).\n", filename );
		return NULL;
	}

	// MXML 4.0 requires on options object
	mxml_options_t* mxmlopts = mxmlOptionsNew();
	mxml_node_t*	xml	 = mxmlLoadFile( NULL, mxmlopts, svg_in );
	mxmlOptionsDelete( mxmlopts );

	if ( xml == NULL ) {
		fprintf( stderr, "Unable to load SVG as XML.\n" );
		return NULL;
	}

	fclose( svg_in );
	return xml;
}

int findLaserdata( mxml_node_t* xml )
{  // Finds important metadata in the SVG file

	mxml_node_t* node;
	const char*  string;  //, *string1;

	// Find laserdata
	//
	// These are all 'text' elements, with an attribute 'laserdata'

	node = xml;
	do {
		node = mxmlFindElement( node, xml, "text", "inkscape:label", "laserdata", MXML_DESCEND_ALL );

		if ( node ) {
			string = mxmlGetOpaque( node );
			if ( string ) {
				fprintf( stderr, "(opaque string, length %u): %s\n", (int)strlen( string ), string );
			}
		}
	} while ( node );

	return true;
}

int embedLaserdata( mxml_node_t* xml, t_laserdata* laserdata )
{  // Adds or replaces the laser metadata in the SVG

	(void)xml;
	(void)laserdata;

	return false;
}

int convertLaserdata( mxml_node_t* xml, t_laserdata* laserdata )
{  // Reads any existing JSON metadata and embeds as new SVG/XML

	(void)xml;
	(void)laserdata;

	// int	     num_attrs;
	// for ( node = xml; node != NULL; node = mxmlWalkNext( node, xml, MXML_DESCEND ) ) {

	// 	string = mxmlGetElement( node );
	// 	if ( string ) {
	// 		printf( "Element Name: %s\n", string );
	// 	}
	// 	num_attrs = mxmlElementGetAttrCount( node );
	// 	if ( num_attrs > 0 ) {
	// 		for ( int i = 0; i < num_attrs; i++ ) {
	// 			string = mxmlElementGetAttrByIndex( node, i, &string1 );
	// 			printf( "Attribute (%d) (%s): %s\n", i, string1, string );
	// 		}
	// 	}
	// 	string = mxmlGetOpaque(node);
	// 	if(string)
	// 	{
	// 		printf("(opaque string, length %u): %s\n", (int)strlen(string), string);
	// 	}
	// }

	return false;
}

int writeSVGToFile( mxml_node_t* xml, const char* filename )
{
	FILE* svg_out;
	if ( !( svg_out = fopen( filename, "w" ) ) ) {
		return false;
	}
	mxml_options_t* mxmlopts = mxmlOptionsNew();
	mxmlSaveFile( xml, mxmlopts, svg_out );
	mxmlOptionsDelete( mxmlopts );
	fclose( svg_out );
	return true;
}

int getCurrentDirectory( void )
{
	char cwd[PATH_MAX];
	if ( getcwd( cwd, sizeof( cwd ) ) != NULL ) {
		printf( "Current working dir: %s\n", cwd );
	} else {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
		perror( __FUNCTION__ );
#pragma GCC diagnostic pop
		return false;
	}
	return true;
}


// ░█▀█░█▀█░█▀▀░▀█▀░█▀▀░█▀▄░█▀▀░█▀▀░▄▀▄░█░░
// ░█▀▀░█░█░▀▀█░░█░░█░█░█▀▄░█▀▀░▀▀█░█\█░█░░
// ░▀░░░▀▀▀░▀▀▀░░▀░░▀▀▀░▀░▀░▀▀▀░▀▀▀░░▀\░▀▀▀

void pg_exit_nicely( PGconn* conn )
{
	PQfinish( conn );
	exit( 1 );
}

int postgresTest( void )
{
	const char* conninfo;
	PGconn*	    conn;
	PGresult*   res;
	int	    nFields;
	int	    i, j;

	conninfo = "host = localhost, dbname = engraving, connect_timeout = 1";

	/* Make a connection to the database */
	conn = PQconnectdb( conninfo );

	/* Check to see that the backend connection was successfully made */
	if ( PQstatus( conn ) != CONNECTION_OK ) {
		fprintf( stderr, "Connection to database failed: %s", PQerrorMessage( conn ) );
		pg_exit_nicely( conn );
	}

	/* Set always-secure search path, so malicious users can't take control. */
	res = PQexec( conn, "SELECT pg_catalog.set_config('search_path', '', false)" );
	if ( PQresultStatus( res ) != PGRES_TUPLES_OK ) {
		fprintf( stderr, "SET failed: %s", PQerrorMessage( conn ) );
		PQclear( res );
		pg_exit_nicely( conn );
	}

	/*
	 * Should PQclear PGresult whenever it is no longer needed to avoid memory
	 * leaks
	 */
	PQclear( res );

	/*
	 * Our test case here involves using a cursor, for which we must be inside
	 * a transaction block.  We could do the whole thing with a single
	 * PQexec() of "select * from pg_database", but that's too trivial to make
	 * a good example.
	 */

	/* Start a transaction block */
	res = PQexec( conn, "BEGIN" );
	if ( PQresultStatus( res ) != PGRES_COMMAND_OK ) {
		fprintf( stderr, "BEGIN command failed: %s", PQerrorMessage( conn ) );
		PQclear( res );
		pg_exit_nicely( conn );
	}
	PQclear( res );

	/*
	 * Fetch rows from pg_database, the system catalog of databases
	 */
	res = PQexec( conn,
		      "DECLARE myportal CURSOR FOR SELECT name, filepath FROM public.customers ORDER BY name ASC" );
	if ( PQresultStatus( res ) != PGRES_COMMAND_OK ) {
		fprintf( stderr, "DECLARE CURSOR failed: %s", PQerrorMessage( conn ) );
		PQclear( res );
		pg_exit_nicely( conn );
	}
	PQclear( res );

	res = PQexec( conn, "FETCH ALL in myportal" );
	if ( PQresultStatus( res ) != PGRES_TUPLES_OK ) {
		fprintf( stderr, "FETCH ALL failed: %s", PQerrorMessage( conn ) );
		PQclear( res );
		pg_exit_nicely( conn );
	}

	/* first, print out the attribute names */
	nFields = PQnfields( res );
	// for ( i = 0; i < nFields; i++ ) {
	// 	printf( "%-30s", PQfname( res, i ) );
	// }
	// printf( "\n\n" );

	/* next, print out the rows */
	for ( i = 0; i < PQntuples( res ); i++ ) {
		for ( j = 0; j < nFields; j++ ) {
			printf( "%-30s", PQgetvalue( res, i, j ) );
		}
		printf( "\n" );
	}

	PQclear( res );

	/* close the portal ... we don't bother to check for errors ... */
	res = PQexec( conn, "CLOSE myportal" );
	PQclear( res );

	/* end the transaction */
	res = PQexec( conn, "END" );
	PQclear( res );

	/* close the connection to the database and cleanup */
	PQfinish( conn );

	return 0;
}
