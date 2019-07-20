#!/usr/bin/env python2

import sys
import time
from os.path import expanduser
sys.path.append(expanduser("~") + "/usr/src/blastpit/src/blastpy")
import blastpy


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
RADIUS=26.35/2
RADIUS=10
SEGMENTS = 12
SECTOR=360.0/SEGMENTS
TABLEX=120.0
RINGWIDTH=6.0
RINGY1=71.5
RINGY2=RINGY1+RINGWIDTH
CALSCALEX=0.025
CALSCALEY=0.5
CALLINES=11 # Needs to be odd
CALHEIGHT=RINGWIDTH/(CALLINES*2)

# The main XML object
xml = blastpy.BpXML()
xml.setCylinder(RADIUS, SECTOR)

# Add layers
xml.addLayer("Shadows", 77.0+RADIUS, 127, 64, 0) 
xml.addLayer("Vernier", 77.0+RADIUS, 0, 127, 0) 

# Create a line at the beginning of each shadow
for segment in range( 1, SEGMENTS ):
    p = blastpy.BpPolyline( "Shadow" + str(SEGMENTS), (TABLEX/SEGMENTS)*segment, RINGY1, "Dry Run" )
    p.setLayer("Shadows")
    p.line( (TABLEX/SEGMENTS)*segment, RINGY2 )
    xml.addPolyline(p)

    # Create the offset calibration lines
    for cal in range( 1, CALLINES + 1 ):
        p = blastpy.BpPolyline( "Vernier" + str(SEGMENTS) + "-" + str(cal), ((TABLEX/SEGMENTS)*segment)+((CALSCALEX*cal)-(6.0*CALSCALEX)), (cal*CALSCALEY)+RINGY1-(CALHEIGHT/2.0), "Dry Run" )
        p.setLayer("Vernier")
        p.line( ((TABLEX/SEGMENTS)*segment)+((CALSCALEX*cal)-(6.0*CALSCALEX)), (cal*CALSCALEY)+RINGY1+(CALHEIGHT/2.0) )
        xml.addPolyline(p)


# Connect to server
bp = blastpy.Blastpit( "SEVEN", 1030 )
# bp = blastpy.Blastpit( "10.53.69.163", 1030 )
# bp.init(1)
# bp.addQpSet(3, "Dry Run", 11, 567, 34567)
# print(xml.xml())
# bp.upload(2, xml.xml())
# time.sleep(1)
# bp.setPosValues(99, "GalvoGroupShadow101", 55, 0, 0)
bp.selfTest(4)
# bp.layerSetHeight(6, "RofinStandard", "120")
# bp.layerSetHeight(6, "RofinBackground", "120")
# bp.layerSetHeight(6, "Vernier", str(77.0+RADIUS))
# bp.layerSetHeight(6, "Shadows", str(77.0+RADIUS))

#bp.saveVLM(3, "C:\\Users\\Rofin\\Desktop\\blastpit_save.VLM")



