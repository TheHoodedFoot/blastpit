#!/usr/bin/env python3

from blastpy import *
import sys
import os.path
if not os.path.isfile(os.path.expanduser("~")
                      + "/projects/blastpit/src/libbp/_blastpy.so"):
    print("The blastpit library is not available.")
    exit(1)

sys.path.append(os.path.expanduser("~")
                + "/projects/blastpit/src/libbp")


self = t_Blastpit()
xml = BpXML()
id = 1
LIVE = False
SERVER = "rfbevan.co.uk"

bp_connectToServer(self, SERVER, "jupyter", 1000, True)

bp_sendCommand(self, id, "lmos", kSelfTest)

if LIVE:
    id = id + 1

    bp_sendCommandAndWait(self, id, "lmos", kInitMachine, 2000)
xml.addLayer("mylayer", 4, 240, 100, 50)

xml.addLayer("anotherlayer", 5, 40, 220, 150)

geo = BpPolyline("triangle01", 60, 65, "Standard", "Standard")
geo.line(65, 55)
geo.line(55, 55)
geo.close()
geo.setLayer("mylayer")
xml.addPolyline(geo)
geo = BpPolyline("triangle02", 70, 55, "Standard", "Standard")
geo.line(75, 65)
geo.line(65, 65)
geo.close()
geo.setLayer("anotherlayer")
xml.addPolyline(geo)
id = id + 1

xml.setCommand(kImportXML, id)

bp_sendMessageAndWait(self, id, "lmos", str(xml.xml()), 1000)

id = id + 1
bp_sendMessageAndWait(
    self,
    id,
    "lmos",
    "<command id=\"" +
    str(id) +
    "\" layer=\"RofinStandard\" height=\"120\">" +
    str(kLayerSetHeight) +
    "</command>",
    1000)

id = id + 1
bp_sendMessageAndWait(
    self,
    id,
    "lmos",
    "<command id=\"" +
    str(id) +
    "\" layer=\"RofinStandard\" laserable=\"false\">" +
    str(kLayerSetLaserable) +
    "</command>",
    1000)

if LIVE:
    id = id + 1
    bp_sendMessageAndWait(
        self,
        id,
        "lmos",
        "<command id=\"" +
        str(id) +
        "\" object=\"triangle02\">" +
        str(kStartPosHelp) +
        "</command>",
        1000)

if LIVE:
    id = id + 1
    bp_sendCommand(self, id, "lmos", kStopPosHelp)
if LIVE:
    id = id + 1
    bp_sendCommand(self, id, "lmos", kLoadJob)
id = id + 1

bp_sendCommandAndWait(self, id, "lmos", kMOUnsetLaserable, 1000)
id = id + 1

bp_sendMessageAndWait(
    self,
    id,
    "lmos",
    "<command id=\"" +
    str(id) +
    "\" object=\"triangle01\">" +
    str(kMOSetLaserable) +
    "</command>",
    1000)

if LIVE:
    id = id + 1
    bp_sendCommand(self, id, "lmos", kStartMarking)
id = id + 1

bp_sendCommand(self, id, "lmos", kCancelJob)

if LIVE:
    id = id + 1
    bp_sendCommand(self, id, "lmos", kTermMachine)
    #bp_sendCommand(self, 95, "lmos", kExit)

bp_disconnectFromServer(self)
