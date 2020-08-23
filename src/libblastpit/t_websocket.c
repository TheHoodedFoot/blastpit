#include "blastpit.h"

#include "websocket.h"

#include "unity_fixture.h"  // MUST be before <stdlib.h>

static t_Websocket *server;

TEST_GROUP(WebsocketGroup);

TEST_SETUP(WebsocketGroup)
{
	server = websocketNew();
	wsServerCreate(server, "8001");
	wsPoll(server);
}

TEST_TEAR_DOWN(WebsocketGroup)
{
	wsPoll(server);
	wsServerDestroy(server);
	websocketDelete(server);
}

TEST(WebsocketGroup, wsServerNew)
{
	t_Websocket *ws_server = websocketNew();

	int result = wsServerCreate(ws_server, "8000");
	TEST_ASSERT_EQUAL(true, result);

	// Should be filled in by constructor
	TEST_ASSERT_EQUAL(true, ws_server->evloopIsRunning);
	TEST_ASSERT_NOT_NULL(ws_server->connection);

	wsServerDestroy(ws_server);
	websocketDelete(ws_server);
}

TEST(WebsocketGroup, PortInUse)
{
	// Should fail with port in use by &server;
	t_Websocket *ws_server = websocketNew();
	int result = wsServerCreate(ws_server, "8001");
	TEST_ASSERT_EQUAL(kAllocationFailure, result);

	websocketDelete(ws_server);
}

int testval;

void
testcallback()
{
	testval++;
}

TEST(WebsocketGroup, wsListen)
{
	t_Websocket *ws_server = websocketNew();

	// Set the callback
	ws_server->messageReceived = &testcallback;

	int result = wsServerCreate(ws_server, "8002");
	TEST_ASSERT_EQUAL(true, result);

	// Should be filled in by constructor
	TEST_ASSERT_EQUAL(true, ws_server->evloopIsRunning);
	TEST_ASSERT_NOT_NULL(ws_server->connection);

	// Create a client
	t_Websocket *ws_client = websocketNew();
	result = wsClientCreate(ws_client, "ws://localhost:8002");
	TEST_ASSERT_EQUAL(true, result);

	ws_client->messageReceived = &testcallback;

	// Wait for connection
	for (int i = 0; i < 100; i++) {
		wsPoll(ws_server);
		wsPoll(ws_client);
		if (ws_client->isConnected)
			break;
	}
	TEST_ASSERT_EQUAL(true, ws_client->isConnected);

	// Send a client message
	testval = 0;
	broadcastClient(ws_client->connection, mg_mk_str("wsPoll"));

	// Check that the message arrived
	for (int i = 0; i < 5; i++) {
		if (testval)
			break;
		wsPoll(ws_server);
		wsPoll(ws_client);
	}

	TEST_ASSERT_EQUAL(1, testval);

	wsServerDestroy(ws_server);
	websocketDelete(ws_client);
	websocketDelete(ws_server);
}

TEST(WebsocketGroup, wsIgnore)
{
	// Should refuse new requests
	// Should close port
	// Should free port for reuse

	// TEST_FAIL();
}

TEST(WebsocketGroup, wsBroadcastToClients)
{
	// Should reject invalid message
	// Should broadcast to all attached clients

	// Create a client
	t_Websocket *ws_client = websocketNew();
	wsClientCreate(ws_client, "ws://localhost:8001");

	// Wait for connection
	for (int i = 0; i < 100; i++) {
		wsPoll(ws_client);
		wsPoll(server);
		if (ws_client->isConnected)
			break;
	}

	// Send a client message
	testval = 0;
	broadcastClient(ws_client->connection, mg_mk_str("wsBroadcastToClients"));

	// Fake event loop
	for (int i = 0; i < 5; i++) {
		if (testval)
			break;
		wsPoll(server);
		wsPoll(ws_client);
	}

	wsClientDestroy(ws_client);
	websocketDelete(ws_client);
}

TEST(WebsocketGroup, StackTest)
{
	// What are the requirements to test 'x'?
	// 	What does the object do?
	// 	How does it interact with the data or hardware it controls?
	// 	How can we make it testable?

	const char *lorem = "Lorem ipsum";
	const char *ipsum = "Ipsum lorem";

	t_Websocket *ws = websocketNew();

	TEST_ASSERT_NULL(ws->messageStack);
	wsPushMessage(ws, (void *)lorem);
	TEST_ASSERT_EQUAL(ws->messageStack->data, lorem);
	TEST_ASSERT_NOT_NULL(ws->messageStack);
	TEST_ASSERT_NULL(ws->messageStack->next);
	wsPushMessage(ws, (void *)ipsum);
	TEST_ASSERT_EQUAL(2, wsGetMessageCount(ws));
	const char *text = (const char *)wsPopMessage(ws);
	TEST_ASSERT_EQUAL(ipsum, text);
	fprintf(stderr, "Popped text: <%s>\n", text);
	TEST_ASSERT_EQUAL(1, wsGetMessageCount(ws));
	text = (const char *)wsPopMessage(ws);
	TEST_ASSERT_EQUAL(lorem, text);
	fprintf(stderr, "Popped text: <%s>\n", text);
}

TEST_GROUP_RUNNER(WebsocketGroup)
{  // Add a line below for each unit test

	RUN_TEST_CASE(WebsocketGroup, wsListen);
	RUN_TEST_CASE(WebsocketGroup, wsBroadcastToClients);
	RUN_TEST_CASE(WebsocketGroup, StackTest);
}

static void
runAllTests()
{
	RUN_TEST_GROUP(WebsocketGroup);
}

int
main(int argc, const char *argv[])
{
	return UnityMain(argc, argv, runAllTests);
}
