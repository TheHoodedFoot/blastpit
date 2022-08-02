//#!/usr/bin/tcc -run 

#include <stdio.h>
#include <string.h>
#include <math.h>

#include <stdlib.h>

#define NANOSVG_IMPLEMENTATION
#include "../sub/nanosvg/src/nanosvg.h"

#define BUF_SIZE 1024

int main (int argc, char const* argv[])
{
	(void) argc;
	(void) argv;

	char buffer[BUF_SIZE];
	size_t contentSize = 1; // includes NULL

	/* Preallocate space.  */
	char *content = (char*)malloc(sizeof(char) * BUF_SIZE);

	if(content == NULL)
	{
	    perror("Failed to allocate content");
	    exit(1);
	}
	content[0] = '\0'; // make null-terminated

	while(fgets(buffer, BUF_SIZE, stdin))
	{
	    char *old = content;
	    contentSize += strlen(buffer);
	    content = (char*)realloc(content, contentSize);
	    if(content == NULL)
	    {
		perror("Failed to reallocate content");
		free(old);
		exit(2);
	    }
	    strcat(content, buffer);
	}

	if(ferror(stdin))
	{
	    free(content);
	    perror("Error reading from stdin.");
	    exit(3);
	}

	struct NSVGimage* image = nsvgParse(content, "mm", 96);

	for (NSVGshape *shape = image->shapes; shape != NULL; shape = shape->next) {
		printf("id: %s, fill: 0x%06x\n", shape->id,
				((shape->fill.color & 0xff0000) >> 16 ) |
				((shape->fill.color & 0x00ff00)       ) |
				((shape->fill.color & 0x0000ff) << 16 ));
		for (NSVGpath *path = shape->paths; path != NULL; path = path->next) {
			for (int i = 0; i < path->npts-1; i += 3) {
				float* p = &path->pts[i*2];
				printf("%f, %f, %f, %f, %f, %f, %f, %f\n", p[0],p[1], p[2],p[3], p[4],p[5], p[6],p[7]);
			}
		}
		printf("\n");
	}

	nsvgDelete(image);

	free(content);

	return 0;
}


