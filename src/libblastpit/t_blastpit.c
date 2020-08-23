#include <unistd.h>
#include "blastpit.h"
#include "xml.hpp"

#include "unity_fixture.h"

// This uncontrolled file defines SERVER and MQTT_TIMEOUT
#include "../../res/cfg/t_common.h"

TEST_GROUP(BlastpitGroup);

t_Blastpit *server;

TEST_SETUP(BlastpitGroup)
{
	server = blastpitNew();
	serverCreate(server, "8123");
}

TEST_TEAR_DOWN(BlastpitGroup)
{
	serverDestroy(server);
}

int testval;

void
testcallback()
{
	testval++;
}

TEST(BlastpitGroup, simpleServerTest)
{
	/* What are the requirements to test 'x'? */
	/* 	What does the object do? */
	/* 	How does it interact with the data or hardware it controls? */
	/* 	How can we make it testable? */

	// Allocates memory for t_Blastpit and t_Websocket
	t_Blastpit *simpleserver = blastpitNew();

	// Set the callback
	registerCallback(simpleserver, &testcallback);

	// Sets the t_Websocket address via serverCreate()
	int result = serverCreate(simpleserver, "8124");
	TEST_ASSERT_EQUAL(true, result);

	// Create a client
	t_Blastpit *client = blastpitNew();

	// Try connecting to non-existent server
	// result = connectToServer(client, "ws://localhost:8999", "simpleServerTest", 100);
	// TEST_ASSERT_EQUAL(false, result);

	// Connect to the server
	result = connectToServer(client, "ws://localhost:8124", 100);
	TEST_ASSERT_EQUAL(true, result);

	// Wait for handshake
	for (int i = 0; i < 100; i++) {
		pollMessages(simpleserver);
		pollMessages(client);
		if (client->ws->isConnected)
			break;
	}
	TEST_ASSERT_EQUAL(true, client->ws->isConnected);

	// Send a message
	testval = 0;
	sendClientMessage(client, "test");
	for (int i = 0; i < 100; i++) {
		if (testval)
			break;
		pollMessages(client);
		pollMessages(simpleserver);
	}

	TEST_ASSERT_EQUAL(1, testval);

	// Tidy up
	disconnectFromServer(client);
	serverDestroy(simpleserver);
	blastpitDelete(client);
	blastpitDelete(simpleserver);
}

TEST(BlastpitGroup, SendAndWaitTest)
{
	// Create a client
	t_Blastpit *client = blastpitNew();

	// Connect to the server
	int result = connectToServer(client, "ws://localhost:8123", 1000);
	TEST_ASSERT_EQUAL(true, result);
	TEST_ASSERT_EQUAL(true, client->ws->isConnected);

	// Send a message
	sendClientMessage(client, "test");

	blastpitDelete(client);
}

TEST(BlastpitGroup, MessageTest)
{
	// What are the requirements to test 'x'?
	// 	What does the object do?
	// 	How does it interact with the data or hardware it controls?
	// 	How can we make it testable?

	// Allocates memory for t_Blastpit and t_Websocket
	t_Blastpit *simpleserver = blastpitNew();

	// Sets the t_Websocket address via serverCreate()
	int result = serverCreate(simpleserver, "8129");
	TEST_ASSERT_EQUAL(true, result);

	// Create two clients
	t_Blastpit *client1 = blastpitNew();
	t_Blastpit *client2 = blastpitNew();

	// Connect to the server
	// We can't check the result because the server is not polling
	connectToServer(client1, "ws://127.0.0.1:8129", 0);
	connectToServer(client2, "ws://127.0.0.1:8129", 0);

	// Wait for handshake
	for (int i = 0; i < 100; i++) {
		pollMessages(simpleserver);
		pollMessages(client1);
		pollMessages(client2);
		if (client1->ws->isConnected && client2->ws->isConnected)
			break;
	}

	TEST_ASSERT_EQUAL(true, client1->ws->isConnected);
	TEST_ASSERT_EQUAL(true, client2->ws->isConnected);

	// Send a message
	sendClientMessage(client1, "client1");
	sendClientMessage(client2, "client2");
	for (int i = 0; i < 100; i++) {
		pollMessages(client1);
		pollMessages(client2);
		pollMessages(simpleserver);
		if (getMessageCount(client1) == 1 && getMessageCount(client2) == 1)
			break;
	}

	TEST_ASSERT_EQUAL(1, getMessageCount(client1));
	TEST_ASSERT_EQUAL(1, getMessageCount(client2));

	const char *msg1 = popMessage(client1);
	const char *msg2 = popMessage(client2);
	TEST_ASSERT_EQUAL_STRING("client2", msg1);
	TEST_ASSERT_EQUAL_STRING("client1", msg2);

	// Tidy up
	disconnectFromServer(client1);
	disconnectFromServer(client2);
	serverDestroy(simpleserver);
	blastpitDelete(client1);
	blastpitDelete(client2);
	blastpitDelete(simpleserver);
}

TEST_GROUP_RUNNER(BlastpitGroup)
{ /* Add a line below for each unit test */

	// RUN_TEST_CASE(BlastpitGroup, simpleServerTest);
	// RUN_TEST_CASE(BlastpitGroup, SendAndWaitTest);
	RUN_TEST_CASE(BlastpitGroup, MessageTest);
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
		static struct option long_options[] = {{"stdin", no_argument, 0, 't'}, {0, 0, 0, 0}};

		int c, option_index = 0;
		c = getopt_long(argc, (char *const *)argv, "tv", long_options, &option_index);
		if (c == -1)
			break;

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
