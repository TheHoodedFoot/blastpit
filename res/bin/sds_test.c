#!/usr/bin/tcc -run sds.o

// Build sds.o first

#include <stdio.h>
#include <stdlib.h>
#include "/home/thf/projects/blastpit/src/submodules/sds/sds.h"

int main (int argc, char const* argv[])
{
	sds mystring = sdsnew("Hello World!");
	printf("%s\n", mystring);
	sdsfree(mystring);

	return 0;
}

