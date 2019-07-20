#include <unistd.h>
#include "blastpit.h"
#include "new.h"

#include "unity_fixture.h"

// This uncontrolled file defines SERVER and MQTT_TIMEOUT
#include "t_common.h"

TEST_GROUP(BlastpitGroup);

TEST_SETUP(BlastpitGroup) {}

TEST_TEAR_DOWN(BlastpitGroup) {}

/***********************
 *  Write a test list  *
 ***********************/

/* What is the set of tests which, when passes, demonstrates that the code
 * works correctly? */

/* When writing a test, imagine the perfect interface for that function */

/* What function do we wish existed? */

TEST(BlastpitGroup, SubscribeTest)
{
	/* What are the requirements to test 'x'? */
	/* 	What does the object do? */
	/* 	How does it interact with the data or hardware it controls? */
	/* 	How can we make it testable? */

	t_Blastpit *client = new (Blastpit);
	t_Blastpit *server = new (Blastpit);

	TEST_ASSERT_EQUAL(0,
			  bp_connectToServer(client, SERVER, "client", 2000));
	TEST_ASSERT_EQUAL(0,
			  bp_connectToServer(server, SERVER, "server", 2000));

	/* usleep(MQTT_TIMEOUT); */
	/* TEST_ASSERT_EQUAL(0, bp_subscribe(client, "client", 1000)); */
	TEST_ASSERT_EQUAL(0, bp_subscribe(client, "server", 1000));
	/* TEST_ASSERT_EQUAL(0, bp_subscribe(server, "server", 1000)); */

	/* usleep(MQTT_TIMEOUT); */

	/* Client should be able to see messages sent to both client and
	 * server, but server should only be able to see messages sent to
	 * server */
	TEST_ASSERT_EQUAL(0, bp_sendMessage(client, "client", "message01"));
	/* usleep(MQTT_TIMEOUT); */
	TEST_ASSERT_EQUAL(0, bp_sendMessage(client, "server", "message02"));
	/* usleep(MQTT_TIMEOUT); */
	TEST_ASSERT_EQUAL(0, bp_sendMessage(server, "client", "message03"));
	/* usleep(MQTT_TIMEOUT); */
	TEST_ASSERT_EQUAL(0, bp_sendMessage(server, "server", "message04"));

	usleep(1000000);
	TEST_ASSERT_EQUAL(4, bp_getMessageCount(client));
	TEST_ASSERT_EQUAL(2, bp_getMessageCount(server));

	/* TEST_ASSERT_EQUAL_STRING("message04", bp_getNewestMessage(client));
	 */
	/* TEST_ASSERT_EQUAL(3, bp_getMessageCount(client)); */
	/* TEST_ASSERT_EQUAL_STRING("message03", bp_getNewestMessage(client));
	 */
	/* TEST_ASSERT_EQUAL(2, bp_getMessageCount(client)); */

	bp_unsubscribe(client, "client", 0);
	bp_unsubscribe(client, "server", 0);
	bp_disconnectFromServer(client);
	bp_unsubscribe(server, "server", 0);
	bp_disconnectFromServer(server);
	delete (client);
	delete (server);
}

TEST(BlastpitGroup, ReconnectTest)
{
	t_Blastpit *bp = new (Blastpit);

	TEST_ASSERT_EQUAL(0, bp_connectToServer(bp, SERVER, "test", 2000));
	/* usleep(MQTT_TIMEOUT); */
	TEST_ASSERT_EQUAL(0, bp_subscribe(bp, "ReconnectTest", 1000));
	bp_unsubscribe(bp, "ReconnectTest", 0);
	/* usleep(MQTT_TIMEOUT); */
	/* usleep(MQTT_TIMEOUT); */
	bp_disconnectFromServer(bp);
	/* usleep(MQTT_TIMEOUT); */
	/* usleep(MQTT_TIMEOUT); */

	TEST_ASSERT_EQUAL(0, bp_connectToServer(bp, SERVER, "test", 2000));
	/* usleep(MQTT_TIMEOUT); */
	TEST_ASSERT_EQUAL(0, bp_subscribe(bp, "ReconnectTest", 1000));
	bp_unsubscribe(bp, "ReconnectTest", 0);
	/* usleep(MQTT_TIMEOUT); */
	/* usleep(MQTT_TIMEOUT); */
	bp_disconnectFromServer(bp);
}

TEST(BlastpitGroup, CommandTest)
{
	/* What are the requirements to test 'x'? */
	/* 	What does the object do? */
	/* 	How does it interact with the data or hardware it controls? */
	/* 	How can we make it testable? */

	t_Blastpit *bp = new (Blastpit);

	TEST_ASSERT_EQUAL(
		0, bp_connectToServer(bp, SERVER, "CommandTest", 2000));
	/* usleep(MQTT_TIMEOUT); */
	/* TEST_ASSERT_EQUAL(0, bp_subscribe(bp, "CommandTest", 1000)); */
	/* usleep(MQTT_TIMEOUT); */
	bp_sendCommand(bp, 11, "lmos", 5);
	/* usleep(MQTT_TIMEOUT); */
	/* usleep(MQTT_TIMEOUT); */
	bp_disconnectFromServer(bp);
}

TEST(BlastpitGroup, SendAndWaitTest)
{
	/* What are the requirements to test 'x'? */
	/* 	What does the object do? */
	/* 	How does it interact with the data or hardware it controls? */
	/* 	How can we make it testable? */

	t_Blastpit *bp = new (Blastpit);

	TEST_ASSERT_EQUAL(
		0, bp_connectToServer(bp, SERVER, "sendandwaittest", 2000));
	/* TEST_ASSERT_EQUAL(0, bp_subscribe(bp, "sendandwaittest", 1000)); */
	bp_sendCommand(bp, 12, "lmos", 5);

	/* usleep(MQTT_TIMEOUT); */
	/* usleep(MQTT_TIMEOUT); */
	/* usleep(MQTT_TIMEOUT); */
	/* usleep(MQTT_TIMEOUT); */
	TEST_ASSERT_EQUAL(
		0, bp_sendMessage(bp, "lmos", "test broadcast message"));

	/* We can fake a broadcast reply by simply sending a message on the
	 * broadcast channel */
	TEST_ASSERT_EQUAL(
		9, bp_sendMessageAndWait(bp, 10, "broadcast", "10,9", 2000));
	/* usleep(MQTT_TIMEOUT); */
	/* usleep(MQTT_TIMEOUT); */
	/* usleep(MQTT_TIMEOUT); */
	/* usleep(MQTT_TIMEOUT); */
	bp_disconnectFromServer(bp);
}

TEST_GROUP_RUNNER(BlastpitGroup)
{ /* Add a line below for each unit test */

	RUN_TEST_CASE(BlastpitGroup, SubscribeTest);
	/* RUN_TEST_CASE(BlastpitGroup, CommandTest); */
	/* RUN_TEST_CASE(BlastpitGroup, ReconnectTest); */
	/* RUN_TEST_CASE(BlastpitGroup, SendAndWaitTest); */
}

static void
runAllTests()
{
	RUN_TEST_GROUP(BlastpitGroup);
}

int
stdinTest(int argc, const char *argv[])
{ /* Run tests using stdin as input,
     mostly used with afl fuzzing */

	(void)argc;
	(void)argv;

	return 0;
}

#include <getopt.h>
/* #include <stdio.h> */
/* #include <stdlib.h> */

int
main(int argc, const char *argv[])
{
	while (1) {
		static struct option long_options[] = {
			{"stdin", no_argument, 0, 't'}, {0, 0, 0, 0}};

		int c, option_index = 0;
		c = getopt_long(argc, (char *const *)argv, "t", long_options,
				&option_index);
		if (c == -1) break;

		switch (c) {
			case 't':
				fprintf(stderr, "Running stdin test...\n");
				return stdinTest(argc, argv);

			default:
				break;
		}
	}

	return UnityMain(argc, argv, runAllTests);
}
