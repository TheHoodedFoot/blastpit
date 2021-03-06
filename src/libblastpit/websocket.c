#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

//#include "mongoose.h"
#include "blastpit.h"
#include "sds.h"
// #include "src/mg_net_if.h"
#include "websocket.h"

static struct mg_serve_http_opts s_http_server_opts;

t_Websocket *
websocketNew()
{  // Constructor

	t_Websocket *ws = (t_Websocket *)calloc(1, sizeof(t_Websocket));
	return ws;
}

void
wsSetMessageReceivedCallback(t_Websocket *self, void (*callback)(void *, void *))
{
	self->messageReceived = callback;
}

void
wsSetMessageReceivedObject(t_Websocket *self, void *object)
{
	self->object = object;
}

void
websocketDelete(t_Websocket *ws)
{  // Destructor

	wsFlushMessages(ws);
	free(ws);
}

void
wsFlushMessages(t_Websocket *ws)
{  // Drop all messages

	if (!ws)
		return;
	while (wsGetMessageCount(ws) > 0) {
		free(wsPopMessage(ws));
	}
}

// static int
// is_websocket(const struct mg_connection *nc)
// {
// 	return nc->flags & MG_F_IS_WEBSOCKET;
// }

void
broadcastServer(struct mg_connection *nc, const struct mg_str msg)
{  // Send message to all connected clients

	struct mg_connection *c;

	for (c = mg_next(nc->mgr, NULL); c != NULL; c = mg_next(nc->mgr, c)) {
		if (c == nc)
			continue;  // Don't send to the sender.
		BPLOG(kLvlDebug, "Server sending message to client %p\n", (void *)c);
		mg_send_websocket_frame(c, WEBSOCKET_OP_TEXT, msg.p, msg.len);
	}
}

void
broadcastClient(struct mg_connection *nc, const struct mg_str msg)
{  // Send message to server

	BPLOG(kLvlDebug, "Client sending message to server %p\n", (void *)nc);
	mg_send_websocket_frame(nc, WEBSOCKET_OP_TEXT, msg.p, msg.len);
}

static void
server_event_handler(struct mg_connection *nc, int ev, void *ev_data)
{
	switch (ev) {
		case MG_EV_WEBSOCKET_HANDSHAKE_DONE: {
			BPLOG(kLvlDebug, "%s: MG_EV_WEBSOCKET_HANDSHAKE_DONE\n", __func__);
			/* New websocket connection. Tell everybody. */
			// broadcastServer(nc, mg_mk_str("++ joined"));
			break;
		}
		case MG_EV_WEBSOCKET_FRAME: {
			BPLOG(kLvlDebug, "%s: MG_EV_WEBSOCKET_FRAME\n", __func__);
			struct websocket_message *wm = (struct websocket_message *)ev_data;

			// Parse message

			// Call our message handler callback
			if (((t_Websocket *)nc->user_data)->messageReceived) {
				((t_Websocket *)nc->user_data)->messageReceived(ev_data, NULL);
			}

			/* New websocket message. Tell everybody. */
			struct mg_str d = {(char *)wm->data, wm->size};
			broadcastServer(nc, d);
			break;
		}
		case MG_EV_HTTP_REQUEST: {
			BPLOG(kLvlDebug, "%s: MG_EV_HTTP_REQUEST\n", __func__);
			mg_serve_http(nc, (struct http_message *)ev_data, s_http_server_opts);
			break;
		}
		case MG_EV_CLOSE: {
			BPLOG(kLvlDebug, "%s: MG_EV_CLOSE\n", __func__);
			/* Disconnect. Tell everybody. */
			// if (is_websocket(nc)) {
			// 	broadcastServer(nc, mg_mk_str("-- left"));
			// }
			break;
		}
		case MG_EV_SEND: {
			// BPLOG(kLvlDebug, "%s: MG_EV_SEND\n", __func__);
			break;
		}
		case MG_EV_RECV: {
			// BPLOG(kLvlDebug, "%s: MG_EV_RECV\n", __func__);
			break;
		}
		case MG_EV_ACCEPT: {
			BPLOG(kLvlDebug, "%s: MG_EV_ACCEPT\n", __func__);
			break;
		}
		case MG_EV_WEBSOCKET_HANDSHAKE_REQUEST: {
			BPLOG(kLvlDebug, "%s: MG_EV_WEBSOCKET_HANDSHAKE_REQUEST\n", __func__);
			break;
		}
		case MG_EV_WEBSOCKET_CONTROL_FRAME: {
			BPLOG(kLvlEverything, "%s: MG_EV_WEBSOCKET_CONTROL_FRAME\n", __func__);
			break;
		}
		case MG_EV_POLL: {
			break;
		}
		default:
			BPLOG(kLvlError, "server_event_handler: Unhandled event (%d)\n", ev);
	}
}

static void
client_event_handler(struct mg_connection *nc, int ev, void *ev_data)
{
	// (void)nc;

	switch (ev) {
		case MG_EV_CONNECT: {
			BPLOG(kLvlDebug, "%s: MG_EV_CONNECT\n", __func__);
			int status = *((int *)ev_data);
			if (status != 0) {
				BPLOG(kLvlError, "%s: MG_EV_CONNECT [Connection Error]\n", __func__);
				// printf("-- Connection error: %d\n", status);
			}
			break;
		}
		case MG_EV_WEBSOCKET_HANDSHAKE_DONE: {
			BPLOG(kLvlDebug, "%s: MG_EV_WEBSOCKET_HANDSHAKE_DONE\n", __func__);
			struct http_message *hm = (struct http_message *)ev_data;
			if (hm->resp_code == 101) {
				BPLOG(kLvlInfo, "%s: Connected\n", __func__);
				((t_Websocket *)nc->user_data)->isConnected = true;
			} else {
				BPLOG(kLvlError, "client_event_handler: Connection failed with HTTP code %d\n",
				    hm->resp_code);
				// printf("-- Connection failed! HTTP code %d\n", hm->resp_code);
				/* Connection will be closed after this. */
			}
			break;
		}
		case MG_EV_POLL: {
			BPLOG(kLvlEverything, "%s: MG_EV_POLL\n", __func__);
			// mg_send_websocket_frame(nc, WEBSOCKET_OP_TEXT, client_message, client_message_len);
			break;
		}
		case MG_EV_WEBSOCKET_FRAME: {
			// Message received
			BPLOG(kLvlDebug, "%s: MG_EV_WEBSOCKET_FRAME\n", __func__);

			// Call our message handler callback
			t_Websocket *ws = (t_Websocket *)nc->user_data;
			if (ws->messageReceived) {
				BPLOG(kLvlDebug, "%s: Calling messageReceivedCpp callback\n", __func__);
				ws->messageReceived(ev_data, ws->object);
			} else {
				BPLOG(kLvlDebug, "%s: Client WEBSOCKET_FRAME callback is unset.\n", __func__);
			}

			// Store message
			BPLOG(kLvlDebug, "%s: Pushing message onto stack\n", __func__);
			struct websocket_message *wm = (struct websocket_message *)ev_data;
			BPLOG(kLvlDebug, "Message size: %d\n", (int)wm->size);

			// We could use calloc here, but we only need to zero the last byte
			void *message = malloc((int)wm->size + 1);  // Allow null terminator
			assert(message);
			BPLOG(kLvlDebug, "Message pointer: %p\n", message);
			*((char *)message + (int)wm->size) = 0;

			void *destination = memmove(message, wm->data, (int)wm->size);
			assert(destination);
			wsPushMessage(ws, message);
			break;
		}
		case MG_EV_CLOSE: {
			BPLOG(kLvlDebug, "%s: MG_EV_CLOSE\n", __func__);
			// if (((t_Websocket *)nc->user_data)->isConnected)
			// 	printf("-- Disconnected\n");
			((t_Websocket *)nc->user_data)->isConnected = false;
			break;
		}
		case MG_EV_SEND: {
			// BPLOG(kLvlDebug, "%s: MG_EV_SEND\n", __func__);
			break;
		}
		case MG_EV_RECV: {
			// BPLOG(kLvlDebug, "%s: MG_EV_RECV\n", __func__);
			break;
		}
		case MG_EV_WEBSOCKET_CONTROL_FRAME: {
			BPLOG(kLvlEverything, "%s: MG_EV_WEBSOCKET_CONTROL_FRAME\n", __func__);
			break;
		}
		default:
			BPLOG(kLvlError, "client_event_handler: Unhandled event (%d)\n", ev);
	}
}

int
wsServerCreate(t_Websocket *self, const char *port)
{  // Server Constructor

	assert(atoi(port));

	self->evloopIsRunning = true;

	mg_mgr_init(&self->mongoose, NULL);

	self->connection = mg_bind(&self->mongoose, port, server_event_handler);
	if (!self->connection) {
		BPLOG(kLvlError, "%s: mg_bind failed", __func__);
		mg_mgr_free(&self->mongoose);
		return kAllocationFailure;
	}

	// Store context
	self->connection->user_data = self;

	mg_set_protocol_http_websocket(self->connection);
	s_http_server_opts.document_root =
		"src/submodules/mongoose/examples/websocket_chat";  // Serve current directory
	s_http_server_opts.enable_directory_listing = "yes";

	return kSuccess;
}

int
wsServerDestroy(t_Websocket *self)
{  // Server Destructor

	if (!self->connection)
		return kNullResource;

	// Free any dynamically allocated memory here
	// Note: This automatically runs mg_mgr_poll()
	mg_mgr_free(&self->mongoose);

	// Invalidate
	self->evloopIsRunning = false;
	self->connection      = NULL;

	// wsFlushMessages(self);
	return true;
}

int
wsClientCreate(t_Websocket *self, const char *address)
{  // Client Constructor

	self->evloopIsRunning = true;

	mg_mgr_init(&self->mongoose, NULL);

	self->connection = mg_connect_ws(&self->mongoose, client_event_handler, address, "wsClientCreate", NULL);
	if (!self->connection) {
		BPLOG(kLvlError, "wsClientCreate: mg_connect_ws() failed to connect to the server (%s).\n", address);
		return kConnectionFailure;
	}

	// Store context
	self->connection->user_data = self;

	return kSuccess;
}

int
wsClientDestroy(t_Websocket *self)
{  // Client Destructor

	if (!self->connection)
		return kNullResource;

	// Free any dynamically allocated memory here
	mg_mgr_free(&self->mongoose);

	// Invalidate
	self->evloopIsRunning = false;
	self->connection      = NULL;

	// wsFlushMessages(self);
	// self = NULL;
	return true;
}

void
wsPoll(t_Websocket *self)
{  // Poll network and trigger callback on activity

	wsPollWithTimeout(self, WS_POLL_TIME_MS);
}

void
wsPollWithTimeout(t_Websocket *self, int timeout)
{  // Poll network and trigger callback on activity

	assert(self);

	mg_mgr_poll(&self->mongoose, timeout);
}

int
wsServerSendMessage(t_Websocket *self, char *data)
{  // Broadcast message to all clients

	BPLOG(kLvlDebug, "%s: Sending message to all clients\n", __func__);
	BPLOG(kLvlDebug, "Message size: %ld\n", strlen(data));

	if (!self->isServer)
		return kBadLogic;

	broadcastServer(self->connection, mg_mk_str(data));
	wsPoll(self);

	return true;
}

int
wsClientSendMessage(t_Websocket *self, char *data)
{  // Send message to the server

	if (!data)
		return kInvalid;

	BPLOG(kLvlDebug, "%s: Sending message to server\n", __func__);
	BPLOG(kLvlDebug, "Message size: %ld\n", strlen(data));
	broadcastClient(self->connection, mg_mk_str(data));
	// wsPoll(self);

	return kSuccess;
}

void
wsPushMessage(t_Websocket *self, void *data)
{  // Add a message to the start of the message stack

	t_Node *message = (t_Node *)malloc(sizeof(t_Node));
	BPLOG(kLvlDebug, "Pushing message pointer: %p\n", (void *)message);
	message->data	   = data;
	message->next	   = self->messageStack;
	self->messageStack = message;
}

void *
wsPopMessage(t_Websocket *self)
{  // Remove newest message from stack

	return wsPopMessageAt(self, 0);
}

int
wsGetMessageCount(t_Websocket *self)
{  // Returns the count of the message stack

	int	i    = 0;
	t_Node *node = self->messageStack;
	for (; node; i++)
		node = node->next;
	return i;
}

void *
wsPopMessageAt(t_Websocket *self, int index)
{  // Pop the message data for a specific index

	if (!self)
		return NULL;
	if (!self->messageStack)
		return NULL;

	t_Node *node = self->messageStack;
	t_Node *prev = NULL;
	for (int i = 0; i < index; i++) {
		if (node) {
			prev = node;
			node = node->next;
		}
	}

	if (!node)
		return NULL;

	void *retval  = node->data;
	void *newHead = node->next;
	BPLOG(kLvlDebug, "Freeing message pointer: %p\n", (void *)node);
	BPLOG(kLvlDebug, "which points to data: %p\n", (void *)node->data);
	free(node);

	if (!prev) {
		// We were index 0, so just point to next item
		self->messageStack = newHead;
	} else {
		// Link previous item to next item
		prev->next = newHead;
	}

	return retval;
}

void *
wsReadMessageAt(t_Websocket *self, int index)
{  // Get the message data for a specific index

	if (!self)
		return NULL;
	if (!self->messageStack)
		return NULL;

	t_Node *node = self->messageStack;
	for (int i = 0; i < index; i++)
		if (node)
			node = node->next;

	if (!node)
		return NULL;

	return node->data;
}

WsMessage
ExtractWsMessageData(void *ev_data)
{  // Helper function to extract the websocket message data
	// Putting it here avoids requiring mongoose.h elsewhere

	WsMessage retval;

	struct websocket_message *wm = (struct websocket_message *)ev_data;
	retval.size		     = (int)wm->size;
	retval.data		     = wm->data;

	return retval;
}
