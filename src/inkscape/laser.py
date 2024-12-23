#!/usr/bin/env python3

# Inkscape extension for blaspit

import csv
import datetime
import io
import json
import math
import os
import re
import socket

# Needed until blastpy is installed system-wide
import sys
from os.path import expanduser
from shutil import copy2

# For duplicating and processing the SVG file
from subprocess import PIPE, Popen

import inkex

sys.path.append(expanduser("~") + "/projects/blastpit/build")
import blastpy

# Import local environment variables and configuration
from dotenv import load_dotenv

if load_dotenv(dotenv_path=".env", verbose=True) is False:
    print(f"Error loading dotenv (cwd: {os.getcwd()})", file=sys.stderr)

# Laser constants
SAGITTA = 0.5  # Focal range
ROTARY_Z_LEVEL = 77.0  # Height from table to centre of rotation
ROTARY_OVERLAP = 0.0  # Shadow overlap
MINIMUM_Z_HEIGHT = 45.0
MAXIMUM_Z_HEIGHT = 120.0

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

    layers.append(nb)


class Laser(inkex.Effect):
    def __init__(self):
        inkex.Effect.__init__(self)

    def setupServer(self):
        # Connect to server
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

    def _update_attribute(self, attribute: str, type_, data):
        try:
            value = type_(data[attribute])
            setattr(self, attribute, value)
        except:
            pass

    def effect(self):
        # Find the ring or flat data
        self.rofin = "flat"
        self.height = 120.0
        self.mode = "save"
        self.current = 0
        self.frequency = 60000
        self.freqstep = 0
        self.speed = 500
        self.server = os.getenv("WS_SERVER_REMOTE")
        self.filename = None
        self.customer = None

        # Defaults
        for group in self.document.getroot():
            for child in group:
                if (
                    "{http://www.inkscape.org/namespaces/inkscape}label"
                    in child.attrib.keys()
                ):
                    if (
                        "laserdata"
                        in child.attrib[
                            "{http://www.inkscape.org/namespaces/inkscape}label"
                        ]
                    ):
                        try:
                            myjson = json.loads(child.text)
                        except json.decoder.JSONDecodeError:
                            continue

                        self._update_attribute("current", float, myjson)
                        self._update_attribute("freqstep", float, myjson)
                        self._update_attribute("frequency", float, myjson)
                        self._update_attribute("mode", str, myjson)
                        self._update_attribute("speed", float, myjson)
                        self._update_attribute("server", str, myjson)
                        self._update_attribute("height", float, myjson)
                        self._update_attribute("filename", str, myjson)
                        self._update_attribute("customer", str, myjson)
                        self._update_attribute("angle", float, myjson)
                        if "angle" in myjson:
                            self.rofin = "rotary"
                        self._update_attribute("diameter", float, myjson)
                        if "diameter" in myjson:
                            self.rofin = "rotary"
                        self._update_attribute("height", float, myjson)
                        self._update_attribute("sectors", float, myjson)
                        self._update_attribute("width", float, myjson)

        self.blast = blastpy.t_Blastpit()

        # Find the current SVG drawing and copy to a temporary file
        file = self.options.input_file
        tempfile = os.path.splitext(file)[0] + "-multicut.svg"
        copy2(file, tempfile)

        # Pipe the SVG through an external program 'svg2bezier'
        # to convert to cubic bezier splines. Store in '/tmp/out.txt'
        p = Popen(
            "cat " + tempfile + " | svg2bezier > /tmp/out.txt",
            shell=True,
            stdout=PIPE,
            stderr=PIPE,
        )
        f = p.communicate()[0]

        fh = open("/tmp/out.txt", "r")

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
            # SECTOR_WIDTH = (math.pi * float(self.diameter)) / SEGMENTS
            # RINGWIDTH = self.options.width
            FOCAL_ADJUSTMENT = 0

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
                # print("Jig angle: " + str(self.angle), file=sys.stderr)
            else:
                convex = True
            xml.setCylinder(
                float(self.diameter) / 2, float(self.width), SECTOR, convex
            )

        # Iterate through the cubic bezier shapes.
        # Ignore anything with a shadow id.
        # Group shapes with the same colour.
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
                    # print >> sys.stderr, "Adding Qp set bp_" + colour

                    # Create a QP set based on hex colour with a prefix 'bp_'
                    qp = ["bp_" + colour, self.current, self.speed, freq]
                    qpsets.append(qp)
                    freq += int(self.freqstep)

                regx = re.compile(r"^id: (.+), fill:.*$")
                m = regx.match(line)
                if m.group(1) is not None:
                    id = m.group(1)
                else:
                    id = line
                # group = xml.addGroup(
                #     "g" + id, "bp_" + colour, "bp_" + colour, "Standard")
                group = xml.addGroup(
                    "g" + id, "bp_" + colour, "bp_" + colour, "bp_0_01"
                )
            else:
                if ignore == 1:
                    continue
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

        if (
            self.mode == "save"
            or self.mode == "upload"
            or self.mode == "savexml"
        ):
            blast = blastpy.blastpitNew()
            result = blastpy.connectToServer(
                blast, self.server, int(os.getenv("WS_TIMEOUT_SHORT"))
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
                    blast, id.id, int(os.getenv("WS_TIMEOUT_UPLOAD"))
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
            blastpy.BpWaitForReplyOrTimeout(
                blast, id.id, int(os.getenv("WS_TIMEOUT_LONG"))
            )

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

            # blastpy.BpQueueCommandArgs(
            #     blast,
            #     blastpy.kLayerSetLaserable,
            #     "layer",
            #     "RofinStandard",
            #     "laserable",
            #     "0",
            #     None,
            #     None,
            #     None,
            #     None,
            #     None,
            # )
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
                filename = "C:\\Rofin\\VisualLaserMarker\\MarkingFiles\\inkscape_export.VLM"

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
            blastpy.BpWaitForReplyOrTimeout(
                blast, id.id, int(os.getenv("WS_TIMEOUT_LONG"))
            )
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
                blastpy.BpWaitForReplyOrTimeout(
                    blast, id.id, int(os.getenv("WS_TIMEOUT_SHORT"))
                )
                if id.retval != blastpy.kSuccess:
                    print("Error patching shadows", file=sys.stderr)

        if self.mode == "save" or self.mode == "upload":
            blastpy.disconnectFromServer(blast)
            blastpy.blastpitDelete(blast)

        if self.mode == "position":
            blast = blastpy.t_Blastpit()
            if (
                blastpy.bp_connectToServer(
                    blast, self.server, "inkscape", 1000, True
                )
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
                blast, self.server, int(os.getenv("WS_TIMEOUT_SHORT"))
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

# If any items are selected, export only those items
# If no objects are selected, export all items on visible layers
# Convert text to paths
# Merge any paths whose bounding boxes overlap

# inkscape --pipe --export-id-only --export-text-to-path --vacuum-defs --export-plain-svg -i text1085 -o - < drawing.svg
# REMEMBER TO MERGE TEXT

# for id_, node in self.selected.iteritems():
#     # get value of attribute 'inkscape:label' from current node
#     node_label = node.get(inkex.addNS('label', 'inkscape'),
#        "No label set for this element")
#     # report back
#     inkex.debug("Label of object %s: %s" % (id_, node_label))
#     # if label starts with "foo", modify it
#     if node_label.startswith("foo"):
#         new_label = node_label[0:3] + "bar"
#         # set label to new value
#         node.set(inkex.addNS('label', 'inkscape'), new_label)
#     # report back current value of attribute 'inkscape:label'
#     inkex.debug("Label of object %s: %s" % (id_,
#           node.get(inkex.addNS('label', 'inkscape'))))
