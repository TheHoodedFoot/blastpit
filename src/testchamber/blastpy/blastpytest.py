#!/usr/bin/env python3

import sys
import os.path
if not os.path.isfile(os.path.expanduser("~") +
                      "/usr/src/blastpit/build/libblastpit/libblastpit.so"):
    print("The blastpit library is not available.")
    exit()
sys.path.append(os.path.expanduser("~") +
                "/usr/src/blastpit/build/libblastpit")
import blastpit

# Tests require a server to be running

import unittest

import os.path
# import math


class TestSequenceFunctions(unittest.TestCase):

    def setUp(self):
        self.bp = blastpit.Blastpit()

    # def test_layers(self):
    #     path = blastpit.Path()
    #     path.Circle(60, 60, 10)
    #     layer = blastpit.Layer("Test Layer", 10, 0x123456, 0xab, False)
    #     layer.AddPath(path)
    #     self.assertEqual(layer.GetName(), "Test Layer")
    #     self.assertEqual(layer.GetHeight(), 10)
    #     self.assertEqual(layer.GetColour(), 0x123456)
    #     self.assertEqual(layer.GetLinetype(), 0xab)
    #     self.assertEqual(layer.GetIsLaserable(), False)
    #     # self.assertEqual( layer.PathCount(), 1 )
    #     self.bp.AddLayer(layer)
    #     # self.assertEqual( self.bp.LayerCount(), 1 )

    # def test_polyline_create(self):
        # Create and verify some polylines

    # def test_rotate(self):
    #     # Rotate elements around pivot
    #     path = blastpit.Path()
    #     path.Line(0, 0, 1, 0)
    #     path.Rotate(0, 0, math.pi / 4)
    #     # self.assertEqual( path.EndX, 0.707 )
    #     # self.assertEqual( path.EndY, 0.707 )

    def test_network(self):
        print("running test_network...")
        self.bp.Connect("localhost", 1234)
        self.bp.Disconnect()
        self.bp.Connect("localhost")
        self.bp.SendCommand(0)
        self.bp.SendPacket(0, "Packet")
        print(self.bp.ReceivePacket())
        print("ending test_network...")

    # def test_circle(self):
    #     path = blastpit.Path()
    #     path.Circle(60, 60, 10)
    #     path.SetHatchType("Argent")
    #     path.SetMarkingType("Silver_With_Dark_Cleaning_Pass")
    #     layer = blastpit.Layer("Test Layer", 10, 0xFF7700)
    #     layer.AddPath(path)
    #     self.bp.AddLayer(layer)
    #     if self.bp.Connect("laser.rfbevan.co.uk", 1234):
    #         self.bp.Burn()
    #         self.bp.OpenDoor()

    def tearDown(self):
        self.bp.Disconnect()


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
