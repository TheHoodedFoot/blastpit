#!/usr/bin/tcc -run -lm

#include <stdio.h>
#include <math.h>
#include <unistd.h>

#define NANOSVG_IMPLEMENTATION
#include "../sub/nanosvg/src/nanosvg.h"

int main(int argc, char *argv[])
{

        // Load
        if (access(argv[1], R_OK) != 0) return 1;
        struct NSVGimage* image;
        image = nsvgParseFromFile(argv[1], "px", 96);
        printf("size: %f x %f\n", image->width, image->height);
        // Use...
        NSVGshape *shape;
        NSVGpath *path;
        int i;
        for (shape = image->shapes; shape != NULL; shape = shape->next) {
                for (path = shape->paths; path != NULL; path = path->next) {
                        for (i = 0; i < path->npts-1; i += 3) {
                                float* p = &path->pts[i*2];
                                printf( "%f %f %f %f %f %f %f %f\n", p[0], p[1], p[2], p[3], p[4], p[5], p[6], p[7]);
                        }
                }
        }
        // Delete
        nsvgDelete(image);

        return 0;
}
