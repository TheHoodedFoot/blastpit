#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>

#include <stdio.h>
#include <string.h>
#include "blastpit.h"
#include "klass.h"
#include "new.h"

static void*
Blastpit_constructor(void* _self, va_list* args)
{
	(void)args;

	struct Blastpit* self = _self;

	self->net = NULL;

	return self;
}

static void*
Blastpit_destructor(void* _self)
{
	struct Blastpit* self = _self;

	if (self->net) delete (self->net);

	return self;
}

/* This struct defines the class */
static const struct Klass _Blastpit = {
	sizeof(struct Blastpit), Blastpit_constructor, Blastpit_destructor};

/* This extern variable points to a template passed to new() */
const void* Blastpit = &_Blastpit;

t_Blastpit*
bp_new()
{ /* Return a t_Blastpit instance (callable from C++)
     Needed because the new keyword is reserved in C++ */

	t_Blastpit* bp = new (Blastpit);
	return bp;
}

void
bp_delete(t_Blastpit* bp)
{ /* As above, we need this function to delete an instance from C++ */

	delete (bp);
}

int
bp_connectToServer(t_Blastpit* self, const char* server, const char* id,
		   int timeout_ms)
{ /* Combined connect and subscribe function */

	self->net = new (Mqtt, server, id);
	if (connectToMqttServer(self->net) != MQTTASYNC_SUCCESS) {
		/* disconnectFromServer(self); */
		printf("bp_connectToServer: cannot connect\n");
		return 999;
		return MQTTASYNC_FAILURE;
	}

	for (int i = 0; i < timeout_ms; i++) {
		if (bp_isConnected(self)) {
			/* Always subscribe to broadcast messages */
			bp_subscribe(self, "broadcast", timeout_ms);
			return bp_subscribe(self, id, timeout_ms);
		}
		usleep(1000);
	}

	return MQTTASYNC_FAILURE;
}

int
bp_subscribe(t_Blastpit* self, const char* topic, int timeout_ms)
{
	for (int i = 0;; i++) {
		if (mqtt_subscribe(self->net, topic) == MQTTASYNC_SUCCESS)
			return 0;
		if (i >= timeout_ms) break;
		usleep(1000);
	}

	printf("bp_subscribe: failed\n");
	return MQTTASYNC_FAILURE;
}

int
bp_unsubscribe(t_Blastpit* self, const char* topic, int timeout_ms)
{
	for (int i = 0;; i++) {
		if (mqtt_unsubscribe(self->net, topic) == MQTTASYNC_SUCCESS)
			return 0;
		if (i >= timeout_ms) break;
		usleep(1000);
	}

	printf("bp_subscribe: failed\n");
	return MQTTASYNC_FAILURE;
}

void
bp_disconnectFromServer(t_Blastpit* self)
{
	mqtt_disconnect(self->net);
	delete (self->net);
	self->net = NULL;
}

int
clearQPSets(t_Blastpit* self)
{
	(void)self;

	/* t_Mqtt* net = self->net; */

	return false;
}

void
registerCallback(t_Blastpit* self, void (*callback)(void*))
{
	setReceiveCallback(self->net, callback);
}

int
bp_sendMessage(t_Blastpit* self, const char* topic, const char* message)
{
	t_Mqtt* net = self->net;
	return mqtt_isConnected(net) ? mqtt_sendMessage(net, topic, message)
				     : -1;
}

int
bp_sendMessageAndWait(t_Blastpit* self, int id, const char* topic,
		      const char* message, int timeout)
{
	if (bp_sendMessage(self, topic, message) == -1) return -1;
	return bp_waitForString(self, id, timeout) == NULL;
}

char*
bp_waitForString(t_Blastpit* self, int id, int timeout)
{ /* Return the message data as a string, or NULL */

	for (int i = 0; i < timeout; i++) {
		if (bp_getMessageCount(self)) {
			/* Test every message with getMessageAt() */
			for (int j = 0; j < bp_getMessageCount(self); j++) {
				t_bp_message msg = {.length = 0,
						    .data = NULL};
				t_Message* message =
					getMessageAt(self->net, j);

				if (message) {
					msg.data = message->data;
					msg.length = message->size;
				}

				char num[3];
				snprintf(num, 3, "%d", id);
				if ((strstr(msg.data, num))) {
					return msg.data;
				}
			}
		}
		usleep(1000);
	}

	return NULL;
}

int
bp_sendCommand(t_Blastpit* self, int id, const char* topic, int command)
{
	char message[] = "<command id=xxxxx>xx</command>";

	snprintf(message, sizeof(message) / sizeof(message[0]),
		 "<command id=\"%d\">%d</command>", id, command);
	return bp_sendMessage(self, topic, message);
}

int
bp_sendCommandAndWait(t_Blastpit* self, int id, const char* topic,
		      int command, int timeout)
{
	if (bp_sendCommand(self, id, topic, command) == -1) return 6661;
	return bp_waitForString(self, id, timeout) == NULL;
}

int
bp_getMessageCount(t_Blastpit* self)
{
	t_Mqtt* net = self->net;
	return mqtt_isConnected(net) ? mqtt_getMessageCount(net) : -1;
}

t_bp_message
bp_getNewestMessage(t_Blastpit* self)
{
	assert(self->net);

	t_bp_message msg = {.length = 0, .data = NULL};

	t_Message* message = popMessage(self->net);
	if (message) {
		msg.data = message->data;
		msg.length = message->size;
	}

	return msg;
}

bool
bp_isConnected(t_Blastpit* self)
{
	return mqtt_isConnected(self->net);
}
