#!/usr/bin/env python2

import blastpy
import sys
import time
import math
from os.path import expanduser
sys.path.append(expanduser("~") + "/usr/src/blastpit/src/blastpy")


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


# GenerateCalibrationLines( ring_radius, ring_width, sagitta )
# arcLength = arcsin *****
# numArcs = circumference / arcLength

# Calculate the number of shadows

# Calculate the chord that has a maximum height
# within the focal depth

# Divide the circle up into

# Resize the shadow slightly wider than needed

# Draw a straight line at the start of the shadow,
# and offset lines covering both sides of the line
# that will be the start of the next shadow
#
# Also draw identifying text/QR code


# Constants for this test
SAGITTA = 0.5  # Focal range
RADIUS = 8.5
OVERLAP = 2.0  # Segments must overlap for calibration lines
Z_LEVEL = 77.0

# Focal point is inside stock by half of sagitta
Z_HEIGHT = Z_LEVEL + RADIUS - (SAGITTA / 2.0)

# Calculate the segment size based on including the overlap each end.
# This makes the segments smaller, but prevents the overlap falling
# outside of the focal range.
SEGMENTS = int(math.ceil((2 * math.pi * RADIUS) /
                         (chordArcLength(RADIUS, SAGITTA) - 2 * OVERLAP)))

SECTOR = 360.0 / SEGMENTS  # Needed in degrees for lmos
RINGWIDTH = 6.0
RINGY1 = 71.5
RINGY2 = RINGY1 + RINGWIDTH
CALSCALEX = 0.025
CALSCALEY = 0.5
CAL_LINES_X = 5  # Needs to be odd
CAL_LINES_Y = 5
CALHEIGHT = RINGWIDTH / (CAL_LINES_Y * 2)
CIRCUMFERENCE = 2 * math.pi * RADIUS
SEGLEN = CIRCUMFERENCE / SEGMENTS + OVERLAP * 2

# Until we have a proper packet numbering system
id = 999

# The main XML object
xml = blastpy.BpXML()
xml.setCylinder(id, RADIUS, SECTOR)
id = id + 1

for segment in range(1, SEGMENTS + 1):

    # Add one layer for each sector
    suffix = "{:02d}".format(segment)
    xml.addLayer("Shadow" + suffix, Z_HEIGHT, 127, 64, 0)
    xml.addLayer("Vernier" + suffix, Z_HEIGHT, 0, 127, 0)

    # Add this sector to the shadow table
    #
    # Shadow table format: [ [ x1, w, x2, x3 ] ]
    #
    # x1: x position of lmos galvo shadow for this sector
    # w:  width of lmos galvo shadow for this sector
    # x2: start x of SVG covered by this shadow
    # x3: end x of SVG covered by this shadow

    shad = [[0, 10, 0, 5]]

    # Create a line at the beginning of each shadow
    p = blastpy.BpPolyline(
        "Shad" + suffix,
        (CIRCUMFERENCE / SEGMENTS) * segment,
        RINGY1,
        "bp_Calibration")
    p.line((CIRCUMFERENCE / SEGMENTS) * segment, RINGY2)
    p.setLayer("Shadow" + suffix)
    xml.addPolyline(p)

    # Create the offset calibration lines
    for cal in range(1, CAL_LINES_Y + 1):
        p = blastpy.BpPolyline("Vern" +
                               suffix +
                               "-" +
                               str(cal), ((CIRCUMFERENCE /
                                           SEGMENTS) *
                                          segment) +
                               ((CALSCALEX *
                                 cal) -
                                (6.0 *
                                 CALSCALEX)), (cal *
                                               CALSCALEY) +
                               RINGY1 -
                               (CALHEIGHT /
                                   2.0), "bp_Calibration")
        p.line(((CIRCUMFERENCE / SEGMENTS) * segment) + ((CALSCALEX * cal) -
                                                         (6.0 * CALSCALEX)), (cal * CALSCALEY) + RINGY1 + (CALHEIGHT / 2.0))
        p.setLayer("Vernier" + suffix)
        xml.addPolyline(p)

# Connect to server
# bp = blastpy.Blastpit( "10.47.1.90", 1030 )
bp = blastpy.Blastpit("SEVEN", 1030)
# bp = blastpy.Blastpit( "10.53.69.163", 1030 )
# bp.init(id)
# id = id + 1

# Add QP Sets
bp.addQpSet(id, "bp_Calibration", 11, 567, 34567)
id = id + 1

# Upload XML
print(xml.xml())
bp.upload(id, xml.xml())
id = id + 1
# time.sleep(1)

# Fix layer heights
bp.layerSetHeight(id, "RofinStandard", "120")
id = id + 1
bp.layerSetHeight(id, "RofinBackground", "120")
id = id + 1

# Cycle through layers
for layer in range(1, SEGMENTS + 1):
    # raw_input("Press Enter to continue...")
    # Enable this layer and disable all others
    for i in range(1, SEGMENTS + 1):
        suffix = "{:02d}".format(i)
        if layer == i:
            bp.layerSetLaserable(id, "Shadow" + suffix, "1")
            id = id + 1
            bp.layerSetLaserable(id, "Vernier" + suffix, "1")
            id = id + 1
            bp.layerSetVisible(id, "Shadow" + suffix, "1")
            id = id + 1
            bp.layerSetVisible(id, "Vernier" + suffix, "1")
            id = id + 1
        else:
            bp.layerSetLaserable(id, "Shadow" + suffix, "0")
            id = id + 1
            bp.layerSetLaserable(id, "Vernier" + suffix, "0")
            id = id + 1
            bp.layerSetVisible(id, "Shadow" + suffix, "0")
            id = id + 1
            bp.layerSetVisible(id, "Vernier" + suffix, "0")
            id = id + 1
    # Move the shadow to cover this layer
    bp.setPosValues(id, "GalvoGroupShadow101", (layer - 1) * SEGLEN, 0, 0)
    id = id + 1
    bp.setDimension(id, "GalvoGroupShadow101", SEGLEN, 120)
    id = id + 1
    # Mark
    # bp.startMarking(id)
    time.sleep(0.5)

bp.saveVLM(id, "C:\\Users\\Rofin\\Desktop\\blastpit_save.VLM")
