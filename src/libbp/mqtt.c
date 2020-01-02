#include <assert.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "MQTTAsync.h"
#include "klass.h"
#include "linkedlist.h"
#include "message.h"
#include "mqtt.h"
#include "new.h"

static void*
Mqtt_constructor(void* _self, va_list* app)
{
	struct Mqtt* self = _self;

	self->connected = false;
	self->subscribed = false;

	const char* server = va_arg(*app, const char*);
	self->server = malloc(strlen(server) + 1);
	assert(self->server);
	strcpy((void*)self->server, server);

	const char* id = va_arg(*app, const char*);
	self->id = malloc(strlen(id) + 1);
	assert(self->id);
	strcpy((void*)self->id, id);

	/* Not needed because template is static and therefore
	 * initialized to zero */

	/* self->messageFifo = NULL; */
	/* self->messageReceived = NULL; */

	return self;
}

static void*
Mqtt_destructor(void* _self)
{
	struct Mqtt* self = _self;

	ll_deleteFamily(self->messageFifo);
	/* delete(self->messageFifo); */

	free((void*)self->server), self->server = 0;
	free((void*)self->id), self->id = 0;

	/* fprintf(stderr, "Destructing mqtt %p\n", (void*)self); */
	return self;
}

/* This struct acts as the class definition */
static const struct Klass _Mqtt = {sizeof(struct Mqtt), Mqtt_constructor,
				   Mqtt_destructor};

/* This extern variable is a template of the class defined above,
 * which is cloned by new() into an indepenent instance */
const void* Mqtt = &_Mqtt;


/**********************************************************************
 *                              Methods                               *
 **********************************************************************/

const char*
getServer(t_Mqtt* mq)
{
	return mq->server;
}

const char*
getId(t_Mqtt* mq)
{
	return mq->id;
}

void
connlost(void* context, char* cause)
{
	t_Mqtt* mq = (t_Mqtt*)context;
	mq->connected = false;

	MQTTAsync client = (MQTTAsync)context;
	MQTTAsync_connectOptions conn_opts =
		MQTTAsync_connectOptions_initializer;
	int rc;
	printf("\nConnection lost\n");
	printf("     cause: %s\n", cause);
	printf("Reconnecting\n");
	conn_opts.keepAliveInterval = 20;
	conn_opts.cleansession = 1;
	if ((rc = MQTTAsync_connect(client, &conn_opts)) !=
	    MQTTASYNC_SUCCESS) {
		printf("Failed to start connect, return code %d\n", rc);
	}
	printf("End of connlost\n");
	connectToMqttServer(mq);
	usleep(1000000);
}

void
onDisconnect(void* context, MQTTAsync_successData* response)
{
	(void)response;

	printf("Successful disconnection\n");
	t_Mqtt* mq = (t_Mqtt*)context;
	mq->connected = false;
}

void
onSubscribe(void* context, MQTTAsync_successData* response)
{
	(void)response;

	/* printf("Subscribe succeeded\n"); */
	t_Mqtt* mq = (t_Mqtt*)context;
	mq->subscribed = true;
}

void
onSubscribeFailure(void* context, MQTTAsync_failureData* response)
{
	printf("Subscribe failed, rc %d\n", response ? response->code : 0);
	t_Mqtt* mq = (t_Mqtt*)context;
	mq->subscribed = false;
}

void
onConnect(void* context, MQTTAsync_successData* response)
{
	(void)response;

	/* printf("Connect succeeded\n"); */
	t_Mqtt* mq = (t_Mqtt*)context;
	mq->connected = true;
}

void
onConnectFailure(void* context, MQTTAsync_failureData* response)
{
	printf("Connect failed, rc %d\n", response ? response->code : 0);
	t_Mqtt* mq = (t_Mqtt*)context;
	mq->connected = false;
}

int
onMessageArrived(void* context, char* topicName, int topicLen,
		 MQTTAsync_message* message)
{ /* Called when a message arrives */

	(void)topicLen;

	t_Mqtt* mq = (t_Mqtt*)context;

	t_Message* msg = new (Message, message->payloadlen, message->payload);
	pushMessage(mq, msg);

	/* Free the message */
	MQTTAsync_freeMessage(&message);
	MQTTAsync_free(topicName);

	/* Call our user callback (if registered) */
	if (mq->messageReceived) mq->messageReceived(context);

	/* Return true to indicate message has been processed here.
	 * Returning false will cause a retry attempt, which is dangerous
	 * since the message and topic were freed above. */
	return true;
}

void
pushMessage(t_Mqtt* self, t_Message* message)
{ /* Add a message to the back of the fifo */

	t_LinkedList* lastMessage = getLast(self->messageFifo);

	t_LinkedList* newListEntry = new (LinkedList);
	setItem(newListEntry, (void*)message);
	if (lastMessage) {
		lastMessage->next = newListEntry;
	} else {
		/* Fifo is empty */
		self->messageFifo = newListEntry;
	}
}

t_Message*
popMessage(t_Mqtt* self)
{ /* Remove the newest message from the fifo */

	t_LinkedList* lastMessage = getLast(self->messageFifo);

	if (!lastMessage) return NULL;

	t_Message* result = (t_Message*)getItem(lastMessage);
	self->messageFifo = removeEntry(self->messageFifo, lastMessage);

	return result;
}

t_Message*
getMessageAt(t_Mqtt* self, int index)
{ /* Get pointer to message at <index> or NULL */

	t_LinkedList* fifo = self->messageFifo;

	for (int i = 0; (i < index) && fifo; i++) fifo = fifo->next;

	if (fifo) return (t_Message*)fifo->item;
	return NULL;
}

bool
deleteMessageAt(t_Mqtt* self, int index)
{ /* Get pointer to message at <index> or NULL */

	t_LinkedList* fifo = self->messageFifo;

	for (int i = 0; (i < index) && fifo; i++) fifo = fifo->next;

	if (fifo) {
		removeEntry(self->messageFifo, fifo);
		return true;
	}

	return false;
}


void
setNewestMessage(t_Mqtt* self, t_Message* msg)
{
	(void)msg;

	abort();

	t_LinkedList* list = self->messageFifo;
	(void)list; /* TODO: fix this */
}

int
connectToMqttServer(t_Mqtt* self)
{
	MQTTAsync_connectOptions conn_opts =
		MQTTAsync_connectOptions_initializer;

	MQTTAsync_create(&self->client, getServer(self), self->id,
			 MQTTCLIENT_PERSISTENCE_NONE, NULL);

	/* We store the 'self' pointer in the context parameter,
	 * which is passed as a parameter to the callback function,
	 * so that we know which instance this refers to */
	if (MQTTAsync_setCallbacks(self->client, (void*)self, connlost,
				   onMessageArrived,
				   NULL) == MQTTASYNC_FAILURE) {
		printf("Setting callbacks failed\n");
		return 0;
	}

	conn_opts.keepAliveInterval = 30;
	conn_opts.cleansession = 1;
	conn_opts.onSuccess = onConnect;
	conn_opts.onFailure = onConnectFailure;
	conn_opts.context = self;

	/* Let the caller deal with the error code */

	/* Note: This returns before the connection is fully established,
	 * so attempting to use it immediately upon return will fail */
	return MQTTAsync_connect(self->client, &conn_opts);
}

int
mqtt_subscribe(t_Mqtt* self, const char* topic)
{
	if (!mqtt_isConnected(self)) return MQTTASYNC_FAILURE;

	MQTTAsync_responseOptions opts =
		MQTTAsync_responseOptions_initializer;
	opts.onSuccess = onSubscribe;
	opts.onFailure = onSubscribeFailure;
	opts.context = self;

	/* Let the caller deal with the error code */
	return MQTTAsync_subscribe(self->client, topic, QOS, &opts);
}

int
mqtt_unsubscribe(t_Mqtt* self, const char* topic)
{
	MQTTAsync_responseOptions opts =
		MQTTAsync_responseOptions_initializer;

	/* Let the caller deal with the error code */
	return MQTTAsync_unsubscribe(self->client, topic, &opts);
}

int
mqtt_sendMessage(t_Mqtt* self, const char* topic, const char* message)
{
	MQTTAsync_responseOptions opts =
		MQTTAsync_responseOptions_initializer;
	opts.onSuccess = NULL;
	opts.context = self->client;

	MQTTAsync_message pubmsg = MQTTAsync_message_initializer;
	pubmsg.payload = (void*)message;
	pubmsg.payloadlen = strlen(message) + 1;
	pubmsg.qos = QOS;
	pubmsg.retained = 0;

	return MQTTAsync_sendMessage(self->client, topic, &pubmsg, &opts);
}

bool
mqtt_isConnected(t_Mqtt* self)
{
	return self->connected;
}

bool
mqtt_isSubscribed(t_Mqtt* self)
{
	return self->subscribed;
}

int
mqtt_getMessageCount(t_Mqtt* self)
{
	return getCount(self->messageFifo);
}

void
setReceiveCallback(t_Mqtt* self, void (*callback)(void*))
{
	self->messageReceived = callback;
}

int
mqtt_disconnect(t_Mqtt* self)
{
	MQTTAsync_disconnectOptions disc_opts = {{'M', 'Q', 'T', 'D'},
						 1,
						 0,
						 NULL,
						 NULL,
						 NULL,
						 MQTTProperties_initializer,
						 MQTTREASONCODE_SUCCESS,
						 NULL,
						 NULL};
	return MQTTAsync_disconnect(self->client, &disc_opts);
}
