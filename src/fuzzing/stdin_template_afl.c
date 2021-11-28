// Create the input and output directories:
//	mkdir afl_{i,o}

// Create some fuzzing data:
//	ls > afl_i/data1.txt

// Compile with:
//	AFL_HARDEN=1 afl-clang -o stdin_test stdin_test.c

// Fuzz with:
//	afl-fuzz -i afl_i -o afl_o -- stdin_test

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

void
processBuffer(const char* buf)
{ // Perform some processing

	int total = 0;
	for (int i=0; i < (int)sysconf(_SC_PAGESIZE); i++) {
		total += (int)buf[i];
	}
	if (total == 0) {
		assert(total != 0);
	}


}

int main(int argc, char *argv[])
{
	int pSize = (int)sysconf(_SC_PAGESIZE);
	char *buf = calloc(pSize, sizeof(char)); // NOLINT (false error)
	assert(buf);

	FILE *fp = argc > 1 ? fopen (argv[1], "re") : stdin;
	if (!fp) {
		perror ("fopen failed");
		free(buf);
		exit(2);
	}

	while (fgets (buf, pSize, fp)) { // NOLINT
		processBuffer(buf);
	}

	if (fp != stdin) {
		fclose (fp);   // close file if not stdin
	}

	free(buf);

	return 0;
}

