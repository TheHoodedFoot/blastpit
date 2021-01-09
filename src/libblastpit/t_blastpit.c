#include <unistd.h>
#include "blastpit.h"
#include "sds.h"
#include "websocket.h"
#include "xml.h"

#include "unity_fixture.h"

// This uncontrolled file defines SERVER and MQTT_TIMEOUT
#include "../../res/cfg/t_common.h"

TEST_GROUP(BlastpitGroup);

t_Blastpit *server;

TEST_SETUP(BlastpitGroup)
{
	server = blastpitNew();
	assert(serverCreate(server, WS_PORT) == kSuccess);
}

TEST_TEAR_DOWN(BlastpitGroup)
{
	serverDestroy(server);
	blastpitDelete(server);
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
	TEST_ASSERT_EQUAL(kSuccess, result);

	// Create a client
	t_Blastpit *client = blastpitNew();

	// Try connecting to non-existent server
	// result = connectToServer(client, "ws://localhost:8999", "simpleServerTest", 100);
	// TEST_ASSERT_EQUAL(false, result);

	// Connect to the server
	// We can't test for timeout because the server is not polling
	connectToServer(client, "ws://localhost:8124", 0);

	// Wait for handshake
	for (int i = 0; i < 100; i++) {
		pollMessages(simpleserver);
		pollMessages(client);
		if (bp_isConnected(client))
			break;
	}
	TEST_ASSERT_EQUAL(true, bp_isConnected(client));

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
	connectToServer(client, WS_SERVER_LOCAL, 0);
	for (int i = 0; i < 100; i++) {
		pollMessages(server);
		pollMessages(client);
		if (((t_Websocket *)client->ws)->isConnected)
			break;
	}
	TEST_ASSERT_EQUAL(true, ((t_Websocket *)client->ws)->isConnected);

	// Send a message
	sendClientMessage(client, "test");

	disconnectFromServer(client);
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
	// We can't check the result here because the server is not polling
	connectToServer(client1, "ws://127.0.0.1:8129", 0);
	connectToServer(client2, "ws://127.0.0.1:8129", 0);

	// Wait for handshake
	for (int i = 0; i < 100; i++) {
		pollMessages(simpleserver);
		pollMessages(client1);
		pollMessages(client2);
		if (((t_Websocket *)client1->ws)->isConnected && ((t_Websocket *)client2->ws)->isConnected)
			break;
	}

	TEST_ASSERT_EQUAL(true, ((t_Websocket *)client1->ws)->isConnected);
	TEST_ASSERT_EQUAL(true, ((t_Websocket *)client2->ws)->isConnected);

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
	free((void *)msg1);
	free((void *)msg2);

	// Tidy up
	disconnectFromServer(client1);
	disconnectFromServer(client2);
	serverDestroy(simpleserver);
	blastpitDelete(client1);
	blastpitDelete(client2);
	blastpitDelete(simpleserver);
}

TEST(BlastpitGroup, AutoGenId)
{
	// The first auto-generated id should be 1
	t_Blastpit *bp = blastpitNew();
	TEST_ASSERT_EQUAL(1, AutoGenerateId(bp));

	// The generated ids should be one greater than the previous
	bp->highest_id = 123;
	TEST_ASSERT_EQUAL(124, AutoGenerateId(bp));
	TEST_ASSERT_EQUAL(125, AutoGenerateId(bp));

	blastpitDelete(bp);
}

TEST(BlastpitGroup, SendCommand)
{
	t_Blastpit *bp1 = blastpitNew();
	t_Blastpit *bp2 = blastpitNew();

	connectToServer(bp1, WS_SERVER_LOCAL, 0);
	connectToServer(bp2, WS_SERVER_LOCAL, 0);

	// Wait for handshake
	for (int i = 0; i < 100; i++) {
		pollMessages(server);
		pollMessages(bp1);
		pollMessages(bp2);
		if (bp_isConnected(bp1) && bp_isConnected(bp2))
			break;
	}
	TEST_ASSERT_EQUAL(true, bp_isConnected(bp1));

	IdAck result = BpQueueCommand(bp1, kSelfTest);
	TEST_ASSERT_EQUAL(kSuccess, result.retval);
	result = BpUploadQueuedMessages(bp1);
	TEST_ASSERT_EQUAL(kSuccess, result.retval);
	TEST_ASSERT_EQUAL(1, result.id);

	for (int i = 0; i < 100; i++) {
		pollMessages(bp1);
		pollMessages(bp2);
		pollMessages(server);
		if (getMessageCount(bp2) == 1)
			break;
	}

	TEST_ASSERT_EQUAL(1, getMessageCount(bp2));
	const char *msg1     = popMessage(bp2);
	sds	    selftest = sdscatprintf(
		sdsempty(), "<?xml version=\"1.0\"?><message id=\"1\" type=\"command\" command=\"%d\" ></message>",
		kSelfTest);
	TEST_ASSERT_EQUAL_STRING(selftest, msg1);
	sdsfree(selftest);
	free((void *)msg1);
	// TEST_ASSERT_EQUAL_STRING("<?xml version=\"1.0\"?><message id=\"1\" type=\"command\" command=\"35\"
	// ></message>", msg1);

	TEST_ASSERT_EQUAL(0, getMessageCount(bp1));
	TEST_ASSERT_EQUAL(0, getMessageCount(bp2));
	disconnectFromServer(bp1);
	disconnectFromServer(bp2);
	blastpitDelete(bp1);
	blastpitDelete(bp2);
}

TEST(BlastpitGroup, QueueQpsetTest)
{
	t_Blastpit *bp = blastpitNew();

	// Should reject invalid current, speed, and frequency
	TEST_ASSERT_EQUAL(
		kInvalid,
		BpQueueQpSet(bp, "Low current", LMOS_CURRENT_MIN - 1, LMOS_SPEED_MIN, LMOS_FREQUENCY_MIN).retval);
	TEST_ASSERT_EQUAL(
		kInvalid,
		BpQueueQpSet(bp, "High current", LMOS_CURRENT_MAX + 1, LMOS_SPEED_MIN, LMOS_FREQUENCY_MIN).retval);
	TEST_ASSERT_EQUAL(
		kInvalid,
		BpQueueQpSet(bp, "Low speed", LMOS_CURRENT_MIN, LMOS_SPEED_MIN - 1, LMOS_FREQUENCY_MIN).retval);
	TEST_ASSERT_EQUAL(
		kInvalid,
		BpQueueQpSet(bp, "High speed", LMOS_CURRENT_MIN, LMOS_SPEED_MAX + 1, LMOS_FREQUENCY_MIN).retval);
	TEST_ASSERT_EQUAL(
		kInvalid,
		BpQueueQpSet(bp, "Low frequency", LMOS_CURRENT_MIN, LMOS_SPEED_MIN, LMOS_FREQUENCY_MIN - 1).retval);
	TEST_ASSERT_EQUAL(
		kInvalid,
		BpQueueQpSet(bp, "High frequency", LMOS_CURRENT_MIN, LMOS_SPEED_MIN, LMOS_FREQUENCY_MAX + 1).retval);

	// Should generate correct xml
	TEST_ASSERT_EQUAL(1, BpQueueQpSet(bp, "Qpset1", 50, 50, 20000).id);
	TEST_ASSERT_EQUAL(2, BpQueueQpSet(bp, "Qpset2", 60, 60, 30000).id);
	TEST_ASSERT_EQUAL_STRING(
		"<?xml version=\"1.0\"?><message id=\"1\" type=\"command\" command=\"1\" name=\"Qpset1\" "
		"current=\"50\" speed=\"50\" frequency=\"20000\" ></message><message id=\"2\" depends=\"1\" "
		"type=\"command\" "
		"command=\"1\" name=\"Qpset2\" current=\"60\" speed=\"60\" frequency=\"30000\" ></message>",
		bp->message_queue);

	blastpitDelete(bp);
}

TEST(BlastpitGroup, DependentChildTest)
{  // Checks that queued messages rely on parent

	// Queued messages should have 'depends' attribute added
	// containing the previous messages in the queue, if any

	// 'depends' attribute should be comma separated and sorted numerically.

	// HasNoDependencies() should return true if all of the
	// message dependencies are recorded as successful.
	// Should return false if any dependency failed.

	t_Blastpit *bp = blastpitNew();

	IdAck message1_id = BpQueueMessage(bp, "payload", NULL);
	// This should have an automatic dependency on message1
	IdAck message2_id = BpQueueMessage(bp, "loadpay", NULL);
	// This should depend on both
	IdAck message3_id = BpQueueMessage(bp, "loadpayload", NULL);

	(void)message1_id;
	(void)message2_id;
	(void)message3_id;

	sds message3_data = XmlGetMessageByIndex(bp->message_queue, 2);
	sds attr	  = XmlGetAttribute(message3_data, "depends");
	TEST_ASSERT_EQUAL_STRING("2", attr);

	sdsfree(attr);
	sdsfree(message3_data);
	blastpitDelete(bp);
}

TEST(BlastpitGroup, RetvalDbTest)
{
	// What are the requirements to test 'x'?
	// 	What does the object do?
	// 	How does it interact with the data or hardware it controls?
	// 	How can we make it testable?

	// Must return null if no record of id
	// Must return stored id if present

	t_Blastpit *bp = blastpitNew();
	// RetvalDb db3 = {0, kInvalid, NULL};
	// RetvalDb db2 = {0, kInvalid, &db3};
	// RetvalDb db1 = {0, kInvalid, &db2};
	// bp->retval_db = &db1;

	BpAddRetvalToDb(bp, (IdAck){4, kBadLogic, NULL});
	BpAddRetvalToDb(bp, (IdAck){5, kBadXml, NULL});
	// We need to rig the check against the highest id
	bp->highest_id = 5;

	// Should return kNotFound
	TEST_ASSERT_EQUAL(kNotFound, BpQueryRetvalDb(bp, 1));

	TEST_ASSERT_EQUAL(kBadLogic, BpQueryRetvalDb(bp, 4));
	TEST_ASSERT_EQUAL(kBadXml, BpQueryRetvalDb(bp, 5));

	BpFreeRetvalDb(bp);
	blastpitDelete(bp);
}

TEST(BlastpitGroup, ConnectivityTest)
{
	// What are the requirements to test 'x'?
	// 	What does the object do?
	// 	How does it interact with the data or hardware it controls?
	// 	How can we make it testable?

	t_Blastpit *client = blastpitNew();

	// Connect to the server
	connectToServer(client, WS_SERVER_LOCAL, 0);
	for (int i = 0; i < 100; i++) {
		pollMessages(server);
		pollMessages(client);
		if (((t_Websocket *)client->ws)->isConnected)
			break;
	}

	TEST_ASSERT_EQUAL(true, ((t_Websocket *)client->ws)->isConnected);
	TEST_ASSERT_EQUAL(false, BpIsLmosUp(client));

	disconnectFromServer(client);
	blastpitDelete(client);
}

TEST_GROUP_RUNNER(BlastpitGroup)
{ /* Add a line below for each unit test */

	RUN_TEST_CASE(BlastpitGroup, ConnectivityTest);
	RUN_TEST_CASE(BlastpitGroup, RetvalDbTest);
	RUN_TEST_CASE(BlastpitGroup, DependentChildTest);
	RUN_TEST_CASE(BlastpitGroup, QueueQpsetTest);
	RUN_TEST_CASE(BlastpitGroup, SendCommand);
	RUN_TEST_CASE(BlastpitGroup, simpleServerTest);
	RUN_TEST_CASE(BlastpitGroup, SendAndWaitTest);
	RUN_TEST_CASE(BlastpitGroup, MessageTest);
	RUN_TEST_CASE(BlastpitGroup, AutoGenId);
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
