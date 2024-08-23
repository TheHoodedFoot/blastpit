#!/usr/bin/env python3

# Inkscape extension for blaspit

import socket
import math
import time
import re
import csv

# from StringIO import StringIO
import io
import inkex
import os
import json
import datetime

# For duplicating and processing the SVG file
from subprocess import Popen, PIPE
from shutil import copy2

# Needed until blastpy is installed system-wide
import sys
from os.path import expanduser

sys.path.append(expanduser("~") + "/projects/blastpit/build")
import blastpy

# Import our machine-specific constants
sys.path.append(expanduser("~") + "/projects/blastpit/.git/untracked")
import myconfig

# Laser constants
SAGITTA = 0.5  # Focal range
ROTARY_Z_LEVEL = 77.0  # Height from table to centre of rotation
ROTARY_OVERLAP = 0.0  # Shadow overlap
MINIMUM_Z_HEIGHT = 45.0

# Global layer array
layers = []
qpsets = []


def chordLength(radius, sagitta):
    return 2 * math.sqrt(2 * radius * sagitta - sagitta * sagitta)


def chordAngle(radius, sagitta):
    return 2 * math.asin(chordLength(radius, sagitta) / (2 * radius))


def chordArcLength(radius, sagitta):
    c = chordLength(radius, sagitta)
    hc = sagitta + ((c * c) / (4 * sagitta))
    return math.asin(c / hc) * hc


def addColourLayer(xml, colour, height=120):
    red = (int(colour[-6:], 16) >> 16) & 0xFF
    green = (int(colour[-6:], 16) >> 8) & 0xFF
    blue = (int(colour[-6:], 16)) & 0xFF

    xml.addLayer("bp_" + colour, round(height, 2), red, green, blue)

    nb = ["bp_" + colour, round(height, 2)]
    # print >> sys.stderr, nb

    layers.append(nb)


class Laser(inkex.Effect):
    def __init__(self):
        inkex.Effect.__init__(self)

    def setupServer(self):
        # Attempt to connect to server. Return None on failure
        try:
            bp = blastpy.connectToServer(
                self.blast, self.server, "INKSCAPE", 1000, True
            )
        except socket.gaierror:
            return None

        # We may as well do this here
        bp.clearQpSets(999)

        if self.mode == "cancel":
            bp.terminate(999)
            return None
        else:
            return bp

    def effect(self):
        # Extract json data

        # Defaults
        self.rofin = "flat"
        self.height = 120.0
        self.mode = "save"
        self.current = 0
        self.frequency = 60000
        self.freqstep = 0
        self.speed = 500
        self.server = myconfig.WS_SERVER_REMOTE
        self.filename = None
        self.customer = None

        key_to_attribute = {
            "current": "current",
            "freqstep": "freqstep",
            "frequency": "frequency",
            "mode": "mode",
            "speed": "speed",
            "server": "server",
            "filename": "filename",
            "customer": "customer",
            "angle": "angle",
            "diameter": "diameter",
            "height": "height",
            "sectors": "sectors",
            "width": "width",
        }

        for group in self.document.getroot():
            for child in group:
                if (
                    "{http://www.inkscape.org/namespaces/inkscape}label"
                    in child.attrib.keys()
                ) and (
                    "laserdata"
                    in child.attrib[
                        "{http://www.inkscape.org/namespaces/inkscape}label"
                    ]
                ):
                    try:
                        myjson = json.loads(child.text)
                    except json.decoder.JSONDecodeError:
                        myjson = None

                    if myjson is not None:
                        for key, attribute in key_to_attribute.items():
                            try:
                                setattr(self, attribute, float(myjson[key]))
                            except (ValueError, TypeError, KeyError):
                                continue

                        if hasattr(self, "angle") or hasattr(self, "diameter"):
                            self.rofin = "rotary"

        self.blast = blastpy.t_Blastpit()

        # Here we copy the current SVG drawing to a temporary file
        file = self.options.input_file
        tempfile = os.path.splitext(file)[0] + "-multicut.svg"
        copy2(file, tempfile)

        # Now we Pipe the file through 'svg2bezier'
        # to convert all geometries to simple bezier curves
        SIMPLIFIED_CURVES_FILENAME = "/tmp/simple_curves.out"
        p = Popen(
            "cat " + tempfile + " | svg2bezier > " + SIMPLIFIED_CURVES_FILENAME,
            shell=True,
            stdout=PIPE,
            stderr=PIPE,
        )
        f = p.communicate()[0]

        fh = open(SIMPLIFIED_CURVES_FILENAME, "r")

        freq = int(self.frequency)

        if self.rofin == "rotary":
            RADIUS = float(self.diameter) / 2

            # Calculate the segment size based on including the overlap each end.
            # This makes the segments smaller, but prevents the overlap falling
            # outside of the focal range.
            SEGMENTS = int(
                math.ceil(
                    (math.pi * float(self.diameter))
                    / (chordArcLength(RADIUS, SAGITTA) - 2 * ROTARY_OVERLAP)
                )
            )

            SECTOR = 360.0 / SEGMENTS  # Needed in degrees for lmos
            FOCAL_ADJUSTMENT = 0

        # Read the splines from the file and convert
        # into the XML format required by LMOS
        id = "error"
        geo = None
        group = None
        collayers = []
        lastx = None
        lasty = None

        # The main XML object
        xml = blastpy.BpXML()

        # If this is a rotary object we must set the cylinder dimensions
        if self.rofin == "rotary":
            if int(self.angle) > 0:
                convex = False
            else:
                convex = True
            xml.setCylinder(float(self.diameter) / 2, float(self.width), SECTOR, convex)

        # Loop through all of the simplified splines
        id_counter = 0
        for line in fh:
            if line.startswith("id:"):
                if line.startswith("id: shadow"):  # Shadows are ignored
                    ignore = 1
                    continue
                else:
                    ignore = 0

                line = line[:-1]  # Remove newline
                colour = line[-6:]  # Last six digits are the colour in hex
                if (
                    colour not in collayers
                ):  # Check if we have already created a layer for this colour
                    collayers.append(
                        colour
                    )  # Create a new layer with the name of the hex colour
                    if self.rofin == "rotary":
                        if not convex:
                            rotaryLayerHeight = (
                                ROTARY_Z_LEVEL - RADIUS + FOCAL_ADJUSTMENT
                            )
                        else:
                            rotaryLayerHeight = (
                                ROTARY_Z_LEVEL + RADIUS + FOCAL_ADJUSTMENT
                            )
                        if (
                            rotaryLayerHeight < MINIMUM_Z_HEIGHT
                        ):  # Rotary marking has a minimum height
                            rotaryLayerHeight = MAXIMUM_Z_HEIGHT
                        addColourLayer(xml, colour, float(str(self.height)))
                    else:
                        addColourLayer(xml, colour, float(str(self.height)))

                    # Create a QP set based on the hex colour but with a prefix 'bp_'
                    qp = ["bp_" + colour, self.current, self.speed, freq]
                    qpsets.append(qp)
                    freq += int(self.freqstep)

                regx = re.compile(r"^id: (.+), fill:.*$")
                m = regx.match(line)
                if m.group(1) is not None:
                    id = m.group(1)
                else:
                    id = line
                group = xml.addGroup(
                    "g" + id, "bp_" + colour, "bp_" + colour, "bp_0_01"
                )
            else:
                if ignore == 1:
                    continue
                # If the line is bezier data, it will be
                # four numbers separated by spaces
                f = io.StringIO(line)
                reader = csv.reader(f, delimiter=",")
                nodes = []
                for row in reader:
                    for i in row:
                        nodes.append(float(i))
                    # Flip in y because SVG is opposite to LMOS
                    for i in range(len(nodes)):
                        if i % 2:
                            nodes[i] = 120 - nodes[i]

                if nodes:
                    if geo is None:
                        geo = blastpy.BpPolyline(
                            id + "#" + str(colour),
                            nodes[0],
                            nodes[1],
                            "bp_" + str(colour),
                        )
                        geo.setLayer("bp_" + str(colour))
                        lastx = nodes[0]
                        lasty = nodes[1]
                    # Is this a new sub path?
                    if nodes[0] != lastx or nodes[1] != lasty:
                        geo.close()
                        xml.addPolylineG(geo, group)
                        geo = blastpy.BpPolyline(
                            id + "-" + str(id_counter) + "#" + str(colour),
                            nodes[0],
                            nodes[1],
                            "bp_" + str(colour),
                        )
                        id_counter += 1
                        geo.setLayer("bp_" + str(colour))
                    geo.bezier(
                        nodes[2],
                        nodes[3],
                        nodes[4],
                        nodes[5],
                        nodes[6],
                        nodes[7],
                    )
                    lastx = nodes[6]
                    lasty = nodes[7]
                else:
                    # If the line is not closed, don't add it.
                    # This fixes the problem of open paths and
                    # text-on-path guide paths
                    geo_start = geo.getStart()
                    if geo_start[0] == lastx and geo_start[1] == lasty:
                        geo.close()
                        xml.addPolylineG(geo, group)
                    else:
                        xml.addPolyline(geo)
                    id_counter = 1
                    geo = None
                    group = None

        # All data has now been converted.
        # Here we save the XML or send it over the network
        if self.mode == "save" or self.mode == "upload" or self.mode == "savexml":
            blast = blastpy.blastpitNew()
            result = blastpy.connectToServer(
                blast, self.server, myconfig.WS_TIMEOUT_SHORT
            )
            if result != blastpy.kSuccess:
                print("Can't connect to server (%d)" % result, file=sys.stderr)
                fh.close()
                sys.exit()

            if blastpy.BpIsLmosUp(blast) != blastpy.kSuccess:
                print("Lmos client could not be reached", file=sys.stderr)
                fh.close()
                sys.exit()

            blastpy.BpQueueCommand(blast, blastpy.kClearLog)
            blastpy.BpQueueCommand(blast, blastpy.kResetRetvalDb)
            blastpy.BpUploadQueuedMessages(blast)

            blastpy.BpQueueCommand(blast, blastpy.kClearQpSets)
            for qpset in qpsets:
                blastpy.BpQueueQpSet(
                    blast, qpset[0], int(qpset[1]), int(qpset[2]), int(qpset[3])
                )
            blastpy.BpQueueCommand(blast, blastpy.kSaveQpSets)
            blastpy.BpUploadQueuedMessages(blast)

            if self.mode == "save" or self.mode == "upload":
                id = blastpy.BpQueueCommandArgs(
                    blast,
                    blastpy.kImportXML,
                    str(xml.xml()),
                    None,
                    None,
                    None,
                    None,
                    None,
                    None,
                    None,
                    None,
                )
                blastpy.BpUploadQueuedMessages(blast)
                blastpy.BpWaitForReplyOrTimeout(
                    blast, id.id, myconfig.WS_TIMEOUT_UPLOAD
                )

            id = blastpy.BpQueueCommandArgs(
                blast,
                blastpy.kZoomWindow,
                "x1",
                "0",
                "y1",
                "0",
                "x2",
                "60",
                "y2",
                "60",
                None,
            )

            # Finally, show lmos window
            blastpy.BpDisplayLmosWindow(blast, 1)

            blastpy.BpUploadQueuedMessages(blast)
            blastpy.BpWaitForReplyOrTimeout(blast, id.id, myconfig.WS_TIMEOUT_LONG)

            for layer in layers:
                blastpy.BpQueueCommandArgs(
                    blast,
                    blastpy.kLayerSetHeight,
                    "layer",
                    str(layer[0]),
                    "height",
                    str(layer[1]),
                    None,
                    None,
                    None,
                    None,
                    None,
                )
                blastpy.BpQueueCommandArgs(
                    blast,
                    blastpy.kLayerSetExportable,
                    "layer",
                    str(layer[0]),
                    "exportable",
                    "1",
                    None,
                    None,
                    None,
                    None,
                    None,
                )

            blastpy.BpQueueCommandArgs(
                blast,
                blastpy.kLayerSetLaserable,
                "layer",
                "RofinStandard",
                "laserable",
                "0",
                None,
                None,
                None,
                None,
                None,
            )
            blastpy.BpQueueCommandArgs(
                blast,
                blastpy.kLayerSetExportable,
                "layer",
                "RofinStandard",
                "exportable",
                "0",
                None,
                None,
                None,
                None,
                None,
            )
            blastpy.BpQueueCommandArgs(
                blast,
                blastpy.kLayerSetHeight,
                "layer",
                "RofinStandard",
                "height",
                "120.0",
                None,
                None,
                None,
                None,
                None,
            )
            blastpy.BpQueueCommandArgs(
                blast,
                blastpy.kLayerSetHeight,
                "layer",
                "RofinBackground",
                "height",
                "120.0",
                None,
                None,
                None,
                None,
                None,
            )
            blastpy.BpUploadQueuedMessages(blast)

        if self.mode == "save":
            tmpxml = open("/tmp/inkscape_xml.xml", "w")
            print(xml.xml(), file=tmpxml)
            tmpxml.close()
            if self.filename is not None and self.customer is not None:
                filename = (
                    "Z:\\drawings\\"
                    + str(datetime.date.today().year)
                    + "\\"
                    + self.customer.upper()
                    + "\\"
                    + self.filename
                    + ".VLM"
                )
            else:
                filename = (
                    "C:\\Rofin\\VisualLaserMarker\\MarkingFiles\\inkscape_export.VLM"
                )

            id = blastpy.BpQueueCommandArgs(
                blast,
                blastpy.kSaveVLM,
                "filename",
                filename,
                None,
                None,
                None,
                None,
                None,
                None,
                None,
            )
            blastpy.BpUploadQueuedMessages(blast)
            blastpy.BpWaitForReplyOrTimeout(blast, id.id, myconfig.WS_TIMEOUT_LONG)
            if id.retval != blastpy.kSuccess:
                print("Error saving VLM file", file=sys.stderr)
                fh.close()
                sys.exit()

            if self.rofin == "rotary":
                id = blastpy.BpQueueCommandArgs(
                    blast,
                    blastpy.kPatchFlexibleShadows,
                    "filename",
                    filename,
                    None,
                    None,
                    None,
                    None,
                    None,
                    None,
                    None,
                )
                blastpy.BpUploadQueuedMessages(blast)
                blastpy.BpWaitForReplyOrTimeout(blast, id.id, myconfig.WS_TIMEOUT_SHORT)
                if id.retval != blastpy.kSuccess:
                    print("Error patching shadows", file=sys.stderr)

        if self.mode == "save" or self.mode == "upload":
            blastpy.disconnectFromServer(blast)
            blastpy.blastpitDelete(blast)

        if self.mode == "position":
            blast = blastpy.t_Blastpit()
            if (
                blastpy.bp_connectToServer(blast, self.server, "inkscape", 1000, True)
                != 0
            ):
                print("Can't connect", file=sys.stderr)
                sys.exit()

        if self.mode == "view":
            print(qpsets, file=sys.stderr)
            print(file=sys.stderr)
            print(xml.xml(), file=sys.stderr)

        if self.mode == "savexml":
            tmpxml = open("/tmp/inkscape_xml.xml", "w")
            print(xml.xml().decode(), file=tmpxml)
            tmpxml.close()

            # We also need to upload the LPs
            blast = blastpy.blastpitNew()
            result = blastpy.connectToServer(
                blast, self.server, myconfig.WS_TIMEOUT_SHORT
            )
            if result != blastpy.kSuccess:
                print("Can't connect to server (%d)" % result, file=sys.stderr)
                fh.close()
                sys.exit()

            if blastpy.BpIsLmosUp(blast) != blastpy.kSuccess:
                print("Lmos client could not be reached", file=sys.stderr)
                fh.close()
                sys.exit()

            blastpy.BpQueueCommand(blast, blastpy.kClearLog)
            blastpy.BpQueueCommand(blast, blastpy.kResetRetvalDb)
            blastpy.BpUploadQueuedMessages(blast)

            blastpy.BpQueueCommand(blast, blastpy.kClearQpSets)
            for qpset in qpsets:
                blastpy.BpQueueQpSet(
                    blast, qpset[0], int(qpset[1]), int(qpset[2]), int(qpset[3])
                )
            blastpy.BpQueueCommand(blast, blastpy.kSaveQpSets)
            blastpy.BpUploadQueuedMessages(blast)
            blastpy.disconnectFromServer(blast)
            blastpy.blastpitDelete(blast)

        fh.close()


if __name__ == "__main__":
    e = Laser()
    e.run()
