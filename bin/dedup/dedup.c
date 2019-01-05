#include <stdio.h>
#include <stdlib.h>

struct element {
	double x1, y1;
	double x2, y2;
	double x3, y3;
};

int breakOverlappingLines( struct element (*elements)[] )
{
	// Detect any overlapping lines and divide them up
	// at the intersection points
	
	return 0;
}

int breakOverlappingArcs( struct element (*elements)[] )
{
	// Detect any overlapping arcs and divide them up
	// at the intersection points
	
	return 0;
}

int sortFunction( const void * a, const void * b )
{
	return 0;
}

int sortElements( struct element (*elements)[] )
{
	qsort( elements, 5, sizeof( struct element ), sortFunction );
	return 0;
}

int removeSimpleDuplicates( struct element (*elements)[] )
{
	// Just remove any obvious duplicates
	return 0;
}

int joinLines( struct element (*elements)[] )
{
	// Join consecutive lines into a single line
	return 0;
}

int joinArcs( struct element (*elements)[] )
{
	// Join consecutive arcs into a single arc
	return 0;
}

int dedup( struct element (*elements)[] )
{
	return 1;
}

int main(int argc, char *argv[])
{
	// Test data
	struct element elements[] = {
		{ 0, 0, 0, 0, 1, 1 },
		{ 0, 0, 0, 0, 1, 1 },
		{ 1, 2, 3, 4, 5, 6 },
		{ 7, 8, 9, 9, 8, 7 },
		{ 8, 7, 9, 9, 7, 8 }
	};

	int result = dedup( &elements );

	return result;
}
