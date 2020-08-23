#!/usr/bin/env python2

import base64
import time
import blastpy
import sys
import os.path
if not os.path.isfile(os.path.expanduser("~")
                      + "/usr/src/blastpit/src/libbp/_blastpy.so"):
    print("The blastpit library is not available.")
    exit(1)

sys.path.append(os.path.expanduser("~")
                + "/usr/src/blastpit/src/libbp")


bp = blastpy.t_Blastpit()
# print("Connecting to server...")
blastpy.bp_connectToServer(bp, "hilly", "jupyter", 4000)
# print("Requesting image...")
blastpy.bp_sendCommand(bp, 1, "lmos", blastpy.BpCommand.kGetPng)
# print("Receiving image...")
print(base64.b64decode(blastpy.bp_waitForString(bp, 1, 5000)))
# print("Disconnecting...")
blastpy.bp_disconnectFromServer(bp)
