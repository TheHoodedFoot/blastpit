#ifndef MQTT_H
#define MQTT_H

#include <stdbool.h>
#include "MQTTAsync.h"
#include "linkedlist.h"
#include "message.h"

#define QOS 2
#define TIMEOUT 10000L

/* extern pointer to our class template */
extern const void *Mqtt;

typedef struct Mqtt {    /* mqtt object struct */
	const void *cls; /* must be first */
	MQTTAsync client;
	char *server;
	char *id;
	bool connected;
	bool subscribed;
	void (*messageReceived)(void *);
	t_LinkedList *messageFifo;
} t_Mqtt;

bool mqtt_isConnected(t_Mqtt *);
bool mqtt_isSubscribed(t_Mqtt *);
const char *getServer(t_Mqtt *);
int connectToMqttServer(t_Mqtt *);
int mqtt_disconnect(t_Mqtt *);
int mqtt_subscribe(t_Mqtt *, const char *);
int mqtt_unsubscribe(t_Mqtt *self, const char *topic);
void setReceiveCallback(t_Mqtt *, void (*)(void *));
t_Message *popMessage(t_Mqtt *);
t_Message *getMessageAt(t_Mqtt *, int index);
void pushMessage(t_Mqtt *, t_Message *);
int mqtt_sendMessage(t_Mqtt *, const char *, const char *);
void setNewestMessage(t_Mqtt *, t_Message *);
/* t_Message *getOldestMessage(t_Mqtt *); */
int mqtt_getMessageCount(t_Mqtt *);

#endif
