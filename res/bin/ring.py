#!/usr/bin/env python2

import blastpy
import sys
import time
import math

import os.path
if not os.path.isfile(os.path.expanduser("~")
                      + "/usr/src/blastpit/src/libbp/_blastpy.so"):
    print("The blastpit library is not available.")
    exit(1)

sys.path.append(os.path.expanduser("~")
                + "/usr/src/blastpit/src/libbp")


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
    RADIUS = 26.35 / 2
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
    id = 1

    # The main XML object
    xml = blastpy.BpXML()
    xml.setCylinder(RADIUS, RINGWIDTH, SECTOR)

    for segment in range(1, SEGMENTS + 1):
        # Add one layer for each sector
        suffix = "{:02d}".format(segment)
        xml.addLayer("Shadow" + suffix, 77.0 + RADIUS, 127, 64, 0)
        xml.addLayer("Vernier" + suffix, 77.0 + RADIUS, 0, 127, 0)
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
            p.line(((CIRCUMFERENCE / SEGMENTS) * segment) + ((CALSCALEX * cal) - \
                   (6.0 * CALSCALEX)), (cal * CALSCALEY) + RINGY1 + (CALHEIGHT / 2.0))
            p.setLayer("Vernier" + suffix)
            xml.addPolyline(p)

    # Connect to server
    bp = blastpy.t_Blastpit()
    print("Connecting to server...")
    blastpy.bp_connectToServer(bp, "rfbevan.co.uk", "jupyter", 4000)
    print("Clearing QPsets...")
    blastpy.bp_sendCommandAndWait(
        bp, id, "lmos", blastpy.BpCommand.kClearQpSets, 2000)
    id = id + 1

    # bp.init(1)

    # Add QP Sets
    print("Uploading QPsets...")
    blastpy.bp_sendMessageAndWait(
        bp,
        id,
        "lmos",
        "<command id=\"" +
        str(id) +
        "\"><qpset name=\"bp_calibration\" current=\"10.0\" speed=\"500\" frequency=\"30000\"/>2</command>",
        3000)
    id = id + 1

    # Upload XML
    xml.setCommand(6, id)
    print(xml.xml())
    blastpy.bp_sendMessageAndWait(bp, id, "lmos", xml.xml(), 30000)

    # Cycle through layers
    # for layer in range(1, SEGMENTS + 1):
    #     # raw_input("Press Enter to continue...")
    #     # Enable this layer and disable all others
    #     for i in range(1, SEGMENTS + 1):
    #         suffix = "{:02d}".format(i)
    #         if layer == i:
    #             bp.layerSetLaserable(id, "Shadow" + suffix, "1")
    #             bp.layerSetLaserable(id, "Vernier" + suffix, "1")
    #             bp.layerSetVisible(id, "Shadow" + suffix, "1")
    #             bp.layerSetVisible(id, "Vernier" + suffix, "1")
    #         else:
    #             bp.layerSetLaserable(id, "Shadow" + suffix, "0")
    #             bp.layerSetLaserable(id, "Vernier" + suffix, "0")
    #             bp.layerSetVisible(id, "Shadow" + suffix, "0")
    #             bp.layerSetVisible(id, "Vernier" + suffix, "0")
    #     # Move the shadow to cover this layer
    #     bp.setPosValues(id, "GalvoGroupShadow101", (layer - 1) * SEGLEN, 0, 0)
    #     bp.setDimension(id, "GalvoGroupShadow101", SEGLEN, 120)
    #     # Mark
    #     bp.startMarking(id)
    #     time.sleep(3)
    #     bp.cancelJob(998)
    #     time.sleep(1)

    blastpy.bp_sendMessageAndWait(
        bp,
        999,
        "lmos",
        r"<command id=\"999\" filename=\"C:\Rofin\VisualLaserMarker\MarkingFiles\\ringtest_vlm_save.VLM\">18</command>",
        3000)


if __name__ == "__main__":
    main()
