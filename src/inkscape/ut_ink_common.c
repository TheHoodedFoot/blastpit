#include <assert.h>

#include "ink_common.h"
#include "mxml.h"
#include "unity_fixture.h"  // MUST be before <stdlib.h>

TEST_GROUP( inkCommonGroup );

TEST_SETUP( inkCommonGroup ) {}

TEST_TEAR_DOWN( inkCommonGroup ) {}

//***********************
//*  Write a test list  *
//***********************

// What is the set of tests which, when passes, demonstrates that the code
// works correctly?

// When writing a test, imagine the perfect interface for that function

// What function do we wish existed?

TEST( inkCommonGroup, utFileToXML )
{
	// What are the requirements to test 'x'?
	// 	What does the object do?
	// 	How does it interact with the data or hardware it controls?
	// 	How can we make it testable?

	TEST_IGNORE();
}

TEST( inkCommonGroup, utGenerateRemoteFilename )
{
	// What are the requirements to test 'x'?
	// 	What does the object do?
	// 	How does it interact with the data or hardware it controls?
	// 	How can we make it testable?

	TEST_IGNORE();
}

TEST( inkCommonGroup, utFindLaserData )
{
	// What are the requirements to test 'x'?
	// 	What does the object do?
	// 	How does it interact with the data or hardware it controls?
	// 	How can we make it testable?

	// Must find all elements with laserdata tag
	// Must successfully parse JSON
	// Must return error if multiple similar tags

	// mxml_node_t* xml = FileToXML( "ut_laserdata.svg" );
	// assert( xml );

	TEST_IGNORE();
}

TEST( inkCommonGroup, utEmbedLaserdata )
{
	// What are the requirements to test 'x'?
	// 	What does the object do?
	// 	How does it interact with the data or hardware it controls?
	// 	How can we make it testable?

	TEST_IGNORE();
}

TEST( inkCommonGroup, utConvertLaserdata )
{
	// What are the requirements to test 'x'?
	// 	What does the object do?
	// 	How does it interact with the data or hardware it controls?
	// 	How can we make it testable?

	// Read test SVG into mxml with FileToXML()

	// O

	TEST_IGNORE();
}


TEST_GROUP_RUNNER( inkCommonGroup )
{  // Add a line below for each unit test

	RUN_TEST_CASE( inkCommonGroup, utFileToXML );
	RUN_TEST_CASE( inkCommonGroup, utGenerateRemoteFilename );
	RUN_TEST_CASE( inkCommonGroup, utFindLaserData );
	RUN_TEST_CASE( inkCommonGroup, utEmbedLaserdata );
	RUN_TEST_CASE( inkCommonGroup, utConvertLaserdata );
}

static void
runAllTests( void )
{
	RUN_TEST_GROUP( inkCommonGroup );
}

int
main( int argc, const char* argv[] )
{
	return UnityMain( argc, argv, runAllTests );
}
