#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "blastpit.h"
#include "sds.h"
#include "websocket.h"

#ifdef TRACY_ENABLE
// #include "Tracy.hpp"
#include "TracyC.h"
#else
#define ZoneScoped
#define TracyCAlloc( a, b )
#define TracyCFree( a )
#define TracyCPlot( a, b )
#define TracyCZone( a, b )
#define TracyCZoneEnd( a )
#endif

// static struct mg_http_serve_opts s_http_server_opts;

t_Websocket*
websocketNew( void )
{  // Constructor

	t_Websocket* websock = (t_Websocket*)calloc( 1, sizeof( t_Websocket ) );
	TracyCAlloc( websock, sizeof( t_Websocket ) );
	return websock;
}

void
wsSetMessageReceivedCallback( t_Websocket* self, void ( *callback )( void*, void* ) )
{
	self->messageReceived = callback;
}

void
wsSetMessageReceivedObject( t_Websocket* self, void* object )
{
	self->object = object;
}

void
websocketDelete( t_Websocket* websock )
{  // Destructor

	TracyCZone( websocketDelete, true );

	wsFlushMessages( websock );
	free( websock );
	TracyCFree( websock );

	TracyCZoneEnd( websocketDelete );
}

void
wsFlushMessages( t_Websocket* websock )
{  // Drop all messages

	if ( !websock ) {
		return;
	}
	while ( wsGetMessageCount( websock ) > 0 ) {
		free( wsPopMessage( websock ) );
	}
}

// static int
// is_websocket(const struct mg_connection *nc)
// {
// 	return nc->flags & MG_F_IS_WEBSOCKET;
// }

void
broadcastServer( struct mg_connection* nconn, const struct mg_str msg )
{  // Send message to client

	TracyCZone( broadcastServer, true );

	for ( struct mg_connection* con = nconn->mgr->conns; con != NULL; con = con->next ) {
		BPLOG( kLvlDebug, "broadcastServer: Sending message to client %p\n", (void*)con );
		// uint16_t port = (uint16_t)( con->peer.port << 8 ) | ( con->peer.port >> 8 );
		// BPLOG( kLvlDebug, "broadcastServer: port %d\n", port );
		BPLOG( kLvlDebug, "msg.len %lu\n", msg.len );
		BPLOG( kLvlEverything, "msg.ptr %s\n", msg.ptr );
		if ( con != nconn ) {
			mg_ws_send( con, msg.ptr, msg.len, WEBSOCKET_OP_TEXT );
		} else {
			BPLOG( kLvlEverything, "%s: Not sending to duplicate nc\n", __func__ );
		}

		// mg_mgr_poll(con->mgr, 0);
	}

	TracyCZoneEnd( broadcastServer );
}

void
broadcastClient( struct mg_connection* nconn, const struct mg_str msg )
{  // Send message to server

	TracyCZone( broadcastClient, true );

	BPLOG( kLvlDebug, "%s: Client sending message to server\n", __func__ );
	BPLOG( kLvlDebug, "nc: %p\n", (void*)nconn );
	BPLOG( kLvlEverything, "msg.ptr: %p\n", (void*)msg.ptr );
	BPLOG( kLvlDebug, "msg.len: %lu\n", msg.len );
	mg_ws_send( nconn, msg.ptr, msg.len, WEBSOCKET_OP_TEXT );

	TracyCZoneEnd( broadcastClient );
}

static void
server_event_handler( struct mg_connection* nconn, int event, void* ev_data, void* fn_data )
{  // Callback when listening server receives an event

	TracyCZone( server_event_handler, true );

	switch ( event ) {
		// case MG_EV_WEBSOCKET_HANDSHAKE_DONE: {
		// 	BPLOG(kLvlDebug, "%s: MG_EV_WEBSOCKET_HANDSHAKE_DONE\n", __func__);
		// 	/* New websocket connection. Tell everybody. */
		// 	// broadcastServer(nc, mg_mk_str("++ joined"));
		// 	break;
		// }
		case MG_EV_WS_MSG: {
			assert( ev_data );
			BPLOG( kLvlDebug, "%s: MG_EV_WS_FRAME\n", __func__ );
			struct mg_ws_message* websock_msg = (struct mg_ws_message*)ev_data;

			// Parse message

			// Call our message handler callback
			if ( ( (t_Websocket*)fn_data )->messageReceived ) {
				( (t_Websocket*)fn_data )->messageReceived( ev_data, NULL );
			} else {
				BPLOG( kLvlWarn,
				       "server MG_EV_WS_MSG: Message received but no message handler callback set for "
				       "connection %p\n",
				       (void*)nconn );
			}

			/* New websocket message. Tell everybody. */
			struct mg_str ws_string = mg_str_n( (char*)websock_msg->data.ptr, websock_msg->data.len );
			if ( websock_msg->data.len == 0 ) {
				BPLOG( kLvlWarn,
				       "server MG_EV_WS_MSG: (nc = %p) Message received has length of zero. Not "
				       "broadcasting to clients.\n",
				       (void*)nconn );
				break;
			}

			broadcastServer( nconn, ws_string );
			break;
		}
		case MG_EV_HTTP_MSG: {
			BPLOG( kLvlDebug, "%s: MG_EV_HTTP_MSG\n", __func__ );
			struct mg_http_message* hmsg = (struct mg_http_message*)ev_data;
			// We should check whether the client is requesting
			// http files, or whether they want to upgrade to websockets.
			// For now, always assume they want to upgrade.

			// Upgrade to WebSockets
			mg_ws_upgrade( nconn, hmsg, NULL );

			// Reply HTTP
			// mg_http_serve_dir(nc, (struct mg_http_message*)ev_data, &s_http_server_opts);
			break;
		}
		case MG_EV_CLOSE: {
			BPLOG( kLvlDebug, "%s: MG_EV_CLOSE\n", __func__ );
			/* Disconnect. Tell everybody. */
			// if (is_websocket(nc)) {
			// 	broadcastServer(nc, mg_mk_str("-- left"));
			// }
			break;
		}
		case MG_EV_WRITE: {
			BPLOG( kLvlDebug, "%s: MG_EV_WRITE\n", __func__ );
			break;
		}
		case MG_EV_READ: {
			BPLOG( kLvlDebug, "%s: MG_EV_READ\n", __func__ );
			break;
		}
		case MG_EV_ACCEPT: {
			BPLOG( kLvlDebug, "%s: MG_EV_ACCEPT\n", __func__ );
			break;
		}
		// case MG_EV_WEBSOCKET_HANDSHAKE_REQUEST: {
		// 	BPLOG(kLvlDebug, "%s: MG_EV_WEBSOCKET_HANDSHAKE_REQUEST\n", __func__);
		// 	break;
		// }
		// case MG_EV_WEBSOCKET_CONTROL_FRAME: {
		// 	BPLOG(kLvlEverything, "%s: MG_EV_WEBSOCKET_CONTROL_FRAME\n", __func__);
		// 	break;
		// }
		case MG_EV_POLL: {
			break;
		}
		default:
			BPLOG( kLvlError, "server_event_handler: Unhandled event (%d)\n", event );
	}

	TracyCZoneEnd( server_event_handler );
}

static void
client_event_handler( struct mg_connection* nconn, int event, void* ev_data, void* fn_data )
{
	TracyCZone( client_event_handler, true );

	(void)nconn;

	if ( !ev_data ) {
		BPLOG( kLvlDebug, "%s: ev_data is null\n", __func__ );
	}

	switch ( event ) {
		case MG_EV_CONNECT: {
			BPLOG( kLvlDebug, "%s: MG_EV_CONNECT\n", __func__ );
			// int status = *((int*)ev_data);
			// if (status != 0) {
			// 	BPLOG(kLvlError, "%s: MG_EV_CONNECT [Connection Error]\n", __func__);
			// 	// printf("-- Connection error: %d\n", status);
			// }
			break;
		}
		case MG_EV_WS_OPEN: {
			BPLOG( kLvlDebug, "%s: MG_EV_WS_OPEN\n", __func__ );
			( (t_Websocket*)fn_data )->isConnected = true;
			break;
		}
		case MG_EV_POLL: {
			BPLOG( kLvlEverything, "%s: MG_EV_POLL\n", __func__ );
			// mg_send_websocket_frame(nc, WEBSOCKET_OP_TEXT, client_message, client_message_len);
			break;
		}
		case MG_EV_WS_MSG: {
			// Message received
			BPLOG( kLvlDebug, "client_event_handler: Message received by nc %p\n", (void*)nconn );

			// Call our message handler callback
			t_Websocket* websock = (t_Websocket*)fn_data;
			if ( websock->messageReceived ) {
				BPLOG( kLvlDebug, "%s: Calling messageReceivedCpp callback\n", __func__ );
				websock->messageReceived( ev_data, websock->object );
			} else {
				BPLOG( kLvlWarn,
				       "%s: WebSocket message received but client WEBSOCKET_FRAME callback is unset.\n",
				       __func__ );
			}

			// Store message
			BPLOG( kLvlDebug, "%s: Pushing message onto stack\n", __func__ );
			assert( ev_data );
			struct mg_ws_message* websock_msg = (struct mg_ws_message*)ev_data;
			BPLOG( kLvlDebug, "Message size: %d\n", (int)websock_msg->data.len );

			// We could use calloc here, but we only need to zero the last byte
			void* message = malloc( (int)websock_msg->data.len + 1 );  // Allow null terminator
			assert( message );
			TracyCAlloc( message, (int)websock_msg->data.len + 1 );
			BPLOG( kLvlDebug, "Message pointer: %p\n", message );
			*( (char*)message + (int)websock_msg->data.len ) = 0;

			void* destination =
				memmove( message, websock_msg->data.ptr, (int)websock_msg->data.len );	// NOLINT

			(void)destination;
			assert( destination );
			wsPushMessage( websock, message );
			break;
		}
		case MG_EV_CLOSE: {
			BPLOG( kLvlDebug, "%s: MG_EV_CLOSE\n", __func__ );
			// if (((t_Websocket *)nc->user_data)->isConnected)
			// 	printf("-- Disconnected\n");
			( (t_Websocket*)fn_data )->isConnected = false;
			break;
		}
		case MG_EV_WRITE: {
			BPLOG( kLvlDebug, "%s: MG_EV_WRITE\n", __func__ );
			break;
		}
		case MG_EV_READ: {
			BPLOG( kLvlDebug, "%s: MG_EV_READ\n", __func__ );
			break;
		}
		case MG_EV_RESOLVE: {
			BPLOG( kLvlDebug, "%s: MG_EV_RESOLVE\n", __func__ );
			break;
		}
		// case MG_EV_WEBSOCKET_CONTROL_FRAME: {
		// 	BPLOG(kLvlEverything, "%s: MG_EV_WEBSOCKET_CONTROL_FRAME\n", __func__);
		// 	break;
		// }
		default:
			BPLOG( kLvlError, "client_event_handler: Unhandled event (%d)\n", event );
	}

	TracyCZoneEnd( client_event_handler );
}

int
wsServerCreate( t_Websocket* self, const char* listen_address )
{  // Server Constructor

	TracyCZone( wsServerCreate, true );

	// assert(atoi(listen_address));

	self->evloopIsRunning = true;

	mg_mgr_init( &self->mongoose );

	self->connection = mg_http_listen( &self->mongoose, listen_address, server_event_handler, (void*)self );
	if ( !self->connection ) {
		BPLOG( kLvlError, "%s: mg_http_listen failed", __func__ );
		mg_mgr_free( &self->mongoose );
		return kAllocationFailure;
	}

	// Store context
	self->connection->fn_data = self;

	// mg_set_protocol_http_websocket(self->connection);
	// s_http_server_opts.document_root =
	// 	"src/submodules/mongoose/examples/websocket_chat";  // Serve current directory
	// s_http_server_opts.enable_directory_listing = "yes";

	return kSuccess;

	TracyCZoneEnd( wsServerCreate );
}

int
wsServerDestroy( t_Websocket* self )
{  // Server Destructor

	TracyCZone( wsServerDestroy, true );

	if ( !self->connection ) {
		return kNullResource;
	}

	// Free any dynamically allocated memory here
	// Note: This automatically runs mg_mgr_poll()
	mg_mgr_free( &self->mongoose );

	// Invalidate
	self->evloopIsRunning = false;
	self->connection      = NULL;

	// wsFlushMessages(self);
	return true;

	TracyCZoneEnd( wsServerDestroy );
}

int
wsClientCreate( t_Websocket* self, const char* address )
{  // Client Constructor

	TracyCZone( wsClientCreate, true );

	self->evloopIsRunning = true;

	mg_mgr_init( &self->mongoose );

	self->connection = mg_ws_connect( &self->mongoose, address, client_event_handler, NULL, NULL );
	if ( !self->connection ) {
		BPLOG( kLvlError, "wsClientCreate: mg_connect_ws() failed to connect to the server (%s).\n", address );
		return kConnectionFailure;
	}

	// Store context
	self->connection->fn_data = self;

	return kSuccess;

	TracyCZoneEnd( wsClientCreate );
}

int
wsClientDestroy( t_Websocket* self )
{  // Client Destructor

	TracyCZone( wsClientDestroy, true );

	if ( !self->connection ) {
		return kNullResource;
	}

	// Free any dynamically allocated memory here
	mg_mgr_free( &self->mongoose );

	// Invalidate
	self->evloopIsRunning = false;
	self->connection      = NULL;

	// wsFlushMessages(self);
	// self = NULL;
	return true;

	TracyCZoneEnd( wsClientDestroy );
}

void
wsPoll( t_Websocket* self )
{  // Poll network and trigger callback on activity

	wsPollWithTimeout( self, WS_POLL_TIME_MS );
}

void
wsPollWithTimeout( t_Websocket* self, int timeout )
{  // Poll network and trigger callback on activity

	assert( self );

	mg_mgr_poll( &self->mongoose, timeout );
}

int
wsServerSendMessage( t_Websocket* self, char* data )
{  // Broadcast message to all clients

	TracyCZone( wsServerSendMessage, true );

	BPLOG( kLvlDebug, "%s: Sending message to all clients\n", __func__ );
	BPLOG( kLvlDebug, "Message size: %ld\n", strlen( data ) );

	if ( !self->isServer ) {
		return kBadLogic;
	}

	broadcastServer( self->connection, mg_str( data ) );
	wsPoll( self );

	return true;

	TracyCZoneEnd( wsServerSendMessage );
}

int
wsClientSendMessage( t_Websocket* self, char* data )
{  // Send message to the server

	TracyCZone( wsClientSendMessage, true );

	if ( !data ) {
		return kInvalid;
	}

	BPLOG( kLvlDebug, "%s: Sending message to server\n", __func__ );
	BPLOG( kLvlDebug, "Message size: %ld\n", strlen( data ) );
	broadcastClient( self->connection, mg_str( data ) );
	// wsPoll(self);

	return kSuccess;

	TracyCZoneEnd( wsClientSendMessage );
}

void
wsPushMessage( t_Websocket* self, void* data )
{  // Add a message to the start of the message stack

	TracyCZone( wsPushMessage, true );

	t_Node* message = (t_Node*)malloc( sizeof( t_Node ) );
	TracyCAlloc( message, sizeof( t_Node ) );

	BPLOG( kLvlDebug, "Pushing message pointer: %p\n", (void*)message );
	message->data	   = data;
	message->next	   = self->messageStack;
	self->messageStack = message;

	TracyCZoneEnd( wsPushMessage );
}

void*
wsPopMessage( t_Websocket* self )
{  // Remove newest message from stack

	return wsPopMessageAt( self, 0 );
}

int
wsGetMessageCount( t_Websocket* self )
{  // Returns the count of the message stack

	int	i    = 0;  // NOLINT
	t_Node* node = self->messageStack;
	for ( ; node; i++ ) {
		node = node->next;
	}
	return i;
}

void*
wsPopMessageAt( t_Websocket* self, int index )
{  // Pop the message data for a specific index

	if ( !self ) {
		return NULL;
	}
	if ( !self->messageStack ) {
		return NULL;
	}

	t_Node* node = self->messageStack;
	t_Node* prev = NULL;
	for ( int i = 0; i < index; i++ ) {
		if ( node ) {
			prev = node;
			node = node->next;
		}
	}

	if ( !node ) {
		return NULL;
	}

	void* retval  = node->data;
	void* newHead = node->next;
	BPLOG( kLvlDebug, "Freeing message pointer: %p\n", (void*)node );
	BPLOG( kLvlDebug, "which points to data: %p\n", (void*)node->data );
	free( node );
	TracyCFree( node );

	if ( !prev ) {
		// We were index 0, so just point to next item
		self->messageStack = newHead;
	} else {
		// Link previous item to next item
		prev->next = newHead;
	}

	return retval;
}

void*
wsReadMessageAt( t_Websocket* self, int index )
{  // Get the message data for a specific index

	if ( !self ) {
		return NULL;
	}
	if ( !self->messageStack ) {
		return NULL;
	}

	t_Node* node = self->messageStack;
	for ( int i = 0; i < index; i++ ) {
		if ( node ) {
			node = node->next;
		}
	}

	if ( !node ) {
		return NULL;
	}

	return node->data;
}

WsMessage
ExtractWsMessageData( void* ev_data )
{  // Helper function to extract the websocket message data
	// Putting it here avoids requiring mongoose.h elsewhere

	WsMessage retval;

	struct mg_ws_message* websock_msg = (struct mg_ws_message*)ev_data;
	retval.size			  = (int)websock_msg->data.len;
	retval.data			  = websock_msg->data.ptr;

	return retval;
}
