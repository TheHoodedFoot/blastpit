#include <stdio.h>
#include <stdlib.h>

#define OUTPUT_FILE "/tmp/imgui_test.txt"

int main( int argc, char* argv[] )
{
	FILE* outfile;

	outfile = fopen( OUTPUT_FILE, "w" );
	if ( outfile == NULL ) {
		fprintf( stderr, "Failed to open output file.\n" );
		exit( 1 );
	}

	char*	linebuf = NULL;	 // malloc(ed) buffer returned by getline()
	size_t	bufsize;	 // Current size of allocated memory
	ssize_t numchars;	 // Count of characters read

	while ( ( numchars = getline( &linebuf, &bufsize, stdin ) ) > 0 ) {
		fprintf( outfile, "(%d) %s", (int)numchars, linebuf );
	}

	fclose( outfile );

	if ( linebuf != NULL ) {
		free( linebuf );
	}

	return 0;
}
