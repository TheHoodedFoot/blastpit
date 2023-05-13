#include "blastpit.h"
#include "unity_fixture.h"  // MUST be before <stdlib.h>
#include "websocket.h"

static t_Websocket* server;

int countServerMessagesReceived;
int countClientMessagesReceived;

void
serverMessageReceivedCallback( void* cb1, void* cb2 )
{
	(void)cb1;
	(void)cb2;

	// A simple callback that increments a counter.
	countServerMessagesReceived++;
}

void
clientMessageReceivedCallback( void* cb1, void* cb2 )
{
	(void)cb1;
	(void)cb2;

	countClientMessagesReceived++;
}

TEST_GROUP( WebsocketGroup );

TEST_SETUP( WebsocketGroup )
{

	server			= websocketNew();
	server->messageReceived = &serverMessageReceivedCallback;
	wsServerCreate( server, "ws://127.0.0.1:8001" );
	wsPoll( server );
}

TEST_TEAR_DOWN( WebsocketGroup )
{
	wsPoll( server );
	wsServerDestroy( server );
	websocketDelete( server );

	// Allow time for the listening server to free the port
	// usleep(100000);
}

TEST( WebsocketGroup, wsServerNew )
{
	t_Websocket* ws_server = websocketNew();

	int result = wsServerCreate( ws_server, "ws://127.0.0.1:8002" );
	TEST_ASSERT_EQUAL( true, result );

	// Should be filled in by constructor
	TEST_ASSERT_EQUAL( true, ws_server->evloopIsRunning );
	TEST_ASSERT_NOT_NULL( ws_server->connection );

	wsServerDestroy( ws_server );
	websocketDelete( ws_server );
}

TEST( WebsocketGroup, PortInUse )
{
	// TEST_IGNORE();

	// Should fail with port in use by &server;
	t_Websocket* ws_server = websocketNew();
	int	     result    = wsServerCreate( ws_server, "ws://127.0.0.1:8001" );
	TEST_ASSERT_EQUAL( kAllocationFailure, result );

	wsServerDestroy( ws_server );
	websocketDelete( ws_server );
}

TEST( WebsocketGroup, wsListen )
{
	t_Websocket* ws_server = websocketNew();

	// Set the callback
	ws_server->messageReceived = &serverMessageReceivedCallback;

	int result = wsServerCreate( ws_server, "ws://127.0.0.1:8003" );
	TEST_ASSERT_EQUAL( true, result );

	// Should be filled in by constructor
	TEST_ASSERT_EQUAL( true, ws_server->evloopIsRunning );
	TEST_ASSERT_NOT_NULL( ws_server->connection );

	// Create a client
	t_Websocket* ws_client = websocketNew();
	result		       = wsClientCreate( ws_client, "ws://127.0.0.1:8003" );
	TEST_ASSERT_EQUAL( kSuccess, result );

	// ws_client->messageReceived = &clientMessageReceivedCallback;

	// Wait for connection
	for ( int i = 0; i < 100; i++ ) {
		wsPoll( ws_server );
		wsPoll( ws_client );
		if ( ws_client->isConnected ) {
			break;
		}
	}
	TEST_ASSERT_EQUAL( true, ws_client->isConnected );

	// Send a client message
	countServerMessagesReceived = 0;
	broadcastClient( ws_client->connection, mg_str( "wsPoll" ) );

	// Check that the message arrived
	for ( int i = 0; i < 5; i++ ) {
		wsPoll( ws_server );
		wsPoll( ws_client );
	}

	TEST_ASSERT_EQUAL( 1, countServerMessagesReceived );

	wsClientDestroy( ws_client );
	wsServerDestroy( ws_server );
	websocketDelete( ws_client );
	websocketDelete( ws_server );
}

TEST( WebsocketGroup, wsBroadcastToClients )
{
	// Should reject invalid message
	// Should broadcast to all attached clients

	// Create a client
	t_Websocket* ws_client_sender	= websocketNew();
	t_Websocket* ws_client_receiver = websocketNew();
	wsClientCreate( ws_client_sender, "ws://127.0.0.1:8001" );
	wsClientCreate( ws_client_receiver, "ws://127.0.0.1:8001" );

	// Wait for connection
	for ( int i = 0; i < 100; i++ ) {
		wsPoll( ws_client_sender );
		wsPoll( ws_client_receiver );
		wsPoll( server );
		if ( ws_client_sender->isConnected && ws_client_receiver->isConnected ) {
			break;
		}
	}

	// Send a client message
	countServerMessagesReceived = 0;
	broadcastClient( ws_client_sender->connection, mg_str( "wsBroadcastToClients" ) );

	// Fake event loop
	for ( int i = 0; i < 5; i++ ) {
		if ( countServerMessagesReceived ) {
			break;
		}
		wsPoll( server );
		wsPoll( ws_client_sender );
		wsPoll( ws_client_receiver );
		usleep( 10000 );
	}

	// TEST_ASSERT_EQUAL(1, wsGetMessageCount(ws_client_receiver));
	TEST_ASSERT_EQUAL( 0, wsGetMessageCount( ws_client_sender ) );

	wsClientDestroy( ws_client_sender );
	websocketDelete( ws_client_sender );
	wsClientDestroy( ws_client_receiver );
	websocketDelete( ws_client_receiver );
}

TEST( WebsocketGroup, StackTest )
{
	// What are the requirements to test 'x'?
	// 	What does the object do?
	// 	How does it interact with the data or hardware it controls?
	// 	How can we make it testable?

	const char* lorem = "Lorem ipsum";
	const char* ipsum = "Ipsum lorem";

	t_Websocket* ws = websocketNew();

	TEST_ASSERT_NULL( ws->messageStack );
	wsPushMessage( ws, (void*)lorem );
	TEST_ASSERT_EQUAL( ws->messageStack->data, lorem );
	TEST_ASSERT_NOT_NULL( ws->messageStack );
	TEST_ASSERT_NULL( ws->messageStack->next );
	wsPushMessage( ws, (void*)ipsum );
	TEST_ASSERT_EQUAL( 2, wsGetMessageCount( ws ) );
	const char* text = (const char*)wsPopMessage( ws );
	TEST_ASSERT_EQUAL( ipsum, text );
	TEST_ASSERT_EQUAL( 1, wsGetMessageCount( ws ) );
	text = (const char*)wsPopMessage( ws );
	TEST_ASSERT_EQUAL( lorem, text );

	websocketDelete( ws );
}

void
updateHash( void* ev_data, void* object )
{  // Add the message data to the existing hash

	(void)object;
	(void)ev_data;

	// struct websocket_message *wm = (struct websocket_message *)ev_data;
	countServerMessagesReceived = true;
	// retval.size = (int)wm->size;
	// retval.data = wm->data;
}

TEST( WebsocketGroup, DataTransferTest )
{
	// Should be able to transfer large amount of data between two clients

	// Data should not be corrupted

	// Create clients
	t_Websocket* ws_client_sender	= websocketNew();
	t_Websocket* ws_client_receiver = websocketNew();
	wsClientCreate( ws_client_sender, "ws://127.0.0.1:8001" );
	ws_client_receiver->messageReceived = &updateHash;
	wsClientCreate( ws_client_receiver, "ws://127.0.0.1:8001" );

	// Wait for connection
	for ( int i = 0; i < 100; i++ ) {
		wsPoll( ws_client_sender );
		wsPoll( ws_client_receiver );
		wsPoll( server );
		if ( ws_client_sender->isConnected && ws_client_receiver->isConnected ) {
			break;
		}
	}
	TEST_ASSERT_EQUAL( true, ws_client_sender->isConnected );
	TEST_ASSERT_EQUAL( true, ws_client_receiver->isConnected );

	// Send a client message
	// This has been tested successfully up to 5000000
	{
#define BSIZE 10000
		char* string = (char*)calloc( BSIZE, 1 );
		TEST_ASSERT_NOT_NULL( string );
		// for (int i = 0; i < BSIZE; i++)
		// 	*(string + i) = 'A';
		// *(string + BSIZE) = 0;
		broadcastClient( ws_client_sender->connection, mg_str_n( string, BSIZE ) );
		free( string );
	}

	// Fake event loop
	countServerMessagesReceived = false;
	for ( int i = 0; i < BSIZE / 100; i++ ) {
		wsPoll( server );
		wsPoll( ws_client_sender );
		wsPoll( ws_client_receiver );
		if ( countServerMessagesReceived ) {
			break;
		}
		usleep( 1000 );
	}

	TEST_ASSERT_EQUAL( true, countServerMessagesReceived );

	wsClientDestroy( ws_client_sender );
	wsClientDestroy( ws_client_receiver );
	websocketDelete( ws_client_sender );
	websocketDelete( ws_client_receiver );
}

TEST_GROUP_RUNNER( WebsocketGroup )
{  // Add a line below for each unit test

	RUN_TEST_CASE( WebsocketGroup, DataTransferTest );
	RUN_TEST_CASE( WebsocketGroup, wsListen );
	RUN_TEST_CASE( WebsocketGroup, wsBroadcastToClients );
	RUN_TEST_CASE( WebsocketGroup, StackTest );
	RUN_TEST_CASE( WebsocketGroup, wsServerNew );
	RUN_TEST_CASE( WebsocketGroup, PortInUse );
}

static void
runAllTests( void )
{
	RUN_TEST_GROUP( WebsocketGroup );
}

int
main( int argc, const char* argv[] )
{
	return UnityMain( argc, argv, runAllTests );
}
