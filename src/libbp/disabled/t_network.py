#!/usr/bin/env python3

import os.path
if not os.path.isfile(os.path.expanduser("~") +
                      "/usr/src/blastpit/src/libblastpit/_blastpit.so"):
    print("The blastpit library is not available.")
    exit()

import sys
sys.path.append(os.path.expanduser("~") +
                "/usr/src/blastpit/src/libblastpit")

import blastpit
import unittest


class TestSequenceFunctions(unittest.TestCase):

    def setUp(self):
        print()
        # self.bp = blastpit.Blastpit()

    def test_network(self):
        print()
        # print("running test_network...")
        # self.bp.Connect("localhost", 1234)
        # self.bp.Disconnect()
        # self.bp.Connect("localhost")
        # self.bp.SendCommand(0)
        # self.bp.SendPacket(0, "Packet")
        # print(self.bp.ReceivePacket())
        # print("ending test_network...")

    def tearDown(self):
        print()
        # self.bp.Disconnect()


if __name__ == '__main__':
    unittest.main(verbosity=2)

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
