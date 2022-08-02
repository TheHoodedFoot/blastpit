#!/usr/bin/tcc -run

// Flexible shadows can be enabled by searching for:
//	0xfffe 0xff00 0x0100 0x0000 0x30
// and replacing with:
//	0xfffe 0xff00 0x0100 0x0000 0xb0

#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#define SEARCH_LEN 9

unsigned char findstring[] = { 0xff, 0xfe, 0xff, 0x00, 0x01, 0x00, 0x00, 0x00, 0x30 };

void
replace_vlm_file(const char *newfile, const char *oldfile)
{
	remove( newfile );
	if( rename( oldfile, newfile ) != 0)
	{
		// Error removing file
		fprintf(stderr, "Could not rename converted file.\n");
	}
}

int
enable_flexible_shadows(FILE *infile, FILE *outfile)
{
	int	      c;
	int	      offset	   = 0;
	int	      num_replaced = 0;
	unsigned char buf[SEARCH_LEN];

	// Fill initial buffer
	for (int i = 0; i < SEARCH_LEN; i++) {
		if ((c = fgetc(infile)) == EOF)
		{
			fprintf(stderr, "Could not fill initial buffer. Exiting.");
			return false;
		}
		buf[i] = c;
		offset++;
	}

	do {
		// Compare string
		int does_match = memcmp(findstring, buf, SEARCH_LEN);
		if (does_match == 0) {
			// Replace
			buf[SEARCH_LEN - 1] = 0xb0;
			fprintf(stderr, "Replaced byte at offset %x\n", offset - 1);
			num_replaced++;
		}

		// Flush first character
		fputc(buf[0], outfile);

		// Rotate bytes
		for (int i = 0; i < SEARCH_LEN - 1; i++) {
			buf[i] = buf[i + 1];
		}

		// Get next character
		c		    = fgetc(infile);
		buf[SEARCH_LEN - 1] = (unsigned char)c;
		offset++;

	} while (c != EOF);

	// Flush and quit
	for (int i = 0; i < SEARCH_LEN - 1; i++) {
		fputc(buf[i], outfile);
	}

	if (num_replaced > 1) {
		fprintf(stderr, "Warning: more than one replacement was made.\n");
	}

	return true;
}

int main(int argc, char *argv[])
{
	(void) argc;
	(void) argv;

	enable_flexible_shadows(stdin, stdout);

	return 0;
}
