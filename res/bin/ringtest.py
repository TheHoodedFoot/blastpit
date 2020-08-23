#!/usr/bin/env python3

from blastpy import *
import sys
import time
import colorsys
from os.path import expanduser
sys.path.append(expanduser("~") + "/projects/blastpit/src/libbp")

self = t_Blastpit()

LIVE = True
SERVER = "192.168.1.40"

SHORT_DELAY = 8000
LONG_DELAY = 10000

bp_connectToServer(self, SERVER, "jupyter", LONG_DELAY, True)

id = 400
if LIVE:
    bp_sendCommandAndWait(self, id, "lmos", kInitMachine, SHORT_DELAY)

    id = id + 1
    print("Cancelling job...")
    bp_sendCommandAndWait(self, id, "lmos", kCancelJob, LONG_DELAY)


numSegments = 10
radius = 10
width = 5
convex = 1
shadowWidth = (math.pi * radius * 2) / numSegments

for shadow in range(numSegments):

    xml = BpXML()
    xml.setCylinder(radius, width, 360 / numSegments, convex)

    geo = BpPolyline(
        "triangle{:02d}".format(shadow),
        (shadowWidth * shadow) + 2,
        58,
        "Standard",
        "Standard")
    geo.line((shadowWidth * shadow) + 4, 62)
    geo.line(shadowWidth * shadow, 62)
    geo.close()
    xml.addPolyline(geo)

    # upload
    id = id + 1
    xml.setCommand(kImportXML, id)
    print("Uploading XML...")
    bp_sendMessageAndWait(self, id, "lmos", str(xml.xml()), SHORT_DELAY)

    id = id + 1
    print("Setting layer height...")
    bp_sendMessageAndWait(
        self,
        id,
        "lmos",
        "<command layer=\"RofinStandard\" height=\"120\">" +
        str(kLayerSetHeight) +
        "</command>",
        SHORT_DELAY)

    id = id + 1
    print("Setting shadow width...")
    bp_sendMessageAndWait(
        self,
        id,
        "lmos",
        "<command object=\"GalvoGroupShadow101\" x=\"" +
        str(
            round(
                shadow *
                shadowWidth,
                3)) +
        "\" y=\"0\" r=\"0\">" +
        str(kSetPosValues) +
        "</command>",
        LONG_DELAY)

    if not LIVE:
        continue

    # Set the shadow properties
    id = id + 1
    print("Setting shadow position...")
    bp_sendMessageAndWait(
        self,
        id,
        "lmos",
        "<command object=\"GalvoGroupShadow101\" x=\"" +
        str(shadowWidth) +
        "\" y=\"120\">" +
        str(kSetDimension) +
        "</command>",
        LONG_DELAY)

    # Move and resize shadow
    id = id + 1
    print("Loading job...")
    bp_sendCommandAndWait(self, id, "lmos", kLoadJob, SHORT_DELAY)

    id = id + 1
    print("Starting marking...")
    bp_sendCommandAndWait(self, id, "lmos", kStartMarking, SHORT_DELAY)

    if bp_waitForSignal(self, kIdImageEnd, LONG_DELAY) == kIdImageEnd:
        # not waiting the ten seconds
        print("Got ImageEnd signal")
    else:
        print("Failed to receive ImageEnd signal")
        break

    id = id + 1
    print("Cancelling job...")
    bp_sendCommandAndWait(self, id, "lmos", kCancelJob, LONG_DELAY)

    # print("Sleeping...")
    # time.sleep(1)

if LIVE:
    id = id + 1
    print("Terminating machine...")
    bp_sendCommandAndWait(self, id, "lmos", kTermMachine, SHORT_DELAY)

bp_disconnectFromServer(self)
