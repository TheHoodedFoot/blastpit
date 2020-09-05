#!/usr/bin/env python

from time import sleep
import sys
import os.path
if not os.path.isfile(os.path.expanduser("~")
                      + "/projects/blastpit/build/_blastpy.so"):
    print("The blastpit library is not available.")
    exit(1)
sys.path.append(os.path.expanduser("~")
                + "/projects/blastpit/build")
import blastpy

SERVER = "ws://minnie:8000"

#print(dir(blastpy))

bp = blastpy.blastpitNew()
result = blastpy.connectToServer(bp, SERVER, 2000)
print("connectToServer: %s" % blastpy.bpRetvalName(result))

# for i in range(20):
#         pollMessages(bp)
#         sleep(0.1)

if not blastpy.bp_isConnected(bp):
    print("Could not connect")
    sys.exit()

# blastpy.startLMOS(bp)

print("Sending selftest command")
result = blastpy.bp_sendCommandAndWait(bp, blastpy.kSelfTest, 5000)
if result.retval != blastpy.kSuccess:
    print("Cannot run selftest")
    sys.exit()

print("Turning on light")
# for i in range(5):
#     blastpy.pollMessages(bp)
#     print("Message count: %d" % blastpy.getMessageCount(bp))
#     if blastpy.getMessageCount(bp) == 1:
#         break
#     sleep(1)

# print(blastpy.popMessage(bp))

# blastpy.stopLMOS(bp)

blastpy.disconnectFromServer(bp)