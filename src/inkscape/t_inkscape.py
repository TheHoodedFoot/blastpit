#!/usr/bin/env python3

import sys
from os.path import expanduser
sys.path.append(expanduser("~") + "/projects/blastpit/build")
import blastpy
import time
import random


def GenerateXml(num_points):

    xml = '<DRAWING UNIT="MM"><ROOT HEIGHT="120.0" ID="Blastpit" WIDTH="120.0">'
    xml = xml + '<LAYER NAME="bp_000000" COLOR="0,0,0" HEIGHT_Z_AXIS="119.0" />'
    xml = xml + '<POLYLINE>'

    for point in range(num_points):
        xml = xml + '<POLYPOINT TYPE="LINE">'
        xml = xml + str(round(random.uniform(0,120),3)) + ' ' + str(round(random.uniform(0,120),3))
        xml = xml + '</POLYPOINT>'

    xml= xml + '</POLYLINE>'
    xml = xml + '</ROOT></DRAWING>'
    return xml


def Poll(blastpit):
    for i in range(10):
        blastpy.pollMessages(blastpit)


qpsets = [['bp_000000', 50.0, 500.0, 60000]]
layers = []

# SERVER = "ws://127.0.0.1:8000"
SERVER = "ws://10.47.1.30:8000"
SHORT_TIMEOUT = 5000

blast = blastpy.blastpitNew()
result = blastpy.connectToServer( blast, SERVER, SHORT_TIMEOUT)
if result != blastpy.kSuccess:
    print("Can't connect to server (%d)" % result, file=sys.stderr)
    sys.exit()

blastpy.BpQueueCommand(blast, blastpy.kClearLog)
blastpy.BpQueueCommand(blast, blastpy.kClearQpSets)
for qpset in qpsets:
    blastpy.BpQueueQpSet(blast, qpset[0], int(qpset[1]), int(qpset[2]), int(qpset[3]))
blastpy.BpUploadQueuedMessages(blast)
Poll(blast)

for i in range(100):
    id = blastpy.BpQueueCommandArgs(blast, blastpy.kImportXML, GenerateXml(10000), None, None, None, None, None, None, None, None)
    blastpy.BpUploadQueuedMessages(blast)
    blastpy.BpWaitForReplyOrTimeout(blast, id, 10000)

# print(result.id, result.retval, result.string)
blastpy.disconnectFromServer(blast)


    # result = blastpy.BpUploadQueuedMessages(blast)
    # result = BpWaitForReplyOrTimeout(blast, result.id, 10000)

    # blastpy.BpQueueCommandArgs(blast, blastpy.kLayerSetLaserable, "layer", "RofinStandard", "laserable", "0", None, None, None, None, None)
    # blastpy.BpQueueCommandArgs(blast, blastpy.kLayerSetHeight, "layer", "RofinStandard", "height", "120", None, None, None, None, None)
    # blastpy.BpQueueCommandArgs(blast, blastpy.kLayerSetHeight, "layer", "RofinBackground", "height", "120", None, None, None, None, None)
    # for layer in layers:
    #     blastpy.BpQueueCommandArgs(blast, blastpy.kLayerSetHeight, "layer", str(layer[0]), "height", str(layer[1]), None, None, None, None, None)
    # result = blastpy.BpUploadQueuedMessages(blast)
    # result = BpWaitForReplyOrTimeout(blast, result.id, 10000)

# time.sleep(1)

# if result.retval != blastpy.kSuccess:
#     print("Cannot add Qpsets", file=sys.stderr)
#     print("num messages: %d" % blastpy.getMessageCount(blast), file=sys.stderr)
#     print("Error: %s" % blastpy.bpRetvalName(result.retval), file=sys.stderr)
#     sys.exit()


# if result.retval != blastpy.kSuccess:
#     print("Can't send the drawing XML", file=sys.stderr)
#     print("Error: %s" % blastpy.bpRetvalName(result.retval), file=sys.stderr)
#     sys.exit()

# result = blastpy.BpUploadQueuedMessages(blast)



# for i in range(1000):
#     Poll(blast)
#     time.sleep(0.1)
# blastpy.disconnectFromServer(blast)
