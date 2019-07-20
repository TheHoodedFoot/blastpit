#!/usr/bin/env python2

import sys
import time
import math
from os.path import expanduser
sys.path.append(expanduser("~") + "/usr/src/blastpit/src/blastpy")
import blastpy


def main():
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
    RADIUS = 10
    SEGMENTS = 6
    SECTOR = 360.0 / SEGMENTS
    RINGWIDTH = 6.0
    RINGY1 = 71.5
    RINGY2 = RINGY1 + RINGWIDTH
    CALSCALEX = 0.025
    CALSCALEY = 0.5
    CALLINES = 3  # Needs to be odd
    CALHEIGHT = RINGWIDTH / (CALLINES * 2)
    CIRCUMFERENCE = 2 * math.pi * RADIUS
    SEGLEN = CIRCUMFERENCE / SEGMENTS

    # Until we have a proper packet numbering system
    id = 999

    # The main XML object
    xml = blastpy.BpXML()
    xml.setCylinder(RADIUS, 6, SECTOR)

    for segment in range(1, SEGMENTS + 1):
        # Add one layer for each sector
        suffix = "{:02d}".format(segment)
        xml.addLayer("Shadow" + suffix, 120.0, 127, 64, 0)
        xml.addLayer("Vernier" + suffix, 120.0, 0, 127, 0)
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
        for cal in range(1, CALLINES + 1):
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
    bp = blastpy.Blastpit( "Seven", 1030 )
    # bp = blastpy.Blastpit( "10.53.69.163", 1030 )
    # bp.init(1)

    # Add QP Sets
    bp.addQpSet(3, "bp_Calibration", 11, 567, 34567)

    # Upload XML
    print(xml.xml())
    bp.upload(2, xml.xml())
    # time.sleep(1)

    # Fix layer heights
    bp.layerSetHeight(6, "RofinStandard", "120")
    bp.layerSetHeight(6, "RofinBackground", "120")

    # Cycle through layers
    for layer in range(1, SEGMENTS + 1):
        # raw_input("Press Enter to continue...")
        # Enable this layer and disable all others
        for i in range(1, SEGMENTS + 1):
            suffix = "{:02d}".format(i)
            if layer == i:
                bp.layerSetLaserable(id, "Shadow" + suffix, "1")
                bp.layerSetLaserable(id, "Vernier" + suffix, "1")
                bp.layerSetVisible(id, "Shadow" + suffix, "1")
                bp.layerSetVisible(id, "Vernier" + suffix, "1")
            else:
                bp.layerSetLaserable(id, "Shadow" + suffix, "0")
                bp.layerSetLaserable(id, "Vernier" + suffix, "0")
                bp.layerSetVisible(id, "Shadow" + suffix, "0")
                bp.layerSetVisible(id, "Vernier" + suffix, "0")
        # Move the shadow to cover this layer
        bp.setPosValues(id, "GalvoGroupShadow101", (layer - 1) * SEGLEN, 0, 0)
        bp.setDimension(id, "GalvoGroupShadow101", SEGLEN, 120)
        # Mark
        # bp.startPosHelp(id, "GalvoGroupShadow101")
        time.sleep(3)
        # bp.stopPosHelp(id)
        # bp.cancelJob(998)
        time.sleep(1)

    bp.terminate(997)
    # bp.saveVLM(3, "C:\\Users\\Rofin\\Desktop\\blastpit_save.VLM")
    # bp.finish(996)

if __name__ == "__main__":
    main()
