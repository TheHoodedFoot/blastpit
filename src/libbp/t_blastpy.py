#!/usr/bin/env python

import os.path
if not os.path.isfile(os.path.expanduser("~")
                      + "/projects/blastpit/src/libbp/_blastpy.so"):
    print("The blastpit library is not available.")
    exit(1)

import sys
sys.path.append(os.path.expanduser("~")
                + "/projects/blastpit/src/libbp")

import blastpy as bp
import unittest
import socket
import time

SERVER = "tcp://rfbevan.co.uk:1883"
# SERVER = "tcp://hilly:1883"
MQTT_ID = socket.gethostname()
TIMEOUT = 2000


class TestBlastpy(unittest.TestCase):

    def setUp(self):
        self.blast = bp.t_Blastpit()
        self.assertEqual(0,
                         bp.bp_connectToServer(
                             self.blast,
                             SERVER,
                             MQTT_ID,
                             TIMEOUT))
        self.assertEqual(0, bp.bp_subscribe(self.blast, MQTT_ID, TIMEOUT))

    def test_sendReceive(self):
        self.assertEqual(
            0,
            bp.bp_sendMessage(
                self.blast,
                MQTT_ID,
                "Test message t_blastpy.py"))

        for i in range(10):
            if bp.bp_getMessageCount(self.blast) > 0:
                break
            time.sleep(0.1)

        self.assertEqual(1, bp.bp_getMessageCount(self.blast))
        self.assertEqual(
            "Test message t_blastpy.py",
            bp.bp_getNewestMessage(
                self.blast).data)

    def test_reconnect(self):
        self.assertEqual(0, bp.bp_getMessageCount(self.blast))
        bp.bp_sendMessage(self.blast, MQTT_ID, "You are abcde!")
        time.sleep(0.5)
        self.assertEqual(1, bp.bp_getMessageCount(self.blast))
        bp.bp_getNewestMessage(self.blast)
        self.assertEqual(0, bp.bp_getMessageCount(self.blast))

    def test_waitForString(self):
        self.assertEqual(0, bp.bp_getMessageCount(self.blast))
        string = bp.bp_waitForString(self.blast, 0, 5000)
        if string is not None:
            self.assertEqual(34, len(string))

    def tearDown(self):
        bp.bp_unsubscribe(self.blast, MQTT_ID, TIMEOUT)
        bp.bp_disconnectFromServer(self.blast)


if __name__ == '__main__':
    unittest.main()

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
