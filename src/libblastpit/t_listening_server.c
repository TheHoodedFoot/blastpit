#include "blastpit.h"

#include "unity_fixture.h" /* MUST be before <stdlib.h> */

TEST_GROUP(ListeningServerGroup);

TEST_SETUP(ListeningServerGroup) {}

TEST_TEAR_DOWN(ListeningServerGroup) {}

/***********************
 *  Write a test list  *
 ***********************/

/* What is the set of tests which, when passes, demonstrates that the code
 * works correctly? */

/* When writing a test, imagine the perfect interface for that function */

/* What function do we wish existed? */

TEST(ListeningServerGroup, CreateServer)
{
	/* What are the requirements to test 'x'? */
	/* 	What does the object do? */
	/* 	How does it interact with the data or hardware it controls? */
	/* 	How can we make it testable? */

	t_Blastpit *server = blastpitNew();
	TEST_ASSERT_EQUAL(true, serverCreate(server, "8003"));
	serverDestroy(server);
	blastpitDelete(server);
}


TEST_GROUP_RUNNER(ListeningServerGroup)
{ /* Add a line below for each unit test */

	RUN_TEST_CASE(ListeningServerGroup, CreateServer);
}

static void
runAllTests()
{
	RUN_TEST_GROUP(ListeningServerGroup);
}

int
main(int argc, const char *argv[])
{
	return UnityMain(argc, argv, runAllTests);
}
