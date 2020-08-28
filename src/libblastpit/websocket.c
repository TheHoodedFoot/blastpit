#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "blastpit.h"
#include "mongoose.h"
#include "sds.h"
// #include "src/mg_net_if.h"
#include "websocket.h"

static struct mg_serve_http_opts s_http_server_opts;

t_Websocket *
websocketNew()
{  // Constructor

	t_Websocket *ws = (t_Websocket *)calloc(1, sizeof(t_Websocket));
	// ws->isServer = false;
	// ws->isConnected = false;
	// ws->evloopIsRunning = false;
	// ws->messageReceived = NULL;
	// ws->messageReceivedCpp = NULL;
	// ws->messageStack = NULL;
	// ws->mongoose.user_data = NULL;

	return ws;
}

void
wsSetMessageReceivedCallback(t_Websocket *self, void (*callback)(void *))
{
	self->messageReceived = callback;
}

void
wsSetMessageReceivedCallbackCpp(t_Websocket *self, void (*callback)(void *, void *))
{
	self->messageReceivedCpp = callback;
}

void
wsSetMessageReceivedObject(t_Websocket *self, void *object)
{
	self->object = object;
}

void
websocketDelete(t_Websocket *ws)
{  // Destructor

	(void)ws;
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
	// char addr[32];

	// char *buf = alloca(msg.len);

	// char buf[500];
	// mg_sock_addr_to_str(&nc->sa, addr, sizeof(addr), MG_SOCK_STRINGIFY_IP | MG_SOCK_STRINGIFY_PORT);

	// snprintf(buf, sizeof(buf), "%s %.*s", addr, (int)msg.len, msg.p);
	// printf("Broadcasting message: %s\n", buf); /* Local echo. */

	// Add message to linked list
	// ((t_Websocket *)nc->user_data)->messageFifo

	for (c = mg_next(nc->mgr, NULL); c != NULL; c = mg_next(nc->mgr, c)) {
		if (c == nc)
			continue; /* Don't send to the sender. */
		LOG(kLvlDebug, "Server sending message to client %p\n", (void *)c);
		mg_send_websocket_frame(c, WEBSOCKET_OP_TEXT, msg.p, msg.len);
	}
}

void
broadcastClient(struct mg_connection *nc, const struct mg_str msg)
{  // Send message to server

	LOG(kLvlDebug, "Client sending message to server %p\n", (void *)nc);
	mg_send_websocket_frame(nc, WEBSOCKET_OP_TEXT, msg.p, msg.len);
}

static void
server_event_handler(struct mg_connection *nc, int ev, void *ev_data)
{
	switch (ev) {
		case MG_EV_WEBSOCKET_HANDSHAKE_DONE: {
			LOG(kLvlDebug, "%s: MG_EV_WEBSOCKET_HANDSHAKE_DONE\n", __func__);
			/* New websocket connection. Tell everybody. */
			// broadcastServer(nc, mg_mk_str("++ joined"));
			break;
		}
		case MG_EV_WEBSOCKET_FRAME: {
			LOG(kLvlDebug, "%s: MG_EV_WEBSOCKET_FRAME\n", __func__);
			struct websocket_message *wm = (struct websocket_message *)ev_data;

			// Parse message

			// Call our message handler callback
			if (((t_Websocket *)nc->user_data)->messageReceived) {
				((t_Websocket *)nc->user_data)->messageReceived(ev_data);
			}

			/* New websocket message. Tell everybody. */
			struct mg_str d = {(char *)wm->data, wm->size};
			broadcastServer(nc, d);
			break;
		}
		case MG_EV_HTTP_REQUEST: {
			LOG(kLvlDebug, "%s: MG_EV_HTTP_REQUEST\n", __func__);
			mg_serve_http(nc, (struct http_message *)ev_data, s_http_server_opts);
			break;
		}
		case MG_EV_CLOSE: {
			LOG(kLvlDebug, "%s: MG_EV_CLOSE\n", __func__);
			/* Disconnect. Tell everybody. */
			// if (is_websocket(nc)) {
			// 	broadcastServer(nc, mg_mk_str("-- left"));
			// }
			break;
		}
		case MG_EV_SEND: {
			// LOG(kLvlDebug, "%s: MG_EV_SEND\n", __func__);
			break;
		}
		case MG_EV_RECV: {
			// LOG(kLvlDebug, "%s: MG_EV_RECV\n", __func__);
			break;
		}
		case MG_EV_ACCEPT: {
			LOG(kLvlDebug, "%s: MG_EV_ACCEPT\n", __func__);
			break;
		}
		case MG_EV_WEBSOCKET_HANDSHAKE_REQUEST: {
			LOG(kLvlDebug, "%s: MG_EV_WEBSOCKET_HANDSHAKE_REQUEST\n", __func__);
			break;
		}
		case MG_EV_WEBSOCKET_CONTROL_FRAME: {
			LOG(kLvlEverything, "%s: MG_EV_WEBSOCKET_CONTROL_FRAME\n", __func__);
			break;
		}
		case MG_EV_POLL: {
			break;
		}
		default:
			LOG(kLvlError, "server_event_handler: Unhandled event (%d)\n", ev);
	}
}

static void
client_event_handler(struct mg_connection *nc, int ev, void *ev_data)
{
	// (void)nc;

	switch (ev) {
		case MG_EV_CONNECT: {
			LOG(kLvlDebug, "%s: MG_EV_CONNECT\n", __func__);
			int status = *((int *)ev_data);
			if (status != 0) {
				LOG(kLvlError, "%s: MG_EV_CONNECT [Connection Error]\n", __func__);
				// printf("-- Connection error: %d\n", status);
			}
			break;
		}
		case MG_EV_WEBSOCKET_HANDSHAKE_DONE: {
			LOG(kLvlDebug, "%s: MG_EV_WEBSOCKET_HANDSHAKE_DONE\n", __func__);
			struct http_message *hm = (struct http_message *)ev_data;
			if (hm->resp_code == 101) {
				// printf("-- Connected\n");
				LOG(kLvlInfo, "%s: Connected\n", __func__);
				((t_Websocket *)nc->user_data)->isConnected = true;
			} else {
				LOG(kLvlError, "client_event_handler: Connection failed with HTTP code %d\n", hm->resp_code);
				// printf("-- Connection failed! HTTP code %d\n", hm->resp_code);
				/* Connection will be closed after this. */
			}
			break;
		}
		case MG_EV_POLL: {
			LOG(kLvlDebug, "%s: MG_EV_POLL\n", __func__);
			// mg_send_websocket_frame(nc, WEBSOCKET_OP_TEXT, client_message, client_message_len);
			break;
		}
		case MG_EV_WEBSOCKET_FRAME: {
			// Message received
			LOG(kLvlDebug, "%s: MG_EV_WEBSOCKET_FRAME\n", __func__);

			// Call our message handler callback
			t_Websocket *ws = (t_Websocket *)nc->user_data;
			if (ws->messageReceivedCpp) {
				LOG(kLvlDebug, "%s: Calling messageReceivedCpp callback\n", __func__);
				ws->messageReceivedCpp(ev_data, ws->object);
			} else {
				LOG(kLvlError, "%s: Client WEBSOCKET_FRAME callback is unset.\n", __func__);
			}

			// Store message
			LOG(kLvlDebug, "%s: Pushing message onto stack\n", __func__);
			struct websocket_message *wm = (struct websocket_message *)ev_data;
			void *message = malloc((int)wm->size);
			memcpy(message, wm->data, (int)wm->size);
			wsPushMessage(ws, message);
			break;
		}
		case MG_EV_CLOSE: {
			LOG(kLvlDebug, "%s: MG_EV_CLOSE\n", __func__);
			// if (((t_Websocket *)nc->user_data)->isConnected)
			// 	printf("-- Disconnected\n");
			((t_Websocket *)nc->user_data)->isConnected = false;
			break;
		}
		case MG_EV_SEND: {
			// LOG(kLvlDebug, "%s: MG_EV_SEND\n", __func__);
			break;
		}
		case MG_EV_RECV: {
			// LOG(kLvlDebug, "%s: MG_EV_RECV\n", __func__);
			break;
		}
		case MG_EV_WEBSOCKET_CONTROL_FRAME: {
			LOG(kLvlDebug, "%s: MG_EV_WEBSOCKET_CONTROL_FRAME\n", __func__);
			break;
		}
		default:
			LOG(kLvlError, "client_event_handler: Unhandled event (%d)\n", ev);
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
		LOG(kLvlError, "%s: mg_bind failed", __func__);
		return kAllocationFailure;
	}

	// Store context
	self->connection->user_data = self;

	mg_set_protocol_http_websocket(self->connection);
	s_http_server_opts.document_root = "src/submodules/mongoose/examples/websocket_chat";  // Serve current directory
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
	self->connection = NULL;

	return true;
}

int
wsClientCreate(t_Websocket *self, const char *address)
{  // Client Constructor

	self->evloopIsRunning = true;

	mg_mgr_init(&self->mongoose, NULL);

	self->connection = mg_connect_ws(&self->mongoose, client_event_handler, address, "wsClientCreate", NULL);
	if (!self->connection) {
		LOG(kLvlError, "wsClientCreate: mg_connect_ws() failed to connect to the server (%s).\n", address);
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
	self->connection = NULL;

	return true;
}

void
wsPoll(t_Websocket *self)
{  // Poll network and trigger callback on activity

	assert(self);

	mg_mgr_poll(&self->mongoose, WS_POLL_TIME_MS);
}

int
wsServerSendMessage(t_Websocket *self, char *data)
{  // Broadcast message to all clients

	LOG(kLvlDebug, "%s: Sending message to all clients\n", __func__);

	if (!self->isServer)
		return kBadLogic;

	broadcastServer(self->connection, mg_mk_str(data));

	return true;
}

int
wsClientSendMessage(t_Websocket *self, char *data)
{  // Send message to the server

	LOG(kLvlDebug, "%s: Sending message to server\n", __func__);

	broadcastClient(self->connection, mg_mk_str(data));

	return true;
}

void
wsPushMessage(t_Websocket *self, void *data)
{  // Add a message to the start of the message stack

	t_Node *message = (t_Node *)malloc(sizeof(t_Node));
	message->data = data;
	message->next = self->messageStack;
	self->messageStack = message;
}

void *
wsPopMessage(t_Websocket *self)
{  // Remove newest message from stack

	// if (!self)
	// 	return NULL;
	// if (!self->messageStack)
	// 	return NULL;

	// void *retval = self->messageStack->data;
	// void *newHead = self->messageStack->next;
	// free(self->messageStack);
	// self->messageStack = newHead;
	// return retval;

	return wsPopMessageAt(self, 0);
}

int
wsGetMessageCount(t_Websocket *self)
{  // Returns the count of the message stack

	int i = 0;
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

	void *retval = node->data;
	void *newHead = node->next;
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
