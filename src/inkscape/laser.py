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
from StringIO import StringIO
import inkex
import os

# For duplicating and processing the SVG file
from subprocess import Popen, PIPE
from shutil import copy2

# Needed until bpy is installed system-wide
import sys
from os.path import expanduser
sys.path.append(expanduser("~") + "/usr/src/blastpit/src/libbp")
import blastpy


# Laser constants
SAGITTA = 0.5  # Focal range
ROTARY_Z_LEVEL = 77.0  # Height from table to centre of rotation
ROTARY_OVERLAP = 0.0  # Shadow overlap


# Math functions

# https://en.wikipedia.org/wiki/Sagitta_(geometry)
# https://en.wikipedia.org/wiki/Circular_segment


# Global layer array
layers = []


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
        options = [
            ["a", "name", "string", "", "Unknown"],
            ["c", "current", "int", "20", "Laser current"],
            ["d", "diameter", "float", "0", "Ring diameter"],
            ["e", "offset", "float", "0", "Ring holder offset"],
            ["f", "flatten", "inkbool", "True", "Should text be flattened"],
            ["i", "height", "float", "120", "Default layer height"],
            ["m", "mode", "string", "", "Operating mode of laser"],
            ["n", "ring_type", "string", "", "Concave or convex ring type"],
            ["o", "holder", "string", "", "Ring holder type"],
            ["p", "port", "int", "1030", "The server port"],
            ["q", "frequency", "int", "44000", "Start frequency"],
            ["r", "rotary", "inkbool", "False", "Enable rotary axis"],
            ["v", "override", "inkbool", "False", "Override the 20 degree limit"],
            ["s", "server", "string", "localhost", "The server address"],
            ["t", "tabs", "string", "", ""],
            ["w", "width", "float", "6.0", "Ring width"],
            ["y", "speed", "int", "500", "Laser speed"],
            ["z", "freqstep", "int", "-1000", "Frequency step"],
        ]
        for arg in options:
            self.OptionParser.add_option(
                "-" + arg[0],
                "--" + arg[1],
                action="store",
                type=arg[2],
                dest=arg[1],
                default=arg[3],
                help=arg[4])

    def setupServer(self):
        # Connect to server
        try:
            bp = blastpy.bp_connectToServer(self.blast, self.options.server, "INKSCAPE", 1000)
        except socket.gaierror:
            return None

        # We may as well do this here
        bp.clearQpSets(999)

        if self.options.mode == "cancel":
            bp.terminate(999)
            return None
        else:
            return bp

    def effect(self):

        self.blast = blastpy.t_Blastpit()

        # If this is rotary, compile a list of shadows from the shadow layers,

        file = self.args[-1]
        tempfile = os.path.splitext(file)[0] + "-multicut.svg"
        copy2(file, tempfile)

        # Pipe the file through svg.c
        p = Popen(
            "cat " + tempfile + " | svg2bezier > /tmp/out.txt",
            shell=True,
            stdout=PIPE,
            stderr=PIPE)
        f = p.communicate()[0]

        fh = open("/tmp/out.txt", 'rb')

        ##########################
        #
        # Main program starts here
        #
        ##########################

        RADIUS = self.options.diameter / 2

        # Calculate the segment size based on including the overlap each end.
        # This makes the segments smaller, but prevents the overlap falling
        # outside of the focal range.
        SEGMENTS = int(math.ceil((math.pi * self.options.diameter)
                                 / (chordArcLength(RADIUS, SAGITTA) -
                                    2 * ROTARY_OVERLAP)))

        SECTOR = 360.0 / SEGMENTS  # Needed in degrees for lmos
        SECTOR_WIDTH = (math.pi * self.options.diameter) / SEGMENTS
        # RINGWIDTH = self.options.width
        FOCAL_ADJUSTMENT = 0

        id = "error"
        geo = None
        group = None
        collayers = []
        lastx = None
        lasty = None
        freq = self.options.frequency

        # The main XML object
        xml = blastpy.BpXML()

        if self.options.rotary == 1:
            if self.options.ring_type == "concave":
                convex = False
                print >> sys.stderr, "Jig angle: " + \
                    str(xml.axialAngle(self.options.diameter / 2,
                                       self.options.width, self.options.override))
            else:
                convex = True
            xml.setCylinder(
                self.options.diameter / 2,
                self.options.width,
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
                    if self.options.rotary is True:
                        if self.options.ring_type == "concave":
                            rotaryLayerHeight = 77 - \
                                RADIUS + FOCAL_ADJUSTMENT
                        else:
                            rotaryLayerHeight = 77 + \
                                RADIUS + FOCAL_ADJUSTMENT
                        if rotaryLayerHeight < 45:
                            rotaryLayerHeight = 120
                        # print >> sys.stderr, "Layer height: " + \
                        #     str(rotaryLayerHeight)
                        addColourLayer(xml, colour, rotaryLayerHeight)
                    else:
                        addColourLayer(xml, colour, self.options.height)
                    # print >> sys.stderr, "Adding Qp set bp_" + colour
                    if self.options.mode != "view":
                        xml.addQpSet(
                            "bp_" + colour,
                            self.options.current,
                            self.options.speed,
                            freq)
                    freq += self.options.freqstep
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
                f = StringIO(line)
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

        if self.options.mode == "engraveclean":
            bp.init(997)
            bp.upload(2, xml.xml())
            bp.layerSetLaserable(999, "RofinStandard", 0)
            # bp.stopMarking(998)
            bp.setDimension(992, "GalvoGroupShadow101", SECTOR_WIDTH, 120)
            bp.setPosValues(991, "GalvoGroupShadow101", 0, 0, 0)
            bp.startMarking(3)

        if self.options.mode == "save":
            blast = blastpy.t_Blastpit()
            if blastpy.bp_connectToServer( blast, self.options.server, "inkscape", 3000) != 0:
                print >> sys.stderr, "Can't connect"
                sys.exit()
            blastpy.bp_sendCommandAndWait(blast, 11, "lmos", blastpy.BpCommand.kClearQpSets, 2000)
            xml.setCommand(blastpy.BpCommand.kAddQpSet, 12)
            retval = blastpy.bp_sendMessageAndWait(blast, 12, "lmos", xml.xml(), 30000)
            xml.setCommand(blastpy.BpCommand.kImportXML, 13)
            retval = blastpy.bp_sendMessageAndWait(blast, 13, "lmos", xml.xml(), 30000)
            if retval == -1:
                print >> sys.stderr, "Can't send"
                sys.exit()
            # print >> sys.stderr, xml.xml()

            blastpy.bp_sendMessageAndWait(blast, 14, "lmos", "<command id=\"14\" layer=\"RofinStandard\" laserable=\"0\">" + str(blastpy.BpCommand.kLayerSetLaserable) + "</command>", 2000)
            blastpy.bp_sendMessageAndWait(blast, 15, "lmos", "<command id=\"15\" layer=\"RofinStandard\" height=\"120\">" + str(blastpy.BpCommand.kLayerSetHeight) + "</command>", 2000)
            blastpy.bp_sendMessageAndWait(blast, 16, "lmos", "<command id=\"15\" layer=\"RofinBackground\" height=\"120\">" + str(blastpy.BpCommand.kLayerSetHeight) + "</command>", 2000)
            # bp.layerSetLaserable(995, "RofinStandard", 0)
            # bp.layerSetHeight(996, "RofinStandard", 120)
            # bp.layerSetHeight(997, "RofinBackground", 120)
            id = 17
            for layer in layers:
                blastpy.bp_sendMessageAndWait(blast, id, "lmos", "<command id=\"" + str(id) + "\" layer=\"" + str(layer[0]) + "\" exportable=\"1\">" + str(blastpy.BpCommand.kLayerSetExportable) + "</command>", 2000)
                id = id + 1
                blastpy.bp_sendMessageAndWait(blast, id, "lmos", "<command id=\"" + str(id) + "\" layer=\"" + str(layer[0]) + "\" height=\"" + str(layer[1]) + "\">" + str(blastpy.BpCommand.kLayerSetHeight) + "</command>", 2000)
                id = id + 1
            #     bp.layerSetHeight(999, layer[0], layer[1])
            blastpy.bp_sendMessageAndWait(blast, 14, "lmos", "<command id=\"14\" filename=\"C:\\Rofin\\VisualLaserMarker\\MarkingFiles\\inkscape_export.VLM\">" + str(blastpy.BpCommand.kSaveVLM) + "</command>", 2000)
            # bp.saveVLM(
            #     3, "C:\\Rofin\\VisualLaserMarker\\MarkingFiles\\inkscape_export.VLM")
            # print >> sys.stderr, layers
            blastpy.bp_disconnectFromServer(blast)

        if self.options.mode == "position":
            blast = blastpy.t_Blastpit()
            if blastpy.bp_connectToServer( blast, self.options.server, "inkscape", 1000) != 0:
                print >> sys.stderr, "Can't connect"
                sys.exit()

        if self.options.mode == "view":
            blast = blastpy.t_Blastpit()
            if blastpy.bp_connectToServer( blast, self.options.server, "inkscape", 1000) != 0:
                print >> sys.stderr, "Can't connect"
            else:
                xml.setCommand(6, 12)
                # blastpy.bp_sendCommand(blast, "lmos", 4)
                if blastpy.bp_sendMessage(blast, "lmos", xml.xml()) != 0:
                    print >> sys.stderr, "Can't send"
                # blastpy.bp_waitForAck(blast, 123)
                time.sleep(1)
                blastpy.bp_disconnectFromServer(blast)
                # print >> sys.stderr, "finished"


if __name__ == '__main__':
    e = Laser()
    e.affect()


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
