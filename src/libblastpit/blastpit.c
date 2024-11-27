#include <assert.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>

#include "blastpit.h"
#include "sds.h"
#include "websocket.h"
#include "xml.h"

t_Blastpit* blastpitNew( void )
{  // Constructor

	t_Blastpit* bp = (t_Blastpit*)calloc( 1, sizeof( t_Blastpit ) );
	if ( bp ) {
		bp->ws		   = (void*)websocketNew();
		bp->highest_id	   = 0;
		bp->message_queue  = NULL;
		bp->retval_db	   = NULL;
		bp->light_is_on	   = 1;	 // Assume default state of laser at startup
		bp->door_is_closed = 1;
	}

	return bp;
}

void blastpitDelete( t_Blastpit* bp )
{  // Destructor

	if ( bp->ws ) {
		websocketDelete( (t_Websocket*)bp->ws );
	}
	if ( bp->message_queue ) {
		sdsfree( bp->message_queue );
	}
	if ( bp->retval_db ) {
		BpFreeRetvalDb( bp );
	}
	free( bp );
}

const char* bpCommandName( int command )
{
	if ( command <= BPCOMMAND_MAX ) {
		return bpCommandString[command];
	}
	return BP_EMPTY_STRING;
}

const char* bpRetvalName( int retval )
{
	if ( retval <= RETVAL_MAX ) {
		return bpRetvalString[retval];
	}
	return BP_EMPTY_STRING;
}

int serverCreate( t_Blastpit* self, const char* port )
{  // Start a listening server

	if ( port == NULL ) {
		return kInvalid;
	}
	if ( ( (t_Websocket*)self->ws )->isServer ) {
		return kAlreadyInUse;  // Already in use
	}

	int result = wsServerCreate( (t_Websocket*)self->ws, port );
	return result;
}

void serverDestroy( t_Blastpit* self )
{  // Close the WebSocket server and free any resources

	wsServerDestroy( (t_Websocket*)self->ws );
}

int connectToServer( t_Blastpit* self, const char* server, int timeout_ms )
{  // Connects a client to the server

	// TODO(thf): timeout_ms may not be needed with WebSockets

	assert( self->ws );

	BPLOG( kLvlDebug, "connectToServer: timout_ms = %d\n", timeout_ms );

	// Prevent servers connecting to servers
	if ( ( (t_Websocket*)self->ws )->isServer ) {
		return kBadLogic;
	}

	wsClientCreate( (t_Websocket*)self->ws, server );

	// Wait until connected or timeout
	// Note: This relies on the server running and polling itself
	while ( !bp_isConnected( self ) && timeout_ms > 0 ) {
		pollMessages( self );
		// TODO(thf): Convert these magic numbers to DEFINES
		usleep( 100000 );
		timeout_ms -= 100;
		if ( timeout_ms <= 0 ) {
			BPLOG( kLvlDebug, "%s: Timed out connecting to server\n", __func__ );
			return kConnectionFailure;
		}
	}

	return kSuccess;
}

int waitForConnection( t_Blastpit* self, int timeout )
{  // Repeatedly poll until we connect to server or timeout

	while ( timeout > 0 ) {
		if ( bp_isConnected( self ) ) {
			fprintf( stderr, "Now connected\n" );
			return 17;
		}
		pollMessages( self );
		usleep( 500 );
		timeout -= 500;
	}

	return false;
}

void disconnectFromServer( t_Blastpit* self )
{
	wsClientDestroy( (t_Websocket*)self->ws );
}

void pollMessages( t_Blastpit* self )
{  // Check for and process network activity

	wsPoll( (t_Websocket*)self->ws );
}

void pollMessagesWithTimeout( t_Blastpit* self, int timeout )
{  // Check for and process network activity

	wsPollWithTimeout( (t_Websocket*)self->ws, timeout );
}

void sendClientMessage( t_Blastpit* self, const char* message )
{
	BPLOG( kLvlEverything, "sendClientMessage: %s\n", message );
	wsClientSendMessage( (t_Websocket*)self->ws, (char*)message );
}

void sendServerMessage( t_Blastpit* self, const char* message )
{
	wsServerSendMessage( (t_Websocket*)self->ws, (char*)message );
}

void registerCallback( t_Blastpit* self, void ( *callback )( void*, void* ) )
{  // Specify an additional callback upon message receipt

	wsSetMessageReceivedCallback( (t_Websocket*)self->ws, callback );
}

WsMessage ConvertCallbackData( void* ev_data )
{  // Gets the size and data from a websocket_message

	return ExtractWsMessageData( ev_data );

	// (void)ev_data;
	// WsMessage a;
	// a.size = 0;
	// return a;
}

void registerObject( t_Blastpit* self, void* object )
{  // Specify an instance to call the callback method

	wsSetMessageReceivedObject( (t_Websocket*)self->ws, object );
}

void UpdateHighestId( t_Blastpit* self, int id )
{  // Updates the highest known id

	if ( id > self->highest_id ) {
		self->highest_id = id;
	}
}

IdAck bp_sendMessage( t_Blastpit* self, const char* message )
{
	assert( self );
	assert( message );

	// BPLOG(kLvlDebug, "%s: start\n", __func__);
	int id	       = 0;
	sds id_message = NULL;

	if ( XmlGetMessageCount( message ) < 1 ) {
		return (IdAck){ kInvalid, kBadXml, NULL };
	}

	if ( XmlGetMessageCount( message ) == 1 ) {
		// Does this message already have an id?
		sds existing_id = XmlGetAttribute( message, "id" );
		if ( existing_id ) {
			id = atoi( existing_id );
			sdsfree( existing_id );
			id_message = sdsnew( message );
		} else {
			id = AutoGenerateId( self );
			UpdateHighestId( self, id );
			sds id_str = sdsfromlonglong( id );

			// We duplicate the message since XmlSetAttribute is destructive
			sds orig_message = sdsnew( message );
			id_message	 = XmlSetAttribute( (char*)orig_message, "id", id_str );
			XmlAddXmlHeader( &id_message );
			sdsfree( id_str );
		}

		if ( !id_message ) {
			return (IdAck){ kInvalid, kSetterFailure, NULL };
		}
	} else {
		id	   = kMultipleCommands;
		id_message = sdsnew( message );
	}

	if ( ( (t_Websocket*)self->ws )->isServer ) {
		sendServerMessage( self, id_message );
	} else {
		if ( !bp_isConnected( self ) ) {
			sdsfree( id_message );
			return (IdAck){ id, kConnectionFailure, NULL };
			// return result;
		}
		sendClientMessage( self, id_message );
	}

	// BPLOG(kLvlDebug, "bp_sendMessage: %s\n", id_message);
	sdsfree( id_message );

	// BPLOG(kLvlDebug, "%s: end\n", __func__);
	return (IdAck){ id, kSuccess, NULL };
}

IdAck bp_sendMessageAndWait( t_Blastpit* self, const char* message, int timeout )
{ /* Sends messageL and waits. Returns xml of reply with matching id */
	// Caller must free the result.string, if any

	IdAck result;

	result = bp_sendMessage( self, message );
	if ( result.retval != kSuccess ) {
		return result;
	}

	result = BpWaitForReplyOrTimeout( self, result.id, timeout );

	return result;
}

// IdAck
// bp_sendMessageAndWaitForString(t_Blastpit* self, const char* message, int timeout)
// { /* Sends message and waits. Returns xml of reply with matching id */

// 	IdAck result = bp_sendMessage(self, message);
// 	if (result.retval != kSuccess) {
// 		return result;
// 	}

// 	return bp_waitForString(self, result.id, timeout);
// }

IdAck BpWaitForReplyOrTimeout( t_Blastpit* self, int id, int timeout )
{  // Continually polls the network until reply received or timeout
	// If the IdAck contains a string the caller must free it after use

	// TODO(thf): This doesn't work if the replies are queued, since it only
	// looks at the first message. We need a function to break multiple replies
	// up into individals for processing

	// TODO(thf): Clean up the logic in this

	struct timeval start_time;
	struct timeval current_time;
	if ( gettimeofday( &start_time, NULL ) == -1 ) {
		BPLOG( kLvlError, "%s: Cannot access system time\n", __func__ );
		return (IdAck){ id, kFailure, NULL };
	}

	for ( int i = 0; i < timeout; i++ ) {
		gettimeofday( &current_time, NULL );
		if ( current_time.tv_usec - start_time.tv_usec > timeout * 1000 ) {
			break;
		}

		pollMessages( self );
		if ( getMessageCount( self ) == 0 ) {
			continue;
		}

		/* Test every message with getMessageAt() */
		for ( int j = 0; j < getMessageCount( self ); j++ ) {
			char* message = readMessageAt( self, j );

			if ( message ) {
				sds parentid_str = XmlGetAttribute( message, "parentid" );
				sds retval_str	 = XmlGetAttribute( message, "retval" );
				sds id_str	 = sdsfromlonglong( id );
				assert( id_str );
				int do_strings_match = -1;
				if ( parentid_str && id_str ) {
					do_strings_match = strcmp( parentid_str, id_str );
				}
				int retval = 0;
				if ( retval_str ) {
					retval = atoi( retval_str );
				} else {
					retval = kInvalid;
				}
				sdsfree( parentid_str );
				sdsfree( id_str );
				sdsfree( retval_str );
				if ( do_strings_match == 0 ) {
					void* msg_data = popMessageAt( self, j );
					// free(msg_data);
					if ( msg_data ) {
						BPLOG( kLvlDebug,
						       "BpWaitForReplyOrTimeout: Message had payload %p\n",
						       msg_data );
					}
					return (IdAck){ id, retval, (char*)msg_data };
				}
			}
		}
	}


	BPLOG( kLvlDebug, "%s: Reply timeout\n", __func__ );
	return (IdAck){ id, kReplyTimeout, NULL };
}

IdAck BpWaitForSignalOrTimeout( t_Blastpit* self, int sigtype, int timeout )
{  // Continually polls the network until reply received or timeout
	// If the IdAck contains a string the caller must free it after use

	// TODO(thf): This doesn't work if the replies are queued, since it only
	// looks at the first message. We need a function to break multiple replies
	// up into individals for processing

	// TODO(thf): Clean up the logic in this

	struct timeval start_time;
	struct timeval current_time;
	if ( gettimeofday( &start_time, NULL ) == -1 ) {
		BPLOG( kLvlError, "%s: Cannot access system time\n", __func__ );
		return (IdAck){ 0, kFailure, NULL };
	}

	for ( int i = 0; i < timeout; i++ ) {
		gettimeofday( &current_time, NULL );
		if ( current_time.tv_usec - start_time.tv_usec > timeout * 1000 ) {
			break;
		}

		pollMessages( self );
		if ( getMessageCount( self ) == 0 ) {
			continue;
		}

		for ( int j = 0; j < getMessageCount( self ); j++ ) {
			char* message = readMessageAt( self, j );

			if ( message ) {
				// Is this a signal?
				sds type_str  = XmlGetAttribute( message, "type" );
				int is_signal = strcmp( "signal", type_str );
				if ( is_signal != 0 ) {
					sdsfree( type_str );
					continue;
				}

				// What type of signal is it?
				sds signal_str	= XmlGetAttribute( message, "signal" );
				int msg_sigtype = atoi( signal_str );
				sdsfree( signal_str );

				if ( msg_sigtype == sigtype || sigtype == -1 ) {
					// -1 means any signal
					void* sig_data = popMessageAt( self, j );
					if ( sig_data ) {
						BPLOG( kLvlDebug,
						       "BpWaitForSignalOrTimeout: Signal had payload %p\n",
						       sig_data );
					}
					// We return the signal type in the id field
					BPLOG( kLvlDebug,
					       "BpWaitForSignalOrTimeout: msg_sigtype is %d\n",
					       msg_sigtype );
					return (IdAck){ msg_sigtype, kSuccess, (char*)sig_data };
				}
			}
		}
	}


	BPLOG( kLvlDebug, "%s: Reply timeout\n", __func__ );
	return (IdAck){ 0, kReplyTimeout, NULL };
}

char* BpSdsToString( sds string )
{  // Helper function to copy an sds string to a standard char string

	int   len     = sdslen( string );
	char* cstring = (char*)malloc( len + 1 );
	assert( cstring );
	strncpy( cstring, string, len );
	return cstring;
}

// IdAck
// bp_waitForString(t_Blastpit* self, int id, int timeout)
// {  // Waits for CDATA reply, with timeout
// 	// Note: The returned IdAck string must be freed by the caller

// 	const char* message = BpWaitForReplyOrTimeout(self, id, timeout, true).string;
// 	sds string = XmlGetString(message);

// 	IdAck result;
// 	if (string) {
// 		char* cdata = BpSdsToString(string);
// 		sdsfree(string);
// 		result = (IdAck){id, kSuccess, cdata};
// 	} else {
// 		result = (IdAck){id, kFailure, NULL};
// 	};

// 	return result;
// }

// IdAck
// sendCommand(t_Blastpit* self, int command)
// {
// 	assert(self);

// 	char message[] = "<command>xxx</command>";

// 	snprintf(message, sizeof(message) / sizeof(message[0]), "<command>%d</command>", command);
// 	return bp_sendMessage(self, message);
// }

// IdAck
// SendAckRetval(t_Blastpit* self, int id, int retval)
// {  // Sends a message acknowledgement with return value

// 	sds id_str = sdsfromlonglong(id);
// 	sds retval_str = sdsfromlonglong(retval);
// 	IdAck result = SendMessageBp(self, "type", "reply", "parentid", id_str, "retval", retval_str, NULL);
// 	sdsfree(retval_str);
// 	sdsfree(id_str);

// 	return result;
// }

IdAck QueueAckRetval( t_Blastpit* self, int id, int retval )
{  // Sends a message acknowledgement with return value

	sds   id_str	 = sdsfromlonglong( id );
	sds   retval_str = sdsfromlonglong( retval );
	IdAck result	 = BpQueueMessage( self, "type", "reply", "parentid", id_str, "retval", retval_str, NULL );
	sdsfree( retval_str );
	sdsfree( id_str );

	return result;
}

IdAck QueueReplyPayload( t_Blastpit* self, int id, int retval, const char* payload )
{  // Sends a reply payload

	sds   id_str	 = sdsfromlonglong( id );
	sds   retval_str = sdsfromlonglong( retval );
	IdAck result = BpQueueMessage( self, "type", "reply", "parentid", id_str, "retval", retval_str, payload, NULL );
	sdsfree( retval_str );
	sdsfree( id_str );

	return result;
}

IdAck BpQueueSignal( t_Blastpit* self, int signal, const char* payload )
{  // Sends an lmos signal

	sds   signal_str = sdsfromlonglong( signal );
	IdAck result	 = BpQueueMessage( self, "type", "signal", "signal", signal_str, payload, NULL );
	sdsfree( signal_str );

	return result;
}

// IdAck
// SendCommand(t_Blastpit* self, int command)
// {
// 	sds cmd = sdsfromlonglong(command);
// 	IdAck result = SendMessageBp(self, "type", "command", "command", cmd, NULL);
// 	sdsfree(cmd);
// 	return result;
// }

// IdAck
// SendMessageBp(t_Blastpit* self, ...)
// {  // Sends single command with optional attributes
// 	// If there are an odd number of optional parameters,
// 	// the final parameter is used as the message CDATA

// 	va_list args;
// 	va_start(args, self);
// 	char *attrib, *value;
// 	sds xml = sdsnew("<?xml version=\"1.0\"?><message ");
// 	bool first_attribute = true;

// 	while (true) {
// 		// Get the attribute name
// 		attrib = va_arg(args, char*);  // Pop the next argument (a char*)
// 		if (!attrib)
// 			break;

// 		// Get the attribute value
// 		value = va_arg(args, char*);  // Pop the next argument (a char*)
// 		if (!value)
// 			break;

// 		// Append the attribute to the XML
// 		if (first_attribute) {
// 			xml = sdscatprintf(xml, "%s=\"%s\"", attrib, value);
// 			first_attribute = false;
// 		} else {
// 			xml = sdscatprintf(xml, " %s=\"%s\"", attrib, value);
// 		}
// 	}

// 	xml = sdscatprintf(xml, ">");

// 	if (attrib) {  // Payload
// 		xml = sdscatprintf(xml, "%s", attrib);
// 	}

// 	xml = sdscatprintf(xml, "</message>");

// 	IdAck result = bp_sendMessage(self, xml);

// 	va_end(args);
// 	sdsfree(xml);

// 	return result;
// }

int AutoGenerateId( t_Blastpit* self )
{  // Generates an id higher than any previous

	self->highest_id++;
	return self->highest_id;
}

// IdAck
// bp_sendCommandAndWait(t_Blastpit* self, int command, int timeout)
// {
// 	IdAck result;

// 	result = SendCommand(self, command);
// 	BPLOG(kLvlDebug, "bp_sendCommandAndWait: id = %d\n", result.id);
// 	BPLOG(kLvlDebug, "bp_sendCommandAndWait: retval = %d\n", result.retval);
// 	if (result.retval != kSuccess) {
// 		BPLOG(kLvlDebug, "%s: Failed to send command", __func__);
// 		result = (IdAck){kInvalid, kCommandFailed, NULL};
// 		return result;
// 	}
// 	result = BpWaitForReplyOrTimeout(self, result.id, timeout);
// 	if (result.retval != kSuccess) {
// 		BPLOG(kLvlDebug, "%s: Timed out waiting for XML", __func__);
// 		result = (IdAck){kInvalid, kReplyTimeout, NULL};
// 		return result;
// 	}

// 	return result;
// }

bool bp_isConnected( t_Blastpit* self )
{
	return ( (t_Websocket*)self->ws )->isConnected;
}

char* popMessage( t_Blastpit* self )
{  // Pop a message from the stack
	// The message must be freed by the user

	return popMessageAt( self, 0 );
}

char* popMessageAt( t_Blastpit* self, int index )
{  // Pop a message at a specific index

	return (char*)wsPopMessageAt( (t_Websocket*)self->ws, index );
}

int getMessageCount( t_Blastpit* self )
{  // Get the count of the message stack

	return wsGetMessageCount( (t_Websocket*)self->ws );
}

char* readMessageAt( t_Blastpit* self, int index )
{  // Get the message for a specific index without deleting

	return (char*)wsReadMessageAt( (t_Websocket*)self->ws, index );
}

void startLMOS( t_Blastpit* self )
{  // Send a message to LMOS to unload the ActiveX control

	BpQueueCommand( self, kCreateLMOS );
}

void stopLMOS( t_Blastpit* self )
{  // Send a message to LMOS to unload the ActiveX control

	BpQueueCommand( self, kDestroyLMOS );
}

void clearQPSets( t_Blastpit* self )
{  // Tell Lmos to erase all existing QPsets

	BpQueueCommand( self, kClearQpSets );
}

void LayerSetLaserable( t_Blastpit* self, const char* layer, bool laserable )
{
	sds message = sdscatprintf( sdsempty(),
				    "<command layer=\"%s\" laserable=\"%d\">%d</command>",
				    layer,
				    (int)laserable,
				    kLayerSetLaserable );
	bp_sendMessage( self, message );
	sdsfree( message );
}

void LayerSetHeight( t_Blastpit* self, const char* layer, int height )
{
	sds message = sdscatprintf(
		sdsempty(), "<command layer=\"%s\" height=\"%d\">%d</command>", layer, height, kLayerSetHeight );
	bp_sendMessage( self, message );
	sdsfree( message );
}

char* SdsEmpty( void )
{  // Helper function to create sds string that can be called from c++
	// Avoids c++ code needing to know anything about sds

	return (char*)sdsempty();
}

char* SdsNew( const char* string )
{  // Helper function to create sds string that can be called from c++
	// Avoids c++ code needing to know anything about sds

	return (char*)sdsnew( string );
}

char* SdsFromLong( long number )
{  // Helper function to create sds string from an int

	return (char*)sdsfromlonglong( number );
}

void SdsFree( char* string )
{  // Helper function to free an sds string disguised as a char string

	sdsfree( (sds)string );
}

int BpGetMessageCount( const char* xml )
{  // C++ wrapper for XmlGetMessageCount

	return XmlGetMessageCount( xml );
}

char* BpGetMessageByIndex( const char* xml, int index )
{  // Wrapper for GetMessageByIndex

	return (char*)XmlGetMessageByIndex( xml, index );
}

char* BpGetMessageAttribute( const char* message, const char* attribute )
{  // Wrapper for GetMessageAttribute

	return (char*)XmlGetAttribute( message, attribute );
}

char* BpGetChildNodeAsString( const char* message, const char* child_name )
{  // Wrapper for XmlGetString

	return (char*)XmlGetChildNodeAsString( message, child_name );
}

IdAck BpQueueCommand( t_Blastpit* self, int command )
{
	if ( !self ) {
		return (IdAck){ kInvalid, kFailure, NULL };
	}

	sds command_str = sdscatprintf( sdsempty(), "%d", command );

	IdAck result = BpQueueMessage( self, "type", "command", "command", command_str, NULL );

	sdsfree( command_str );

	return result;
}

IdAck BpQueueMessage( t_Blastpit* self, ... )
{  // REMEMBER THE NULL
	// Append message to global queue for bulk upload
	// If there are an odd number of optional parameters,
	// the final parameter is used as the message CDATA

	va_list args;
	va_start( args, self );
	char* attrib  = NULL;
	char* value   = NULL;
	int   id      = AutoGenerateId( self );
	sds   message = NULL;

	if ( self->message_queue ) {  // Find parent dependencies and add them
		sds parent = XmlGetMessageByIndex( self->message_queue, BpGetMessageCount( self->message_queue ) - 1 );
		sds parent_id = BpGetMessageAttribute( parent, "id" );
		message	      = sdscatprintf( sdsempty(), "<message id=\"%d\" depends=\"%s\" ", id, parent_id );
		sdsfree( parent_id );
		sdsfree( parent );
	} else {
		message = sdscatprintf( sdsempty(), "<message id=\"%d\" ", id );
	}

	sds xml = NULL;

	// BPLOG(kLvlDebug, "message: %s\n", message);
	while ( true ) {
		// Get the attribute name
		attrib = va_arg( args, char* );	 // Pop the next argument (a char*)
		if ( !attrib ) {
			break;
		}

		// Get the attribute value
		value = va_arg( args, char* );	// Pop the next argument (a char*)
		if ( !value ) {
			break;
		}
		// assert(value > 0xffff);
		if ( *value == 0 ) {
			break;
		}

		// Append the attribute to the message
		message = sdscatprintf( message, "%s=\"%s\" ", attrib, value );
	}
	BPLOG( kLvlDebug, "found attribute %s\n", attrib );
	BPLOG( kLvlDebug, "found value %s\n", value );

	message = sdscatprintf( message, ">" );
	BPLOG( kLvlDebug, "message: %s\n", message );

	if ( attrib ) {	 // Payload
		BPLOG( kLvlDebug, "payload: %s\n", attrib );
		message = sdscatprintf( message, "%s", attrib );
	}

	message = sdscatprintf( message, "</message>" );

	if ( self->message_queue ) {
		xml = self->message_queue;
	} else {
		xml = sdsnew( "<?xml version=\"1.0\"?>" );
	}

	xml = sdscat( xml, message );

	BPLOG( kLvlEverything, "(BpQueueMessage) New queue: %s\n", xml );

	va_end( args );
	sdsfree( message );

	self->message_queue = xml;

	return (IdAck){ id, kSuccess, NULL };
}

IdAck BpUploadQueuedMessages( t_Blastpit* self )
{  // Uploads the queued message and frees the sds string

	if ( !self->message_queue ) {
		return (IdAck){ kInvalid, kFailure, NULL };
	}

	BPLOG( kLvlEverything, "(BpUploadQueuedMessages) Queue: %s\n", self->message_queue );
	IdAck result = bp_sendMessage( self, self->message_queue );
	sdsfree( self->message_queue );
	self->message_queue = NULL;

	return result;
}

IdAck BpQueueQpSet( t_Blastpit* self, char* name, int current, int speed, int frequency )
{  // Queue a qp set for upload

	if ( !self ) {
		return (IdAck){ kInvalid, kInvalid, NULL };
	}
	if ( current < LMOS_CURRENT_MIN || current > LMOS_CURRENT_MAX ) {
		return (IdAck){ kInvalid, kInvalid, NULL };
	}
	if ( speed < LMOS_SPEED_MIN || speed > LMOS_SPEED_MAX ) {
		return (IdAck){ kInvalid, kInvalid, NULL };
	}
	if ( frequency < LMOS_FREQUENCY_MIN || frequency > LMOS_FREQUENCY_MAX ) {
		return (IdAck){ kInvalid, kInvalid, NULL };
	}

	sds command_str	  = sdscatprintf( sdsempty(), "%d", kAddQpSet );
	sds current_str	  = sdscatprintf( sdsempty(), "%d", current );
	sds speed_str	  = sdscatprintf( sdsempty(), "%d", speed );
	sds frequency_str = sdscatprintf( sdsempty(), "%d", frequency );

	IdAck result = BpQueueMessage( self,
				       "type",
				       "command",
				       "command",
				       command_str,
				       "name",
				       name,
				       "current",
				       current_str,
				       "speed",
				       speed_str,
				       "frequency",
				       frequency_str,
				       NULL );

	sdsfree( frequency_str );
	sdsfree( speed_str );
	sdsfree( current_str );
	sdsfree( command_str );

	return result;
}

IdAck BpQueueCommandArgs( t_Blastpit* self,
			  int	      command,
			  const char* attr1,
			  const char* val1,
			  const char* attr2,
			  const char* val2,
			  const char* attr3,
			  const char* val3,
			  const char* attr4,
			  const char* val4,
			  const char* payload )
{  // Queue a command with four attributes and payload
	// This is a fix for lack of swig support for variadic functions
	// Supply dummy command/value pairs for unneeded values or payload

	if ( !self ) {
		return (IdAck){ kInvalid, kFailure, NULL };
	}

	sds command_str = sdscatprintf( sdsempty(), "%d", command );

	IdAck result = BpQueueMessage( self,
				       "type",
				       "command",
				       "command",
				       command_str,
				       attr1,
				       val1,
				       attr2,
				       val2,
				       attr3,
				       val3,
				       attr4,
				       val4,
				       payload,
				       NULL );

	sdsfree( command_str );

	return result;
}

// void
// BpQueueDrawing(t_Blastpit *self, char *drawing)
// { // Rewrites xml

// 	sds dirty = sdsnew(drawing);
// 	sds tidy = XmlDrawingToMessage(dirty);
// 	sds command_str = sdscatprintf(sdsempty(), "%d", kImportXML);

// 	int id = AutoGenerateId(self);
// 	IdAck result = bp_sendMessage(self, self->message_queue);
// 	printf("\n\n%s\n\n", tidy);
// 	BpQueueMessage(self, "type", "command",
// 				    "command", command_str,
// 				    tidy,
// 				    NULL);

// 	sdsfree(command_str);
// 	sdsfree(tidy);
// 	sdsfree(dirty);
// }

void BpPrintQueue( t_Blastpit* self )
{
	printf( "\n\nMessage Queue\n-------------\n\n%s\n\n", self->message_queue );
}

int BpQueryRetvalDb( t_Blastpit* self, int id )
{  // Returns retval of completed commands or kInvalid if not found

	if ( !self ) {
		return kInvalid;
	}

	RetvalDb* item = self->retval_db;

	// BPLOG(kLvlDebug, "BpQueryRetvalDb: highest id = %d\n", self->highest_id);
	while ( item ) {
		BPLOG( kLvlDebug, "BpQueryRetvalDb: id = %d\n", item->id );
		if ( item->id == id ) {
			return item->retval;
		}
		if ( item->id > self->highest_id ) {
			return kBadLogic;  // Detect a corrupt list
		}
		item = item->next;
	}

	return kNotFound;
}

int BpAddRetvalToDb( t_Blastpit* self, IdAck record )
{
	if ( !self ) {
		return kInvalid;
	}

	RetvalDb* new_record = (RetvalDb*)malloc( sizeof( RetvalDb ) );
	new_record->id	     = record.id;
	new_record->retval   = record.retval;
	new_record->next     = self->retval_db;
	self->retval_db	     = new_record;

	return kSuccess;
}

void BpFreeRetvalDb( t_Blastpit* self )
{  // Free all memory used by the return value database

	if ( !self ) {
		return;
	}

	RetvalDb* record = self->retval_db;

	while ( record ) {
		// A simple sanity check
		// assert(record->id <= self->highest_id);
		RetvalDb* next = record->next;
		free( record );
		record = next;
	}

	self->retval_db = NULL;
}

int BpIsLmosUp( t_Blastpit* self )
{  // Fast dedicated check to see if lmos controller is running
   // Note that this will return false if lmos is busy.

	// We don't want any queued messages before the test
	char* existing_queue = self->message_queue;
	self->message_queue  = NULL;

	IdAck lmos = BpQueueCommand( self, kIsLmosRunning );
	BpUploadQueuedMessages( self );
	IdAck timeout = BpWaitForReplyOrTimeout( self, lmos.id, BP_ISLMOSUP_TIMEOUT );
	if ( timeout.string ) {
		BPLOG( kLvlDebug, "BpIsLmosUp: string = %s\n", timeout.string );
	}

	self->message_queue = existing_queue;

	return ( timeout.retval == kSuccess );
}

void BpSetLightState( t_Blastpit* self, bool state )
{  // Sets the light state

	// We have no way of requesting the actual light status
	if ( state == self->light_is_on ) {
		BPLOG( kLvlDebug, "(%s) Light is already in requested state\n", __func__ );
		return;
	}

	char* existing_queue = self->message_queue;
	self->message_queue  = NULL;

	sds command_str = sdsfromlonglong( kWriteIoBit );
	if ( self->light_is_on == 1 ) {
		BPLOG( kLvlDebug, "%s: Turning light off\n", __func__ );
		BpQueueMessage(
			self, "type", "command", "command", command_str, "bitfunction", "Light", "value", "0", NULL );
		self->light_is_on = 0;
	} else {
		BPLOG( kLvlDebug, "%s: Turning light on\n", __func__ );
		BpQueueMessage(
			self, "type", "command", "command", command_str, "bitfunction", "Light", "value", "1", NULL );
		self->light_is_on = 1;
	}
	BpUploadQueuedMessages( self );

	sdsfree( command_str );

	self->message_queue = existing_queue;
}

void BpFreePayload( IdAck ack )
{  // Python helper to free any message payload

	if ( ack.string ) {
		free( ack.string );
	}
}

void BpSetDoorState( t_Blastpit* self, bool state )
{  // Toggles the door state
	// Write to 'OpenDoor' or 'CloseDoor', hold, then clear

	char* existing_queue = self->message_queue;
	self->message_queue  = NULL;

	// Get the current door status
	// We can read either 'DoorOpen' or 'DoorClosed'. I assume that they are
	// the names of the sensors at the top and bottom of the door.
	sds   command_str = sdsfromlonglong( kReadIOBit );
	IdAck light =
		BpQueueMessage( self, "type", "command", "command", command_str, "bitfunction", "DoorOpen", NULL );
	BpUploadQueuedMessages( self );

	IdAck timeout = BpWaitForReplyOrTimeout( self, light.id, BP_SHORT_TIMEOUT );
	(void)timeout;

	// if (timeout.retval != kSuccess || !timeout.string) {
	// 	sdsfree(command_str);
	// 	self->message_queue = existing_queue;
	// 	return;
	// }

	// sds payload = XmlExtractMessagePayload(timeout.string);
	// if (!payload) {
	// 	sdsfree(command_str);
	// 	self->message_queue = existing_queue;
	// 	return;
	// }

	// BPLOG(kLvlDebug, "BpSetDoorState: The return value from kReadIOBit(DoorOpen) is %s\n", payload);

	// int dooropen = strncmp(payload, "1", 1);
	// if (state == dooropen) {
	// 	BPLOG(kLvlDebug, "(%s) Door is already in requested state\n", __func__);
	// 	// return;
	// }

	command_str = sdsfromlonglong( kWriteIoBit );
	if ( state ) {
		BPLOG( kLvlDebug, "%s: Opening door\n", __func__ );
		BpQueueMessage( self,
				"type",
				"command",
				"command",
				command_str,
				"bitfunction",
				"OpenDoor",
				"value",
				"1",
				NULL );
	} else {
		BPLOG( kLvlDebug, "%s: Closing door\n", __func__ );
		BpQueueMessage( self,
				"type",
				"command",
				"command",
				command_str,
				"bitfunction",
				"CloseDoor",
				"value",
				"1",
				NULL );
	}
	BpUploadQueuedMessages( self );

	BpQueueMessage(
		self, "type", "command", "command", command_str, "bitfunction", "OpenDoor", "value", "0", NULL );
	BpQueueMessage(
		self, "type", "command", "command", command_str, "bitfunction", "CloseDoor", "value", "0", NULL );
	BpUploadQueuedMessages( self );

	// sdsfree(payload);
	sdsfree( command_str );

	self->message_queue = existing_queue;
}

void BpMoveW( t_Blastpit* self, int angle )
{  // Sets the rotary axis

	sds command_str = sdsfromlonglong( kMoveW );
	sds rotation	= sdsfromlonglong( angle );
	BpQueueMessage( self, "type", "command", "command", command_str, "rotation", rotation, NULL );
	BpUploadQueuedMessages( self );
	sdsfree( command_str );
	sdsfree( rotation );
}

void BpInitMachine( t_Blastpit* self )
{  // Initialise the laser hardware

	sds command_str = sdsfromlonglong( kInitMachine );
	BpQueueMessage( self, "type", "command", "command", command_str, NULL );
	BpUploadQueuedMessages( self );
	sdsfree( command_str );
}

void BpTermMachine( t_Blastpit* self )
{  // Release the laser hardware

	sds command_str = sdsfromlonglong( kTermMachine );
	BpQueueMessage( self, "type", "command", "command", command_str, NULL );
	BpUploadQueuedMessages( self );
	sdsfree( command_str );
}

void BpDisplayLmosWindow( t_Blastpit* self, int visibility )
{  // Show or hide the Lmos ActiveX window

	sds command_str = sdsfromlonglong( visibility == 1 ? kShowLMOS : kHideLMOS );
	BpQueueMessage( self, "type", "command", "command", command_str, NULL );
	BpUploadQueuedMessages( self );
	sdsfree( command_str );
}

void BpSaveQpSets( t_Blastpit* self )
{  // Save any uploaded Qp sets to disk

	sds command_str = sdsfromlonglong( kSaveQpSets );
	BpQueueMessage( self, "type", "command", "command", command_str, NULL );
	BpUploadQueuedMessages( self );
	sdsfree( command_str );
}
