#!/usr/bin/env python
# -*- coding: utf-8 -*-

import numpy
import math
import colorsys
'''
Copyright (C) 2009 Richard Querin, screencasters@heathenx.org
Copyright (C) 2009 heathenx, screencasters@heathenx.org
Modified from an extension distributed with JessyInk (code.google.com/p/jessyink).

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see http://www.gnu.org/licenses/.
'''

# These lines are only needed if you don't put the script directly into
# the installation directory
import sys
# Unix
sys.path.append('/usr/share/inkscape/extensions')


# Laser constants
# SAGITTA = 2.0  # Focal range
# ROTARY_Z_LEVEL = 77.0  # Height from table to centre of rotation
# ROTARY_OVERLAP = 2.0  # Shadow overlap


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


def getHeightAtAngle(height, angle):
    return math.sin(math.radians(90 - angle)) * height


def axialAngle(radius, width, overrideMin=False):
    # Find tilt (in degrees so we can round it next)
    tilt = math.degrees(math.atan(float(width) / float(radius * 2)))

    # Round up to nearest 5 degrees
    if int(round(tilt)) != tilt:
        tilt = int(round(tilt / 5) * 5)

    # The angle must be at least 20 degrees to avoid the mounts
    if tilt < 20 and overrideMin is False:
        tilt = 20

    return tilt


options = [
    ["a", "name", "string", "", "Unknown"],
    ["d", "diameter", "float", "0", "Ring diameter"],
    ["l", "holder", "string", "", "Ring holder type"],
    ["o", "offset", "float", "0", "Ring holder offset"],
    ["t", "ring_type", "string", "", "Concave or convex ring type"],
    ["w", "width", "float", "6.0", "Ring width"],
    ["s", "sagitta", "float", "1.0", "Focal range"],
    ["v", "overlap", "float", "2.0", "Shadow overlap"],
    ["f", "face", "float", "76.0", "Ring holder face offset"],
    ["x", "axisheight", "float", "77.0", "Height of rotary axis"],
    ["r", "override", "inkbool", "False", "Override the 20 degree limit"],
]

diameter = 20.0
sagitta = 0.5
overlap = 0
width = 6.0
layerHeight = 10
axisAngle = 20

RADIUS = diameter / 2
SEGMENTS = int(math.ceil((math.pi *
                          diameter) /
                         (chordArcLength(RADIUS, sagitta) -
                          2 *
                          overlap)))
SECTOR_WIDTH = (math.pi * diameter) / SEGMENTS

# Resize the document to the area of the ring
text = u'Ø' + str(round(diameter, 2)) + \
    ' x ' + str(round(width, 2)) + \
    ' Sectors ' + str(round(360.0 / SEGMENTS, 2)) + u'°' \
    ' Height ' + str((round(layerHeight, 1))) + \
    ' Angle ' + str(int(axisAngle)) + u'°'

print(text)
