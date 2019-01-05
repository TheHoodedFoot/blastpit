#!/usr/bin/env python2

import sys
import time
from os.path import expanduser
sys.path.append(expanduser("~") + "/usr/src/blastpit/src/blastpy")
import blastpy

from StringIO import StringIO
import csv

# The main XML object
xml = blastpy.BpXML()

# Add layers
#xml.addLayer("Shadows", 77.0+RADIUS, 127, 64, 0) 
#xml.addLayer("Vernier", 77.0+RADIUS, 0, 127, 0) 


def addColourLayer( colour ):

    red =   (int(colour[-6:], 16) >> 16) & 0xff
    green = (int(colour[-6:], 16) >>  8) & 0xff
    blue =  (int(colour[-6:], 16)      ) & 0xff

    xml.addLayer(
            colour,
            120,
            red,
            green,
            blue)


# For each path, create a group
# for each line in a subpath, check if it starts where the old one ended
# if so, continue
# if not, end the subpath and start a new one

id = "error"
geo = None
group = None
layers = []
lastx = None
lasty = None
for line in sys.stdin:
    if line.startswith("id:"):
        line = line[:-1] # Remove newline
        colour = line[-6:]
        if colour not in layers:
            layers.append(colour)
            addColourLayer(colour)
        id = line
        group = xml.addGroup( id, colour, "Dry Run", "Two_Pass" )
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
                geo = blastpy.BpPolyline( id, nodes[0], nodes[1], "Dry Run", "Two_Pass" )
                geo.setLayer(id[-6:])
                lastx = nodes[0]
                lasty = nodes[1]
            # Is this a new sub path?
            if nodes[0] != lastx or nodes[1] != lasty:
                geo.close()
                xml.addPolylineG(geo, group)
                geo = blastpy.BpPolyline( id, nodes[0], nodes[1], "Dry Run", "Two_Pass" )
            geo.bezier( nodes[2], nodes[3], nodes[4], nodes[5], nodes[6], nodes[7] )
            lastx = nodes[6]
            lasty = nodes[7]
        else:
            # Line is blank
            geo.close()
            xml.addPolylineG(geo, group)
            geo = None
            group = None

# for layer in layers:
    # print( "Adding layer " + layer )
    # addColourLayer(layer)






# Connect to server
bp = blastpy.Blastpit( "10.47.1.231", 1030 )
# bp = blastpy.Blastpit( "10.53.69.163", 1030 )
# bp.init(1)
bp.addQpSet(3, "Dry Run", 11, 567, 34567)
print(xml.xml())
bp.upload(2, xml.xml())
# time.sleep(1)
# bp.setPosValues(99, "GalvoGroupShadow101", 55, 0, 0)
#bp.selfTest(4)
# bp.layerSetHeight(6, "RofinStandard", "120")
# bp.layerSetHeight(6, "RofinBackground", "120")
# bp.layerSetHeight(6, "Vernier", str(77.0+RADIUS))
# bp.layerSetHeight(6, "Shadows", str(77.0+RADIUS))

#bp.saveVLM(3, "C:\\Users\\Rofin\\Desktop\\blastpit_save.VLM")



