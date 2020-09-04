#include <assert.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

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

t_Blastpit*
blastpitNew()
{  // Constructor

	t_Blastpit* bp = (t_Blastpit*)malloc(sizeof(t_Blastpit));
	if (bp) {
		bp->ws = (void*)websocketNew();
		bp->highest_id = 0;
	}

	return bp;
}

void
blastpitDelete(t_Blastpit* bp)
{  // Destructor

	if (bp->ws)
		websocketDelete((t_Websocket*)bp->ws);
	free(bp);
}

int
serverCreate(t_Blastpit* self, const char* port)
{  // Start a listening server

	assert(port != NULL);
	if (((t_Websocket*)self->ws)->isServer)
		return kAlreadyInUse;  // Already in use

	int result = wsServerCreate((t_Websocket*)self->ws, port);
	return result;
}

void
serverDestroy(t_Blastpit* self)
{  // Close the WebSocket server and free any resources

	wsServerDestroy((t_Websocket*)self->ws);
}

int
connectToServer(t_Blastpit* self, const char* server, int timeout_ms)
{  // Connects a client to the server

	// TODO: timeout_ms may not be needed with WebSockets

	assert(self->ws);

	// Prevent servers connecting to servers
	if (((t_Websocket*)self->ws)->isServer)
		return kBadLogic;

	wsClientCreate((t_Websocket*)self->ws, server);

	// Wait until connected or timeout
	// Note: This relies on the server running and polling itself
	while (!((t_Websocket*)self->ws)->isConnected) {
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
		if (((t_Websocket*)self->ws)->isConnected) {
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

	wsPoll((t_Websocket*)self->ws);
}

void
sendClientMessage(t_Blastpit* self, const char* message)
{
	LOG(kLvlDebug, "sendClientMessage: %s\n", message);
	wsClientSendMessage((t_Websocket*)self->ws, (char*)message);
}

void
sendServerMessage(t_Blastpit* self, const char* message)
{
	wsServerSendMessage((t_Websocket*)self->ws, (char*)message);
}

void
registerCallback(t_Blastpit* self, void (*callback)(void*, void*))
{  // Specify an additional callback upon message receipt

	wsSetMessageReceivedCallback((t_Websocket*)self->ws, callback);
}

void
registerObject(t_Blastpit* self, void* object)
{  // Specify an instance to call the callback method

	wsSetMessageReceivedObject((t_Websocket*)self->ws, object);
}

void
UpdateHighestId(t_Blastpit* self, int id)
{  // Updates the highest known id

	if (id > self->highest_id)
		self->highest_id = id;
}

IdAck
bp_sendMessage(t_Blastpit* self, const char* message)
{
	int id = AutoGenerateId(self);

	UpdateHighestId(self, id);

	char* id_message = xml_setId(id, message);
	if (!id_message) {
		return (IdAck){kInvalid, kSetterFailure, NULL};
	}

	if (((t_Websocket*)self->ws)->isServer) {
		sendServerMessage(self, id_message);
	} else {
		if (!((t_Websocket*)self->ws)->isConnected) {
			return (IdAck){id, kConnectionFailure, NULL};
			// return result;
		}
		sendClientMessage(self, id_message);
	}

	LOG(kLvlDebug, "bp_sendMessage: %s\n", id_message);
	free(id_message);

	return (IdAck){id, kSuccess, NULL};
}

IdAck
bp_sendMessageAndWait(t_Blastpit* self, const char* message, int timeout)
{ /* Sends message and waits. Returns xml of reply with matching id */

	IdAck result;

	result = bp_sendMessage(self, message);
	if (result.retval != kSuccess) {
		return result;
	}

	result = bp_waitForXml(self, result.id, timeout, true);

	return result;
}

IdAck
bp_sendMessageAndWaitForString(t_Blastpit* self, const char* message, int timeout)
{ /* Sends message and waits. Returns xml of reply with matching id */

	IdAck result = bp_sendMessage(self, message);
	if (result.retval != kSuccess) {
		return result;
	}

	return bp_waitForString(self, result.id, timeout);
}

IdAck
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
						if (del) {
							popMessageAt(self, j);
						} else {
							readMessageAt(self, j);
						}
						return (IdAck){id, kSuccess, message};
					}
				}
			}
		}
		usleep(1000);
		if (!(i % 10))
			pollMessages(self);
	}
	return (IdAck){id, kFailure, NULL};
}

IdAck
bp_waitForString(t_Blastpit* self, int id, int timeout)
{
	char* string = xml_getCommandString(bp_waitForXml(self, id, timeout, false).string);
	LOG(kLvlDebug, "bp_waitForString:\n\n%s\n\n", string);

	IdAck result;
	if (string) {
		result = (IdAck){id, kSuccess, string};
	} else {
		result = (IdAck){id, kFailure, BP_EMPTY_STRING};
	};

	return result;
}

IdAck
sendCommand(t_Blastpit* self, int command)
{
	assert(self);

	char message[] = "<command>xxx</command>";

	snprintf(message, sizeof(message) / sizeof(message[0]), "<command>%d</command>", command);
	return bp_sendMessage(self, message);
}

IdAck
SendCommand(t_Blastpit* self, int command)
{
	sds cmd = sdsfromlonglong(command);
	IdAck result = SendMessage(self, "command", cmd, NULL);
	sdsfree(cmd);
	return result;
}

IdAck
SendMessage(t_Blastpit* self, ...)
{  // Sends single command with optional attributes
	// If there are an odd number of optional parameters,
	// the final parameter is used as the message CDATA

	(void)self;
	va_list args;
	va_start(args, self);
	char *attrib, *value;
	sds xml = sdsnew("<?xml version=\"1.0\"?><message ");
	bool first_attribute = true;

	while (true) {
		// Get the attribute name
		attrib = va_arg(args, char*);  // Pop the next argument (a char*)
		if (!attrib)
			break;

		// Get the attribute value
		value = va_arg(args, char*);  // Pop the next argument (a char*)
		if (!value)
			break;

		// Append the attribute to the XML
		if (first_attribute) {
			xml = sdscatprintf(xml, "%s=\"%s\"", attrib, value);
			first_attribute = false;
		} else {
			xml = sdscatprintf(xml, ", %s=\"%s\"", attrib, value);
		}
	}

	xml = sdscatprintf(xml, ">");

	if (attrib) {  // Payload
		xml = sdscatprintf(xml, "%s", attrib);
	}

	xml = sdscatprintf(xml, "</message>");

	printf("xml is: %s\n", xml);
	IdAck result = bp_sendMessage(self, xml);

	va_end(args);
	sdsfree(xml);

	return result;
}

int
AutoGenerateId(t_Blastpit* self)
{  // Generates an id higher than any previous

	self->highest_id++;
	return self->highest_id;
}

IdAck
bp_sendCommandAndWait(t_Blastpit* self, int command, int timeout)
{
	IdAck result;

	result = sendCommand(self, command);
	if (result.retval != kSuccess) {
		LOG(kLvlDebug, "%s: Failed to send command", __func__);
		result = (IdAck){kInvalid, kCommandFailed, NULL};
		return result;
	}
	result = bp_waitForXml(self, result.id, timeout, false);
	if (result.retval != kSuccess) {
		LOG(kLvlDebug, "%s: Timed out waiting for XML", __func__);
		result = (IdAck){kInvalid, kReplyTimeout, NULL};
		return result;
	}

	return result;
}

bool
bp_isConnected(t_Blastpit* self)
{
	return ((t_Websocket*)self->ws)->isConnected;
}

int
bp_waitForSignal(t_Blastpit* self, int signal, int timeout)
{ /* Waits for an Lmos signal */

	int result = xml_getId(bp_waitForXml(self, signal, timeout, true).string);
	return result ? result : false;
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

	return (char*)wsPopMessageAt((t_Websocket*)self->ws, index);
}

int
getMessageCount(t_Blastpit* self)
{  // Get the count of the message stack

	return wsGetMessageCount((t_Websocket*)self->ws);
}

char*
readMessageAt(t_Blastpit* self, int index)
{  // Get the message for a specific index without deleting

	return (char*)wsReadMessageAt((t_Websocket*)self->ws, index);
}

void
startLMOS(t_Blastpit* self)
{  // Send a message to LMOS to unload the ActiveX control

	sendCommand(self, kCreateLMOS);
}

void
stopLMOS(t_Blastpit* self)
{  // Send a message to LMOS to unload the ActiveX control

	sendCommand(self, kDestroyLMOS);
}

void
clearQPSets(t_Blastpit* self)
{  // Tell Lmos to erase all existing QPsets

	sendCommand(self, kClearQpSets);
}

void
LayerSetLaserable(t_Blastpit* self, const char* layer, bool laserable)
{
	sds message = sdscatprintf(sdsempty(),
				   "<command layer=\"%s\" laserable=\"%d\">%d</command>",
				   layer, (int)laserable, kLayerSetLaserable);
	bp_sendMessage(self, message);
	sdsfree(message);
}

void
LayerSetHeight(t_Blastpit* self, const char* layer, int height)
{
	sds message = sdscatprintf(sdsempty(),
				   "<command layer=\"%s\" height=\"%d\">%d</command>",
				   layer, height, kLayerSetHeight);
	bp_sendMessage(self, message);
	sdsfree(message);
}
