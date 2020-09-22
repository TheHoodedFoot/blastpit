#!/usr/bin/env python

import os.path
import sys
import time
import unittest

# Configuration variables
if not os.path.isfile(os.path.expanduser("~")
                      + "/projects/blastpit/res/cfg/myconfig.py"):
    print("The blastpit myconfig.py file is not available.")
    exit(1)
sys.path.append(os.path.expanduser("~")
                + "/projects/blastpit/res/cfg")
import myconfig

if not os.path.isfile(os.path.expanduser("~")
                      + "/projects/blastpit/build/_blastpy.so"):
    print("The blastpit library is not available.")
    exit(1)
sys.path.append(os.path.expanduser("~")
                + "/projects/blastpit/build")

import blastpy as bp


# Helper functions needed due to lack of running event loop
def PollUntilConnected(client, server, timeout=myconfig.WS_TEST_TIMEOUT):
    for i in range(timeout):
        bp.pollMessages(client)
        bp.pollMessages(server)
        if bp.bp_isConnected(client):
            return
        time.sleep(0.001)


def PollUntilMessageCount(client, server, count=1, timeout=myconfig.WS_TEST_TIMEOUT):
    for i in range(timeout):
        bp.pollMessages(client)
        bp.pollMessages(server)
        if bp.getMessageCount(client) == count:
            return
        time.sleep(0.001)


class Testbp(unittest.TestCase):

    def setUp(self):
        self.server = bp.blastpitNew()
        self.assertEqual(bp.kSuccess, bp.serverCreate(self.server, myconfig.WS_PORT))
        print("Creating server")

    def test_MultipleCommands(self):
        self.client1 = bp.blastpitNew()
        self.client2 = bp.blastpitNew()

        # These next lines will give a timeout error because we don't poll
        bp.connectToServer(self.client1, myconfig.WS_SERVER_LOCAL, 0)
        bp.connectToServer(self.client2, myconfig.WS_SERVER_LOCAL, 0)

        PollUntilConnected(self.client1, self.server)
        PollUntilConnected(self.client2, self.server)
        # sys.exit()

        xml = "<?xml?><message id=\"1\" command=\"98\"></message>"
        xml = xml + "<message id=\"2\" command=\"99\"></message>"

        result = bp.bp_sendMessage(self.client1, xml)
        bp.pollMessages(self.client1)
        PollUntilMessageCount(self.client2, self.server, 1, 100)
        print("Message count: %d" % bp.getMessageCount(self.client2))

        bp.disconnectFromServer(self.client1)
        bp.disconnectFromServer(self.client2)
        bp.blastpitDelete(self.client1)
        bp.blastpitDelete(self.client2)

    def test_QueueCommands(self):
        self.client = bp.blastpitNew()
        bp.connectToServer(self.client, myconfig.WS_SERVER_LOCAL, myconfig.WS_TEST_TIMEOUT)
        PollUntilConnected(self.client, self.client)

        bp.BpQueueCommand(self.client, 99)
        bp.BpQueueCommand(self.client, bp.kClearLayout)
        bp.BpQueueCommand(self.client, bp.kClearLayout)
        bp.pollMessages(self.client)

        result = bp.BpUploadQueuedMessages(self.client)
        bp.pollMessages(self.client)

        print(result.id)
        print(result.retval)

        bp.disconnectFromServer(self.client)
        bp.blastpitDelete(self.client)

    def tearDown(self):
        bp.serverDestroy(self.server)
        bp.blastpitDelete(self.server)
        print("Destroying server")


if __name__ == '__main__':
    unittest.main(failfast=True)

# def test_sendReceive(self):
#     self.assertEqual(
#         0,
#         bp.bp_sendMessage(
#             self.blast,
#             88,
#             myconfig.MQTT_ID,
#             "<command>Test</command>"))

#     for i in range(10):
#         if bp.bp_getMessageCount(self.blast) > 0:
#             break
#         time.sleep(0.1)

#     self.assertEqual(1, bp.bp_getMessageCount(self.blast))
#     self.assertEqual(
#         "<?xml version=\"1.0\"?>\n<command id=\"88\">Test</command>\n",
#         bp.bp_waitForXml(
#             self.blast, 88, myconfig.TIMEOUT, False))

# def test_reconnect(self):
#     self.assertEqual(0, bp.bp_getMessageCount(self.blast))
#     bp.bp_sendMessage(
#         self.blast,
#         1,
#         myconfig.MQTT_ID,
#         "<command>You are abcde!</command>")
#     time.sleep(0.5)
#     self.assertEqual(1, bp.bp_getMessageCount(self.blast))
#     bp.bp_getNewestMessage(self.blast)
#     self.assertEqual(0, bp.bp_getMessageCount(self.blast))

# def test_waitForString(self):
#     self.assertEqual(0, bp.bp_getMessageCount(self.blast))
#     bp.bp_sendMessage(
#         self.blast,
#         9,
#         myconfig.MQTT_ID,
#         "<command>You are abcde!</command>")
#     time.sleep(1)
#     self.assertEqual(1, bp.bp_getMessageCount(self.blast))
#     string = bp.bp_waitForString(self.blast, 9, 100)
#     self.assertEqual("You are abcde!", string)

# def test_lmosReplyTimingTest(self):
#     self.assertEqual(0, bp.bp_getMessageCount(self.blast))
#     maxTime = 0
#     startTime = time.time()
#     for i in range(1, 10):
#         localStartTime = time.time()
#         bp.bp_sendCommandAndWait(
#             self.blast, i, "lmos", bp.kGetVersion, 2000)

#         # bp.bp_sendMessage(
#         #     self.blast,
#         #     i,
#         #     myconfig.MQTT_ID,
#         #     "<command>You are abcde!</command>")
#         string = bp.bp_waitForString(self.blast, i, 1000)
#         # self.assertEqual( "You are abcde!", string)
#         localTotalTime = time.time() - localStartTime
#         if localTotalTime > maxTime:
#             maxTime = localTotalTime
#     endTime = time.time()
#     print("Total elapsed time: " + str(endTime - startTime))
#     print("Max individual time: " + str(maxTime))
# assertEqual(a, b) 	a == b
# assertNotEqual(a, b) 	a != b
# assertTrue(x) 	bool(x) is True
# assertFalse(x) 	bool(x) is False
# assertIs(a, b) 	a is b 	3.1
# assertIsNot(a, b) 	a is not b 	3.1
# assertIsNone(x) 	x is None 	3.1
# assertIsNotNone(x) 	x is not None 	3.1
# assertIn(a, b) 	a in b 	3.1
# assertNotIn(a, b) 	a not in b 	3.1
# assertIsInstance(a, b) 	isinstance(a, b) 	3.2
# assertNotIsInstance(a, b) 	not isinstance(a, b) 	3.2
