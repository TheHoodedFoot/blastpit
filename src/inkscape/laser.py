#!/usr/bin/env python2

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
import re
import csv
from StringIO import StringIO
import inkex
import os

# For duplicating and processing the SVG file
from subprocess import Popen, PIPE
from shutil import copy2

# Needed until blastpy is installed system-wide
import sys
from os.path import expanduser
sys.path.append(expanduser("~") + "/usr/src/blastpit/src/blastpy")

import blastpy

# Laser constants
SAGITTA = 1.0  # Focal range
ROTARY_Z_LEVEL = 77.0  # Height from table to centre of rotation
ROTARY_OVERLAP = 2.0  # Shadow overlap


# Math functions

# https://en.wikipedia.org/wiki/Sagitta_(geometry)
# https://en.wikipedia.org/wiki/Circular_segment

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
            bp = blastpy.Blastpit(self.options.server, self.options.port)
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

        bp = self.setupServer()
        if bp is None:
            print >> sys.stderr, "Could not connect to server"
            return

        file = self.args[-1]
        tempfile = os.path.splitext(file)[0] + "-multicut.svg"
        copy2(file, tempfile)

        ###
        # Use Regex to get the layer names and laser parameters
        ###

        if self.options.flatten == 1:

            commandstring = "inkscape "

            if len(self.selected) > 0:
                p = Popen(
                    'inkscape --query-all ' +
                    file,
                    shell=True,
                    stdout=PIPE,
                    stderr=PIPE)
                err = p.stderr
                f = p.communicate()[0]
                lines = csv.reader(f.split(os.linesep))
                err.close()

                documentobjects = []
                for line in lines:
                    if len(line) > 0:
                        documentobjects.append(line[0])
                documentobjects.reverse()

                selecteditems = self.selected

                if len(selecteditems) > 0:
                    for o in selecteditems:
                        commandstring = commandstring + "--select=" + o + " "
            else:
                commandstring = commandstring + "--verb=EditSelectAll "

            commandstring += "--verb=ObjectToPath --verb=EditUnlinkClone " + \
                "--verb=EditInvert --verb=EditDelete " + \
                "--verb=FileSave --verb=FileQuit "
            commandstring += tempfile
            # print >> sys.stderr, commandstring
            p = Popen(
                commandstring,
                shell=True,
                stdout=PIPE,
                stderr=PIPE)
            err = p.stderr
            f = p.communicate()[0]
            err.close()

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
        SEGMENTS = int(math.ceil((math.pi * self.options.diameter) /
                                 (chordArcLength(RADIUS, SAGITTA) -
                                  2 * ROTARY_OVERLAP)))

        SECTOR = 360.0 / SEGMENTS  # Needed in degrees for lmos
        SECTOR_WIDTH = (math.pi * self.options.diameter) / SEGMENTS
        RINGWIDTH = self.options.width
        FOCAL_ADJUSTMENT = 0

        id = "error"
        geo = None
        group = None
        layers = []
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
                                       self.options.width))
            else:
                convex = True
            xml.setCylinder(
                self.options.diameter / 2,
                self.options.width,
                SECTOR,
                convex)

        for line in fh:
            if line.startswith("id:"):
                line = line[:-1]  # Remove newline
                colour = line[-6:]
                if colour not in layers:
                    layers.append(colour)
                    if self.options.rotary is True:
                        if self.options.ring_type == "concave":
                            rotaryLayerHeight = 77 - \
                                RADIUS + FOCAL_ADJUSTMENT
                        else:
                            rotaryLayerHeight = 77 + \
                                RADIUS + FOCAL_ADJUSTMENT
                        if rotaryLayerHeight < 45:
                            rotaryLayerHeight = 120
                        print >> sys.stderr, "Layer height: " + \
                            str(rotaryLayerHeight)
                        addColourLayer(xml, colour, rotaryLayerHeight)
                    else:
                        addColourLayer(xml, colour, self.options.height)
                    # print >> sys.stderr, "Adding Qp set bp_" + colour
                    bp.addQpSet(
                        999,
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
                group = xml.addGroup(
                    "g" + id, "bp_" + colour, "bp_" + colour, "Standard")
            else:
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
            bp.upload(2, xml.xml())
            bp.layerSetLaserable(999, "RofinStandard", 0)
            bp.saveVLM(
                3, "C:\\Rofin\\VisualLaserMarker\\MarkingFiles\\inkscape_export.VLM")

        if self.options.mode == "position":
            bp.init(997)
            bp.upload(2, xml.xml())
            if len(self.selected) > 0:
                for id, node in self.selected.iteritems():
                    print >> sys.stderr, id, node
                    bp.startPosHelp(999, id)
                    # bp.startPosHelp(999, "g" + id)
                    break
            else:
                bp.startPosHelp(999, "Root100")


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
