#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>

#include <stdio.h>
#include <string.h>
#include "blastpit.h"
#include "sds.h"
#include "websocket.h"
#include "xml.hpp"

const char*
bpCommandName(int command)
{
	return bpCommandString[command];
}
const char*
bpRetvalName(int retval)
{
	return bpRetvalString[retval];
}

// void
// debugInfo(t_Blastpit* bp)
// {
// 	LOG(kLvlDebug, "messageReceived = %p\n", (void*)bp->ws->messageReceived);
// 	LOG(kLvlDebug, "messageReceived = %p\n", (void*)bp->ws->messageReceivedCpp);

// 	(void) bp;
// }

t_Blastpit*
blastpitNew()
{  // Constructor

	t_Blastpit* bp = (t_Blastpit*)malloc(sizeof(t_Blastpit));
	if (bp) {
		bp->ws = websocketNew();
	}

	return bp;
}

void
blastpitDelete(t_Blastpit* bp)
{  // Destructor

	if (bp->ws)
		websocketDelete(bp->ws);
	free(bp);
}

int
serverCreate(t_Blastpit* self, const char* port)
{  // Start a listening server

	assert(port != NULL);
	if (self->ws->isServer)
		return kAlreadyInUse;  // Already in use

	int result = wsServerCreate(self->ws, port);
	return result;
}

void
serverDestroy(t_Blastpit* self)
{  // Close the WebSocket server and free any resources

	wsServerDestroy(self->ws);
}

int
connectToServer(t_Blastpit* self, const char* server, int timeout_ms)
{  // Connects a client to the server

	// TODO: timeout_ms may not be needed with WebSockets

	assert(self->ws);

	// Prevent servers connecting to servers
	if (self->ws->isServer)
		return kBadLogic;

	wsClientCreate(self->ws, server);

	// Wait until connected or timeout
	// Note: This relies on the server running and polling itself
	while (!self->ws->isConnected) {
		pollMessages(self);
		// TODO: Convert these magic numbers to DEFINES
		usleep(100000);
		timeout_ms -= 100;
		if (timeout_ms <= 0) {
			LOG(kLvlDebug, "%s: Timed out connecting to server\n", __func__);
			return kConnectionFailure;
		}
	}

	return kSuccess;
}

int
waitForConnection(t_Blastpit* self, int timeout)
{  // Repeatedly poll until we connect to server or timeout

	(void)timeout;

	while (timeout > 0) {
		if (self->ws->isConnected) {
			fprintf(stderr, "Now connected\n");
			return 17;
		}
		pollMessages(self);
		usleep(500);
		timeout -= 500;
	}

	return false;
}

void
disconnectFromServer(t_Blastpit* self)
{
	(void)self;
}

void
pollMessages(t_Blastpit* self)
{  // Check for and process network activity

	wsPoll(self->ws);
}

void
sendClientMessage(t_Blastpit* self, const char* message)
{
	wsClientSendMessage(self->ws, (char*)message);
}

void
sendServerMessage(t_Blastpit* self, const char* message)
{
	wsServerSendMessage(self->ws, (char*)message);
}

int
clearQPSets(t_Blastpit* self)
{
	(void)self;

	/* t_Mqtt* net = self->ws; */

	return false;
}

void
registerCallback(t_Blastpit* self, void (*callback)(void*))
{  // Specify an additional callback upon message receipt

	wsSetMessageReceivedCallback(self->ws, callback);
}

void
registerCallbackCpp(t_Blastpit* self, void (*callback)(void*, void*))
{  // Specify an additional callback upon message receipt

	wsSetMessageReceivedCallbackCpp(self->ws, callback);
}

void
registerObject(t_Blastpit* self, void* object)
{  // Specify an instance to call the callback method

	wsSetMessageReceivedObject(self->ws, object);
}

int
bp_sendMessage(t_Blastpit* self, int id, const char* message)
{
	char* id_message = xml_setId(id, message);
	if (!id_message)
		return kSetterFailure;

	if (self->ws->isServer) {
		sendServerMessage(self, id_message);
	} else {
		if (!self->ws->isConnected)
			return kConnectionFailure;
		sendClientMessage(self, id_message);
	}

	free(id_message);

	return kSuccess;
}

int
bp_sendMessageAndWait(t_Blastpit* self, int id, const char* message, int timeout)
{ /* Sends message and waits. Returns xml of reply with matching id */

	int result = bp_sendMessage(self, id, message);
	if (result != kSuccess) {
		return result;
	}

	return bp_waitForXml(self, id, timeout, false) != NULL ? kSuccess : kReplyTimeout;
}

char*
bp_sendMessageAndWaitForString(t_Blastpit* self, int id, const char* message, int timeout)
{ /* Sends message and waits. Returns xml of reply with matching id */

	int result = bp_sendMessage(self, id, message);
	if (result != kSuccess) {
		return NULL;
	}

	return bp_waitForString(self, id, timeout);
}

char*
bp_waitForXml(t_Blastpit* self, int id, int timeout, int del)
{ /* Return the message data as a string, or NULL */

	for (int i = 0; i < timeout; i++) {
		if (getMessageCount(self) > 0) {
			/* Test every message with getMessageAt() */
			for (int j = 0; j < getMessageCount(self); j++) {
				char* message = readMessageAt(self, j);

				if (message) {
					/* fprintf(stderr, "(bp_waitForString)
					 * Checking message\n"); */
					if (xml_getId(message) == id) {
						/* fprintf(stderr,
						 * "(bp_waitForString) message
						 * id matched\n"); */
						if (del)
							popMessageAt(
								self, j);
						return message;
					}
				}
			}
		}
		usleep(1000);
		if (!(i % 10))
			pollMessages(self);
	}
	return NULL;
}

char*
bp_waitForString(t_Blastpit* self, int id, int timeout)
{
	char* result = xml_getCommandString(bp_waitForXml(self, id, timeout, false));
	return result ? result : BP_EMPTY_STRING;
}

int
sendCommand(t_Blastpit* self, int id, int command)
{
	assert(self);

	char message[] = "<command id=xxxxx>xx</command>";

	snprintf(message, sizeof(message) / sizeof(message[0]), "<command id=\"%d\">%d</command>", id, command);
	return bp_sendMessage(self, id, message);
}

int
bp_sendCommandAndWait(t_Blastpit* self, int id, int command, int timeout)
{
	if (!sendCommand(self, id, command)) {
		LOG(kLvlDebug, "%s: Failed to send command", __func__);
		return kCommandFailed;
	}
	if (bp_waitForXml(self, id, timeout, false) == NULL) {
		LOG(kLvlDebug, "%s: Timed out waiting for XML", __func__);
		return kReplyTimeout;
	}

	return kSuccess;
}

bool
bp_isConnected(t_Blastpit* self)
{
	return self->ws->isConnected;
}

int
bp_waitForSignal(t_Blastpit* self, int signal, int timeout)
{ /* Waits for an Lmos signal */

	int result = xml_getId(bp_waitForXml(self, signal, timeout, true));
	return result ? result : false;
}

int
bp_getNextFreeID(t_Blastpit* self)
{
	(void)self;
	// return mqtt_getNextFreeID(self->ws);
	return -1;
}

char*
popMessage(t_Blastpit* self)
{  // Pop a message from the stack
	// The message must be freed by the user

	return popMessageAt(self, 0);
}

char*
popMessageAt(t_Blastpit* self, int index)
{  // Pop a message at a specific index

	return (char*)wsPopMessageAt(self->ws, index);
}

int
getMessageCount(t_Blastpit* self)
{  // Get the count of the message stack

	return wsGetMessageCount(self->ws);
}

char*
readMessageAt(t_Blastpit* self, int index)
{  // Get the message for a specific index without deleting

	return (char*)wsReadMessageAt(self->ws, index);
}
