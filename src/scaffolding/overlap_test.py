#!/usr/bin/env python

# overlap_test.py:

import sys

# import xml.dom.minidom
import xml.etree.ElementTree as ET

# Needed until blastpy is installed system-wide
from os.path import expanduser

sys.path.append(expanduser("~") + "/projects/blastpit/build")
import blastpy

# Get our machine-specific constants
sys.path.append(expanduser("~") + "/projects/blastpit/res/cfg")
import myconfig

LIVE = False

blast = blastpy.blastpitNew()
result = blastpy.connectToServer(
    blast, myconfig.WS_SERVER_LOCAL, myconfig.WS_TIMEOUT_SHORT
)

if result != blastpy.kSuccess:
    print("Failed to connect to server.")
    sys.exit(1)

if LIVE:
    blastpy.bp_sendCommand(id, "lmos", blastpy.kInitMachine, 2000)

# Upload Qp Sets
print("Uploading Qp Sets…")

blastpy.BpQueueCommand(blast, blastpy.kClearQpSets)
blastpy.BpUploadQueuedMessages(blast)

for i in range(1, 11):
    print(i)
    blastpy.BpQueueQpSet(blast, str(i), i, 500, 20000)
blastpy.BpSaveQpSets(blast)

blastpy.BpUploadQueuedMessages(blast)
result = blastpy.BpWaitForReplyOrTimeout(
    blast, result.id, myconfig.WS_TIMEOUT_LONG
)
if result.retval != blastpy.kSuccess:
    print("Uploading Qp Sets Failed", file=sys.stderr)
    print("Retval:", result.retval)

# Get the list of objects
print("Obtaining objects...", file=sys.stderr)

result = blastpy.BpQueueCommand(blast, blastpy.kGetGeoList)
blastpy.BpUploadQueuedMessages(blast)
print("waiting", file=sys.stderr)
print(result.id, file=sys.stderr)
result = blastpy.BpWaitForReplyOrTimeout(
    blast, result.id, myconfig.WS_TIMEOUT_LONG
)
print("waited", file=sys.stderr)
if result.retval == blastpy.kSuccess:
    print("kGetGeoList result:", file=sys.stderr)
    marking_objects = ET.fromstring(result.string)
else:
    print("kGetGeoList failed", file=sys.stderr)
    sys.exit()

blastpy.disconnectFromServer(blast)
blastpy.blastpitDelete(blast)


# vim: tabstop=8 expandtab shiftwidth=4 softtabstop=4
