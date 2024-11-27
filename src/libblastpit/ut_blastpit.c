#include <assert.h>
#include <getopt.h>
#include <stdlib.h>
#include <unistd.h>

#include "blastpit.h"
#include "sds.h"
#include "unity.h"
#include "websocket.h"
#include "xml.h"

// This uncontrolled file defines SERVER and MQTT_TIMEOUT
#include "t_common.h"

t_Blastpit* server;

void testSetup( void )
{
	BPLOG( kLvlDebug, "%s: Creating server\n", __func__ );
	server	   = blastpitNew();
	int result = serverCreate( server, WS_SERVER_TEST ) == kSuccess;
	(void)result;
	assert( result );
	BPLOG( kLvlDebug, "%s: Server created\n", __func__ );
	BPLOG( kLvlDebug, "Server nc: %p\n", (void*)( (t_Websocket*)server->ws )->connection );
}

void testTeardown( void )
{
	BPLOG( kLvlDebug, "%s: Destroying server\n", __func__ );
	serverDestroy( server );
	blastpitDelete( server );
	BPLOG( kLvlDebug, "%s: Server destroyed\n", __func__ );
}

int testval;

void testcallback( void* cb1, void* cb2 )
{
	(void)cb1;
	(void)cb2;

	testval++;
}

/*! \file  blastpit.h
    \brief Brief description

    Long description
*/

/*! \fn int BpGetMessageCount( const char* xml )
    \brief Brief description
    \param xml The XML
    \return Returns an integer

    This is the long text
*/

void simpleServerTest( void )
{
	/* What are the requirements to test 'x'? */
	/* 	What does the object do? */
	/* 	How does it interact with the data or hardware it controls? */
	/* 	How can we make it testable? */

	// Allocates memory for t_Blastpit and t_Websocket
	t_Blastpit* simpleserver = blastpitNew();

	// Set the callback
	registerCallback( simpleserver, &testcallback );

	// Sets the t_Websocket address via serverCreate()
	int result = serverCreate( simpleserver, "ws://127.0.0.1:8124" );
	TEST_ASSERT_EQUAL( kSuccess, result );

	// Create a client
	t_Blastpit* client = blastpitNew();

	// Try connecting to non-existent server
	// result = connectToServer(client, "ws://localhost:8999", "simpleServerTest", 100);
	// TEST_ASSERT_EQUAL(false, result);

	// Connect to the server
	// We can't test for timeout because the server is not polling
	BPLOG( kLvlDebug, "%s: TIMEOUT EXPECTED SINCE EVENT LOOP IS NOT RUNNING\n", __func__ );
	connectToServer( client, "ws://127.0.0.1:8124", 0 );

	// Wait for handshake
	for ( int i = 0; i < 100; i++ ) {
		pollMessages( simpleserver );
		pollMessages( client );
		if ( bp_isConnected( client ) ) {
			break;
		}
	}
	TEST_ASSERT_EQUAL( true, bp_isConnected( client ) );

	// Send a message
	testval = 0;
	sendClientMessage( client, "test" );
	for ( int i = 0; i < 100; i++ ) {
		if ( testval ) {
			break;
		}
		pollMessages( client );
		pollMessages( simpleserver );
	}

	TEST_ASSERT_EQUAL( 1, testval );

	// Tidy up
	disconnectFromServer( client );
	serverDestroy( simpleserver );
	blastpitDelete( client );
	blastpitDelete( simpleserver );
}

void SendAndWaitTest( void )
{
	// Create a client
	t_Blastpit* client = blastpitNew();

	// Connect to the server
	connectToServer( client, WS_SERVER_TEST, 0 );
	for ( int i = 0; i < 100; i++ ) {
		pollMessages( server );
		pollMessages( client );
		if ( ( (t_Websocket*)client->ws )->isConnected ) {
			break;
		}
	}
	TEST_ASSERT_EQUAL( true, ( (t_Websocket*)client->ws )->isConnected );

	// Send a message
	sendClientMessage( client, "test" );

	disconnectFromServer( client );
	blastpitDelete( client );
}

void MessageTest( void )
{
	// What are the requirements to test 'x'?
	// 	What does the object do?
	// 	How does it interact with the data or hardware it controls?
	// 	How can we make it testable?

	// Allocates memory for t_Blastpit and t_Websocket
	t_Blastpit* simpleserver = blastpitNew();

	// Sets the t_Websocket address via serverCreate()
	int result = serverCreate( simpleserver, "ws://127.0.0.1:8129" );
	TEST_ASSERT_EQUAL( true, result );

	// Create two clients
	t_Blastpit* client1 = blastpitNew();
	t_Blastpit* client2 = blastpitNew();

	// Connect to the server
	// We can't check the result here because the server is not polling
	connectToServer( client1, "ws://127.0.0.1:8129", 0 );
	connectToServer( client2, "ws://127.0.0.1:8129", 0 );

	// Wait for handshake
	for ( int i = 0; i < 100; i++ ) {
		pollMessages( simpleserver );
		pollMessages( client1 );
		pollMessages( client2 );
		if ( ( (t_Websocket*)client1->ws )->isConnected && ( (t_Websocket*)client2->ws )->isConnected ) {
			break;
		}
	}

	TEST_ASSERT_EQUAL( true, ( (t_Websocket*)client1->ws )->isConnected );
	TEST_ASSERT_EQUAL( true, ( (t_Websocket*)client2->ws )->isConnected );

	// Send a message
	sendClientMessage( client1, "client1" );
	sendClientMessage( client2, "client2" );
	for ( int i = 0; i < 100; i++ ) {
		pollMessages( client1 );
		pollMessages( client2 );
		pollMessages( simpleserver );
		if ( getMessageCount( client1 ) == 1 && getMessageCount( client2 ) == 1 ) {
			break;
		}
	}

	TEST_ASSERT_EQUAL( 1, getMessageCount( client1 ) );
	TEST_ASSERT_EQUAL( 1, getMessageCount( client2 ) );

	const char* msg1 = popMessage( client1 );
	const char* msg2 = popMessage( client2 );
	TEST_ASSERT_EQUAL_STRING( "client2", msg1 );
	TEST_ASSERT_EQUAL_STRING( "client1", msg2 );
	free( (void*)msg1 );
	free( (void*)msg2 );

	// Tidy up
	disconnectFromServer( client1 );
	disconnectFromServer( client2 );
	serverDestroy( simpleserver );
	blastpitDelete( client1 );
	blastpitDelete( client2 );
	blastpitDelete( simpleserver );
}

void SignalTest( void )
{
	// What are the requirements to test 'x'?
	// 	What does the object do?
	// 	How does it interact with the data or hardware it controls?
	// 	How can we make it testable?

	// Allocates memory for t_Blastpit and t_Websocket
	t_Blastpit* simpleserver = blastpitNew();

	// Sets the t_Websocket address via serverCreate()
	int result = serverCreate( simpleserver, "ws://127.0.0.1:8129" );
	TEST_ASSERT_EQUAL( true, result );

	// Create two clients
	t_Blastpit* client1 = blastpitNew();

	// Connect to the server
	// We can't check the result here because the server is not polling
	connectToServer( client1, "ws://127.0.0.1:8129", 0 );

	// Wait for handshake
	for ( int i = 0; i < 100; i++ ) {
		pollMessages( simpleserver );
		pollMessages( client1 );
		if ( ( (t_Websocket*)client1->ws )->isConnected ) {
			break;
		}
	}

	TEST_ASSERT_EQUAL( true, ( (t_Websocket*)client1->ws )->isConnected );

	// Send a message
	// IdAck reply = QueueSignal( client1, 0x99, NULL );

	// for ( int i = 0; i < 100; i++ ) {
	// 	pollMessages( client1 );
	// 	pollMessages( simpleserver );
	// 	if ( getMessageCount( client1 ) == 1 )
	// 		break;
	// }

	// TEST_ASSERT_EQUAL( 1, getMessageCount( client1 ) );

	// const char* msg1 = popMessage( client1 );
	// TEST_ASSERT_EQUAL_STRING( "client2", msg1 );
	// free( (void*)msg1 );

	// Tidy up
	disconnectFromServer( client1 );
	serverDestroy( simpleserver );
	blastpitDelete( client1 );
	blastpitDelete( simpleserver );
}

void AutoGenId( void )
{
	// The first auto-generated id should be 1
	t_Blastpit* bp = blastpitNew();
	TEST_ASSERT_EQUAL( 1, AutoGenerateId( bp ) );

	// The generated ids should be one greater than the previous
	bp->highest_id = 123;
	TEST_ASSERT_EQUAL( 124, AutoGenerateId( bp ) );
	TEST_ASSERT_EQUAL( 125, AutoGenerateId( bp ) );

	blastpitDelete( bp );
}

void SendCommand( void )
{
	t_Blastpit* bp1 = blastpitNew();
	t_Blastpit* bp2 = blastpitNew();

	// usleep(100000);

	// The event loop is not running here, so we can't check the return result
	connectToServer( bp1, WS_SERVER_TEST, 0 );
	// usleep(100000);
	connectToServer( bp2, WS_SERVER_TEST, 0 );
	// usleep(100000);
	BPLOG( kLvlDebug, "%s: TIMEOUT EXPECTED SINCE EVENT LOOP IS NOT RUNNING\n", __func__ );

	// Wait for handshake
	for ( int i = 0; i < 10; i++ ) {
		pollMessages( server );
		usleep( 100000 );
		pollMessages( bp1 );
		// usleep(100000);
		pollMessages( bp2 );
		// usleep(100000);
		if ( bp_isConnected( bp1 ) && bp_isConnected( bp2 ) ) {
			break;
		}
	}
	TEST_ASSERT_EQUAL( true, bp_isConnected( bp1 ) );
	TEST_ASSERT_EQUAL( true, bp_isConnected( bp2 ) );
	TEST_ASSERT_EQUAL( 0, getMessageCount( server ) );
	TEST_ASSERT_EQUAL( 0, getMessageCount( bp1 ) );
	TEST_ASSERT_EQUAL( 0, getMessageCount( bp2 ) );

	BPLOG( kLvlDebug, "SendCommand: bp1 = %p\n", (void*)( (t_Websocket*)bp1->ws )->connection );
	BPLOG( kLvlDebug, "SendCommand: bp2 = %p\n", (void*)( (t_Websocket*)bp2->ws )->connection );

	IdAck result = BpQueueCommand( bp1, kSelfTest );
	TEST_ASSERT_EQUAL( kSuccess, result.retval );
	result = BpUploadQueuedMessages( bp1 );
	usleep( 100000 );
	TEST_ASSERT_EQUAL( kSuccess, result.retval );
	TEST_ASSERT_EQUAL( 1, result.id );

	for ( int i = 0; i < 10; i++ ) {
		pollMessages( bp1 );
		// usleep(100000);
		pollMessages( bp2 );
		// usleep(100000);
		pollMessages( server );
		// usleep(100000);
		if ( getMessageCount( bp2 ) == 1 ) {
			break;
		}
	}

	TEST_ASSERT_EQUAL( 1, getMessageCount( bp2 ) );
	const char* msg1 = popMessage( bp2 );
	sds	    selftest =
		sdscatprintf( sdsempty(),
			      "<?xml version=\"1.0\"?><message id=\"1\" type=\"command\" command=\"%d\" ></message>",
			      kSelfTest );
	TEST_ASSERT_EQUAL_STRING( selftest, msg1 );
	sdsfree( selftest );
	free( (void*)msg1 );
	// TEST_ASSERT_EQUAL_STRING("<?xml version=\"1.0\"?><message id=\"1\" type=\"command\" command=\"35\"
	// ></message>", msg1);

	TEST_ASSERT_EQUAL( 0, getMessageCount( bp1 ) );
	TEST_ASSERT_EQUAL( 0, getMessageCount( bp2 ) );
	disconnectFromServer( bp1 );
	disconnectFromServer( bp2 );
	blastpitDelete( bp1 );
	blastpitDelete( bp2 );
}

void QueueQpsetTest( void )
{
	t_Blastpit* bp = blastpitNew();

	// Should reject invalid current, speed, and frequency
	TEST_ASSERT_EQUAL(
		kInvalid,
		BpQueueQpSet( bp, "Low current", LMOS_CURRENT_MIN - 1, LMOS_SPEED_MIN, LMOS_FREQUENCY_MIN ).retval );
	TEST_ASSERT_EQUAL(
		kInvalid,
		BpQueueQpSet( bp, "High current", LMOS_CURRENT_MAX + 1, LMOS_SPEED_MIN, LMOS_FREQUENCY_MIN ).retval );
	TEST_ASSERT_EQUAL(
		kInvalid,
		BpQueueQpSet( bp, "Low speed", LMOS_CURRENT_MIN, LMOS_SPEED_MIN - 1, LMOS_FREQUENCY_MIN ).retval );
	TEST_ASSERT_EQUAL(
		kInvalid,
		BpQueueQpSet( bp, "High speed", LMOS_CURRENT_MIN, LMOS_SPEED_MAX + 1, LMOS_FREQUENCY_MIN ).retval );
	TEST_ASSERT_EQUAL(
		kInvalid,
		BpQueueQpSet( bp, "Low frequency", LMOS_CURRENT_MIN, LMOS_SPEED_MIN, LMOS_FREQUENCY_MIN - 1 ).retval );
	TEST_ASSERT_EQUAL(
		kInvalid,
		BpQueueQpSet( bp, "High frequency", LMOS_CURRENT_MIN, LMOS_SPEED_MIN, LMOS_FREQUENCY_MAX + 1 ).retval );

	// Should generate correct xml
	TEST_ASSERT_EQUAL( 1, BpQueueQpSet( bp, "Qpset1", 50, 50, 20000 ).id );
	TEST_ASSERT_EQUAL( 2, BpQueueQpSet( bp, "Qpset2", 60, 60, 30000 ).id );
	TEST_ASSERT_EQUAL_STRING(
		"<?xml version=\"1.0\"?><message id=\"1\" type=\"command\" command=\"1\" name=\"Qpset1\" "
		"current=\"50\" speed=\"50\" frequency=\"20000\" ></message><message id=\"2\" depends=\"1\" "
		"type=\"command\" "
		"command=\"1\" name=\"Qpset2\" current=\"60\" speed=\"60\" frequency=\"30000\" ></message>",
		bp->message_queue );

	blastpitDelete( bp );
}

void DependentChildTest( void )
{  // Checks that queued messages rely on parent

	// Queued messages should have 'depends' attribute added
	// containing the previous messages in the queue, if any

	// 'depends' attribute should be comma separated and sorted numerically.

	// HasNoDependencies() should return true if all of the
	// message dependencies are recorded as successful.
	// Should return false if any dependency failed.

	t_Blastpit* bp = blastpitNew();

	IdAck message1_id = BpQueueMessage( bp, "payload", NULL );
	// This should have an automatic dependency on message1
	IdAck message2_id = BpQueueMessage( bp, "loadpay", NULL );
	// This should depend on both
	IdAck message3_id = BpQueueMessage( bp, "loadpayload", NULL );

	(void)message1_id;
	(void)message2_id;
	(void)message3_id;

	sds message3_data = XmlGetMessageByIndex( bp->message_queue, 2 );
	sds attr	  = XmlGetAttribute( message3_data, "depends" );
	TEST_ASSERT_EQUAL_STRING( "2", attr );

	sdsfree( attr );
	sdsfree( message3_data );
	blastpitDelete( bp );
}

void RetvalDbTest( void )
{
	// What are the requirements to test 'x'?
	// 	What does the object do?
	// 	How does it interact with the data or hardware it controls?
	// 	How can we make it testable?

	// Must return null if no record of id
	// Must return stored id if present

	t_Blastpit* bp = blastpitNew();
	// RetvalDb db3 = {0, kInvalid, NULL};
	// RetvalDb db2 = {0, kInvalid, &db3};
	// RetvalDb db1 = {0, kInvalid, &db2};
	// bp->retval_db = &db1;

	BpAddRetvalToDb( bp, (IdAck){ 4, kBadLogic, NULL } );
	BpAddRetvalToDb( bp, (IdAck){ 5, kBadXml, NULL } );
	// We need to rig the check against the highest id
	bp->highest_id = 5;

	// Should return kNotFound
	TEST_ASSERT_EQUAL( kNotFound, BpQueryRetvalDb( bp, 1 ) );

	TEST_ASSERT_EQUAL( kBadLogic, BpQueryRetvalDb( bp, 4 ) );
	TEST_ASSERT_EQUAL( kBadXml, BpQueryRetvalDb( bp, 5 ) );

	BpFreeRetvalDb( bp );
	blastpitDelete( bp );
}

void ConnectivityTest( void )
{
	// What are the requirements to test 'x'?
	// 	What does the object do?
	// 	How does it interact with the data or hardware it controls?
	// 	How can we make it testable?

	t_Blastpit* client = blastpitNew();

	// Connect to the server
	connectToServer( client, WS_SERVER_TEST, 0 );
	for ( int i = 0; i < 100; i++ ) {
		pollMessages( server );
		pollMessages( client );
		if ( ( (t_Websocket*)client->ws )->isConnected ) {
			break;
		}
		usleep( 1000 );
	}

	TEST_ASSERT_EQUAL( true, ( (t_Websocket*)client->ws )->isConnected );
	TEST_ASSERT_EQUAL( false, BpIsLmosUp( client ) );

	disconnectFromServer( client );
	blastpitDelete( client );
}

static void unityShortTests( void )
{  // These tests must pass or fail within microseconds. No network tests.

	RUN_TEST( RetvalDbTest );
	RUN_TEST( AutoGenId );
	RUN_TEST( QueueQpsetTest );
	RUN_TEST( DependentChildTest );
}

static void unityLongTests( void )
{  // Long-running tests, meant to be run under Buildbot.

	testSetup();
	RUN_TEST( ConnectivityTest );
	testTeardown();

	testSetup();
	RUN_TEST( SendAndWaitTest );
	testTeardown();

	testSetup();
	RUN_TEST( SendCommand );
	testTeardown();

	testSetup();
	RUN_TEST( simpleServerTest );
	testTeardown();

	testSetup();
	RUN_TEST( MessageTest );
	testTeardown();

	testSetup();
	RUN_TEST( SignalTest );
	testTeardown();
}

int StdinTest( int argc, const char* argv[] )
{  // Run tests using stdin as input, mostly used with afl fuzzing

	(void)argc;
	(void)argv;

	return 0;
}


int main( int argc, const char* argv[] )
{
	while ( 1 ) {
		static struct option long_options[] = { { "stdin", no_argument, 0, 't' },
							{ "longtests", no_argument, 0, 'l' },
							{ 0, 0, 0, 0 } };

		int c, option_index = 0;
		c = getopt_long( argc, (char* const*)argv, "ltv", long_options, &option_index );
		if ( c == -1 ) {
			break;
		}

		switch ( c ) {
			case 't':
				// stdin test is used with afl
				fprintf( stderr, "Running stdin test...\n" );
				return StdinTest( argc, argv );

			case 'l':
				// To speed the development cycle, long tests
				// must be forced (usually by buildbot)
				fprintf( stderr, "Running long tests...\n" );
				UNITY_BEGIN();
				unityShortTests();
				unityLongTests();
				return UNITY_END();

			default:
				UNITY_BEGIN();
				unityShortTests();
				return UNITY_END();
		}
	}

	return 0;
}
