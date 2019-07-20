#!/usr/bin/env python2

import os.path
if not os.path.isfile(os.path.expanduser("~")
                      + "/usr/src/blastpit/src/libbp/_blastpy.so"):
    print("The blastpit library is not available.")
    exit(1)

import sys
sys.path.append(os.path.expanduser("~")
                + "/usr/src/blastpit/src/libbp")

import blastpy
import time

bp = blastpy.t_Blastpit()
print("Connecting to server...")
blastpy.bp_connectToServer(bp, "rfbevan.co.uk", "jupyter", 4000)
print("Clearing QPsets...")
blastpy.bp_sendCommandAndWait(bp, 1, "lmos", blastpy.BpCommand.kClearQpSets, 2000)
print("Uploading QPsets...")
blastpy.bp_sendMessageAndWait(bp, 2, "lmos", "<command id=\"2\"><qpset name=\"bp_73d216\" current=\"10.0\" speed=\"500\" frequency=\"30000\"/>2</command>", 3000)
print("Uploading geometry...")
blastpy.bp_sendMessageAndWait(bp, 3, "lmos", "<command id=\"3\">6<DRAWING UNIT=\"MM\"><ROOT HEIGHT=\"120.0\" ID=\"Blastpit\" WIDTH=\"120.0\"><LAYER NAME=\"bp_73d216\" COLOR=\"115,210,220\" HEIGHT_Z_AXIS=\"120.0\" /><GROUP HATCH=\"Y\" HP=\"bp_0_01\" ID=\"gpath820\" LAYER=\"bp_73d216\" LP=\"bp_73d216\" REF_POINT=\"CC\" USE_BOX=\"Y\"><POLYLINE HATCH=\"Y\" HP=\"Standard\" ID=\"path820\" LAYER=\"bp_73d216\" LP=\"bp_73d216\"><POLYPOINT TYPE=\"LINE\">39.119869 83.990284</POLYPOINT><POLYPOINT TYPE=\"BEZIER\">36.123108 79.994602</POLYPOINT><POLYPOINT TYPE=\"BEZIER\">33.12635 75.99892</POLYPOINT><POLYPOINT TYPE=\"LINE\">30.129591 72.003239</POLYPOINT><POLYPOINT TYPE=\"BEZIER\">35.934128 70.869331</POLYPOINT><POLYPOINT TYPE=\"BEZIER\">41.738663 69.73542</POLYPOINT><POLYPOINT TYPE=\"LINE\">47.543198 68.601513</POLYPOINT><POLYPOINT TYPE=\"BEZIER\">44.73542 73.731102</POLYPOINT><POLYPOINT TYPE=\"BEZIER\">41.927647 78.860695</POLYPOINT><POLYPOINT TYPE=\"LINE\">39.119869 83.990284</POLYPOINT><POLYPOINT TYPE=\"LINE\">39.119869 83.990284</POLYPOINT></POLYLINE></GROUP></ROOT></DRAWING></command>", 5000)
print("Setting layer exportable...")
blastpy.bp_sendMessageAndWait(bp, 4, "lmos", "<command id=\"4\" layer=bp_73d216\" exportable=\"1\">" + str(blastpy.BpCommand.kLayerSetExportable) + "</command>", 3000)
print("Saving...")
blastpy.bp_sendMessageAndWait(bp, 5, "lmos", "<command id=\"5\" filename=\"C:\Rofin\VisualLaserMarker\MarkingFiles\test_vlm_save.VLM\">18</command>", 3000)
print("Disconnecting...")
blastpy.bp_disconnectFromServer(bp)
