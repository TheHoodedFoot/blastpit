#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int
main()
{
	char  line[100];
	char* token;
	char* delim = ",";
	int   integer;
	char  string[100];

	// Test case 1: input line is empty
	strcpy( line, "" );
	token = strtok( line, delim );
	assert( token == NULL );

	// Test case 2: input line contains only a comma
	strcpy( line, "," );
	token = strtok( line, delim );
	assert( token == NULL );

	// Test case 3: input line contains a comma and a string
	strcpy( line, "hello,world" );
	token = strtok( line, delim );
	assert( token != NULL );
	assert( strcmp( token, "hello" ) == 0 );
	token = strtok( NULL, delim );
	assert( token != NULL );
	assert( strcmp( token, "world" ) == 0 );

	// Test case 4: input line contains a comma and an integer
	strcpy( line, "123,456" );
	token = strtok( line, delim );
	assert( token != NULL );
	assert( atoi( token ) == 123 );
	token = strtok( NULL, delim );
	assert( token != NULL );
	assert( atoi( token ) == 456 );

	return 0;
}
