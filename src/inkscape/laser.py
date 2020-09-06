#!/usr/bin/env python3

# Inkscape extension for blaspit
#
# Original work copyright 2014 Ryan Lerch
# Modified work copyright 2016 Maren Hachmann <marenhachmannATyahoo.com>
# Modified work copyright 2016 Andrew Black <andrew@rfbevan.co.uk>
#
#    This program is free software; you can redistribute it and/or modify
#    it under the terms of the GNU General Public License as published by
#    the Free Software Foundation; either version 2 of the License, or
#    (at your option) any later version.
#
#    This program is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#    GNU General Public License for more details.
#
#    You should have received a copy of the GNU General Public License along
#    with this program; if not, write to the Free Software Foundation, Inc.,
#    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.


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

# Needed until bpy is installed system-wide
import sys
from os.path import expanduser
sys.path.append(expanduser("~") + "/projects/blastpit/build")
import blastpy

# This disables stderr to prevent popup window when debugging
# os.close(sys.stderr.fileno())

# Server timeouts
SHORT_TIMEOUT=2000
LONG_TIMEOUT=60000

# Laser constants
SAGITTA = 0.5  # Focal range
ROTARY_Z_LEVEL = 77.0  # Height from table to centre of rotation
ROTARY_OVERLAP = 0.0  # Shadow overlap


# Math functions

# https://en.wikipedia.org/wiki/Sagitta_(geometry)
# https://en.wikipedia.org/wiki/Circular_segment


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
    red = (int(colour[-6:], 16) >> 16) & 0xff
    green = (int(colour[-6:], 16) >> 8) & 0xff
    blue = (int(colour[-6:], 16)) & 0xff

    xml.addLayer(
        "bp_" + colour,
        round(height, 2),
        red,
        green,
        blue)

    nb = ["bp_" + colour, round(height, 2)]
    # print >> sys.stderr, nb

    layers.append(nb)


class Laser(inkex.Effect):
    def __init__(self):
        inkex.Effect.__init__(self)

    def setupServer(self):
        # Connect to server
        try:
            bp = blastpy.connectToServer(
                self.blast, self.server, "INKSCAPE", 1000, True)
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

        # Find the ring or flat data

        # Defaults
        self.rofin = "flat"
        self.height = 120.0
        self.mode = "save"
        self.current = 0
        self.frequency = 60000
        self.freqstep = 0
        self.speed = 500
        self.server = "ws://10.47.1.30:8000"
        self.filename = None
        self.customer = None

        for group in self.document.getroot():
            for child in group:
                if '{http://www.inkscape.org/namespaces/inkscape}label' in child.attrib.keys():
                    if "laserdata" in child.attrib['{http://www.inkscape.org/namespaces/inkscape}label']:
                        myjson = json.loads(child.text)
                        if myjson is not None:
                            try:
                                self.current = float(myjson["current"])
                            except BaseException:
                                pass
                            try:
                                self.freqstep = float(myjson["freqstep"])
                            except BaseException:
                                pass
                            try:
                                self.frequency = float(myjson["frequency"])
                            except BaseException:
                                pass
                            try:
                                self.mode = myjson["mode"]
                            except BaseException:
                                pass
                            try:
                                self.speed = float(myjson["speed"])
                            except BaseException:
                                pass
                            try:
                                self.server = myjson["server"]
                            except BaseException:
                                pass
                            try:
                                self.height = float(myjson["height"])
                            except BaseException:
                                pass
                            try:
                                self.filename = myjson["filename"]
                            except BaseException:
                                pass
                            try:
                                self.customer = myjson["customer"]
                            except BaseException:
                                pass
                            try:
                                self.angle = float(myjson["angle"])
                                self.rofin = "rotary"
                            except BaseException:
                                pass
                            try:
                                self.diameter = float(myjson["diameter"])
                                self.rofin = "rotary"
                            except BaseException:
                                pass
                            try:
                                self.height = float(myjson["height"])
                            except BaseException:
                                pass
                            try:
                                self.sectors = float(myjson["sectors"])
                            except BaseException:
                                pass
                            try:
                                self.width = float(myjson["width"])
                            except BaseException:
                                pass

        self.blast = blastpy.t_Blastpit()

        # If this is rotary, compile a list of shadows from the shadow layers,

        # HARD: Calculate all shadows

        # Explode all text
        # Convert all strokes to paths
        # Merge all paths
        # For each shadow, crop to the shadow and export/laser

        # Work backwards
        #
        # Starting with all of the geometries ready to go and all of the shadows calculated,
        # loop through the shadows, isolate the geometries within that shadow,
        # and mark the geometries

        file = self.options.input_file
        tempfile = os.path.splitext(file)[0] + "-multicut.svg"
        copy2(file, tempfile)

        # Pipe the file through svg.c
        p = Popen(
            "cat " + tempfile + " | svg2bezier > /tmp/out.txt",
            shell=True,
            stdout=PIPE,
            stderr=PIPE)
        f = p.communicate()[0]

        fh = open("/tmp/out.txt", 'r')

        ##########################
        #
        # Main program starts here
        #
        ##########################

        freq = 60000

        if self.rofin == "rotary":
            RADIUS = float(self.diameter) / 2

            # Calculate the segment size based on including the overlap each end.
            # This makes the segments smaller, but prevents the overlap falling
            # outside of the focal range.
            SEGMENTS = int(math.ceil((math.pi * float(self.diameter))
                                     / (chordArcLength(RADIUS, SAGITTA) -
                                        2 * ROTARY_OVERLAP)))

            SECTOR = 360.0 / SEGMENTS  # Needed in degrees for lmos
            SECTOR_WIDTH = (math.pi * float(self.diameter)) / SEGMENTS
            # RINGWIDTH = self.options.width
            FOCAL_ADJUSTMENT = 0

            freq = int(self.frequency)

        id = "error"
        geo = None
        group = None
        collayers = []
        lastx = None
        lasty = None

        # The main XML object
        xml = blastpy.BpXML()

        # print(self.options.rotary, file=sys.stderr)
        if self.rofin == "rotary":
            if int(self.angle) > 0:
                convex = False
                print("Jig angle: " + str(self.angle), file=sys.stderr)
            else:
                convex = True
            xml.setCylinder(
                float(self.diameter) / 2,
                float(self.width),
                SECTOR,
                convex)

        for line in fh:
            if line.startswith("id:"):
                if line.startswith("id: shadow"):
                    ignore = 1
                    continue
                else:
                    ignore = 0
                line = line[:-1]  # Remove newline
                colour = line[-6:]
                if colour not in collayers:
                    collayers.append(colour)
                    if self.rofin == "rotary":
                        if not convex:
                            rotaryLayerHeight = 77 - \
                                RADIUS + FOCAL_ADJUSTMENT
                        else:
                            rotaryLayerHeight = 77 + \
                                RADIUS + FOCAL_ADJUSTMENT
                        if rotaryLayerHeight < 45:
                            rotaryLayerHeight = 120
                        # print >> sys.stderr, "Layer height: " + \
                        #     str(rotaryLayerHeight)
                        addColourLayer(xml, colour, float(str(self.height)))
                    else:
                        addColourLayer(xml, colour, float(str(self.height)))
                    # print >> sys.stderr, "Adding Qp set bp_" + colour

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
                    "g" + id, "bp_" + colour, "bp_" + colour, "bp_0_01")
            else:
                if ignore == 1:
                    continue
                f = io.StringIO(line)
                reader = csv.reader(f, delimiter=',')
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
                            id, nodes[0], nodes[1],
                            "bp_" + str(colour), "Standard")
                        geo.setLayer("bp_" + str(colour))
                        lastx = nodes[0]
                        lasty = nodes[1]
                    # Is this a new sub path?
                    if nodes[0] != lastx or nodes[1] != lasty:
                        geo.close()
                        xml.addPolylineG(geo, group)
                        geo = blastpy.BpPolyline(
                            id, nodes[0], nodes[1],
                            "bp_" + str(colour), "Standard")
                        geo.setLayer("bp_" + str(colour))
                    geo.bezier(
                        nodes[2],
                        nodes[3],
                        nodes[4],
                        nodes[5],
                        nodes[6],
                        nodes[7])
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
                    geo = None
                    group = None

        if self.mode == "engraveclean":
            bp.init(997)
            bp.upload(2, xml.xml())
            bp.layerSetLaserable(999, "RofinStandard", 0)
            # bp.stopMarking(998)
            bp.setDimension(992, "GalvoGroupShadow101", SECTOR_WIDTH, 120)
            bp.setPosValues(991, "GalvoGroupShadow101", 0, 0, 0)
            bp.startMarking(3)

        if self.mode == "save":
            blast = blastpy.blastpitNew()
            result = blastpy.connectToServer( blast, self.server, SHORT_TIMEOUT)
            if result != blastpy.kSuccess:
                print("Can't connect to server (%d)" % result, file=sys.stderr)
                sys.exit()
            blastpy.SendCommand(blast, blastpy.kClearLog)
            result = blastpy.bp_sendCommandAndWait( blast, blastpy.kClearQpSets, SHORT_TIMEOUT)
            if result.retval != blastpy.kSuccess:
                print("Cannot clear Qpsets (is lmos running?)", file=sys.stderr)
                print("Error: %s" % blastpy.bpRetvalName(result.retval), file=sys.stderr)
                sys.exit()

            print(qpsets, file=sys.stderr)
            for qpset in qpsets:
                blastpy.BpQueueQpSet(blast, qpset[0], int(qpset[1]), int(qpset[2]), int(qpset[3]))
            blastpy.BpUploadQueuedMessages(blast)

            if result.retval != blastpy.kSuccess:
                print("Cannot add Qpsets", file=sys.stderr)
                print("num messages: %d" % blastpy.getMessageCount(blast), file=sys.stderr)
                print("Error: %s" % blastpy.bpRetvalName(result.retval), file=sys.stderr)
                sys.exit()

            xml.setCommand(blastpy.kImportXML)
            result = blastpy.bp_sendMessageAndWait( blast, str(xml.xml()), LONG_TIMEOUT)
            if result.retval != blastpy.kSuccess:
                print("Can't send the drawing XML", file=sys.stderr)
                print("Error: %s" % blastpy.bpRetvalName(result.retval), file=sys.stderr)
                sys.exit()
            result = blastpy.LayerSetLaserable(blast, "RofinStandard", False)
            if result.retval != blastpy.kSuccess:
                print("Can't set RofinStandard layer as not laserable", file=sys.stderr)
                print("Error: %s" % blastpy.bpRetvalName(result.retval), file=sys.stderr)
                sys.exit()
            blastpy.LayerSetHeight(blast, "RofinStandard", 120)
            blastpy.LayerSetHeight(blast, "RofinBackground", 120)
            for layer in layers:
                blastpy.LayerSetHeight(blast, 0, str(layer[0]), float(layer[1]))
            if self.filename is not None and self.customer is not None:
                # print("saving as customer/filename", file=sys.stderr)
                blastpy.bp_sendMessageAndWait(blast, id, "<command id=\"" +
                                              str(id) +
                                              "\" filename=\"Z:\\drawings\\" +
                                              str(datetime.date.today().year) +
                                              "\\" +
                                              self.customer +
                                              "\\" +
                                              self.filename +
                                              ".VLM\">" +
                                              str(blastpy.kSaveVLM) +
                                              "</command>", SHORT_TIMEOUT)
            else:
                # print("saving default", file=sys.stderr)
                blastpy.bp_sendMessageAndWait(
                    blast,
                    id,
                    "<command id=\"" +
                    str(id) +
                    "\" filename=\"C:\\Rofin\\VisualLaserMarker\\MarkingFiles\\inkscape_export.VLM\">" +
                    str(
                        blastpy.kSaveVLM) +
                    "</command>",
                    SHORT_TIMEOUT)
            blastpy.disconnectFromServer(blast)

        if self.mode == "position":
            blast = blastpy.t_Blastpit()
            if blastpy.bp_connectToServer(
                    blast, self.server, "inkscape", 1000, True) != 0:
                print("Can't connect", file=sys.stderr)
                sys.exit()

        if self.mode == "view":
            print(qpsets.xml(), file=sys.stderr)
            print(file=sys.stderr)
            print(xml.xml(), file=sys.stderr)

        fh.close()


if __name__ == '__main__':
    e = Laser()
    e.run()


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
