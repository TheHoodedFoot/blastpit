#include <stdio.h>
#include <unistd.h>

#include "message.h"
#include "mqtt.h"
#include "new.h"
#include "unity_fixture.h"

#include "t_common.h"

TEST_GROUP(MqttGroup);

TEST_SETUP(MqttGroup) {}

TEST_TEAR_DOWN(MqttGroup) {}

TEST(MqttGroup, sendReceiveMessage)
{ /* Send a message and check if we can receive it */

	/* Create and initialise client object */
	t_Mqtt *net = new (Mqtt, SERVER, CLIENTID);

	// Connect to server
	TEST_ASSERT_EQUAL(0, connectToMqttServer(net));

	for (int i = 0; i < 2000; i++) {
		if (mqtt_isConnected(net)) break;
		usleep(1000);
	}

	// Subscribe
	TEST_ASSERT_EQUAL(0, mqtt_subscribe(net, CLIENTID));

	for (int i = 0; i < 20; i++) {
		if (mqtt_isSubscribed(net)) break;
		usleep(1000);
	}

	// Send messages
	for (int j = 1; j < 5; j++) {
		mqtt_sendMessage(net, CLIENTID, "Test message t_mqtt.c");

		// Wait for message or timeout
		for (int i = 0; i < 2000; i++) {
			if (mqtt_getMessageCount(net) == j) break;
			usleep(1000);
		}
	}

	TEST_ASSERT_EQUAL(4, mqtt_getMessageCount(net));

	t_Message *message = popMessage(net);
	TEST_ASSERT_NOT_NULL(message);
	/* TEST_ASSERT_EQUAL(0, */
	/* 		  isMessageEqual(message, "Test message t_mqtt.c"));
	 */
	TEST_ASSERT_EQUAL(3, mqtt_getMessageCount(net));

	mqtt_unsubscribe(net, CLIENTID);
	mqtt_disconnect(net);
	delete (net);
}

TEST(MqttGroup, LinkedListTest)
{ /* Check adding and removing messages to the list */

	/* Message count should be zero */
	t_Mqtt *net = new (Mqtt, SERVER, CLIENTID);
	t_Message *message = new (Message, 5, "First");

	TEST_ASSERT_EQUAL(0, getCount(net->messageFifo));
	pushMessage(net, message);
	TEST_ASSERT_EQUAL(1, getCount(net->messageFifo));
	message = new (Message, 6, "Second");
	pushMessage(net, message);
	TEST_ASSERT_EQUAL(2, getCount(net->messageFifo));
	popMessage(net);
	TEST_ASSERT_EQUAL(1, getCount(net->messageFifo));
	message = popMessage(net);
	TEST_ASSERT_EQUAL(0, getCount(net->messageFifo));
}

TEST(MqttGroup, RandomAccessTest)
{
	/* What are the requirements to test 'x'? */
	/* 	What does the object do? */
	/* 	How does it interact with the data or hardware it controls? */
	/* 	How can we make it testable? */

	TEST_IGNORE();
}

TEST_GROUP_RUNNER(MqttGroup)
{
	RUN_TEST_CASE(MqttGroup, sendReceiveMessage);
	RUN_TEST_CASE(MqttGroup, LinkedListTest);
	RUN_TEST_CASE(MqttGroup, RandomAccessTest);
}

static void
runAllTests()
{
	RUN_TEST_GROUP(MqttGroup);
}

int
main(int argc, const char *argv[])
{
	printf("\nRunning %s...\n", argv[0]);
	return UnityMain(argc, argv, runAllTests);
}

// https://www.eclipse.org/paho/files/mqttdoc/MQTTAsync/html/index.html

// Applications that use the client library typically use a similar structure:

//     Create a client object
//     Set the options to connect to an MQTT server
//     Set up callback functions
//     Connect the client to an MQTT server
//     Subscribe to any topics the client needs to receive
//     Repeat until finished:
//         Publish any messages the client needs to
//         Handle any incoming messages
//     Disconnect the client
//     Free any memory being used by the client
