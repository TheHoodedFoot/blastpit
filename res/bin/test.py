#!/usr/bin/env python2

import blastpy
import sys
import time
from os.path import expanduser
sys.path.append(expanduser("~") + "/usr/src/blastpit/src/blastpy")

# The main XML object
xml = blastpy.BpXML()
xml.setCylinder(999, 20, 20)

# Add layers
xml.addLayer("Orange", 120, 127, 64, 0)
xml.addLayer("Green", 120, 0, 127, 0)
xml.addLayer("Purple", 120, 127, 0, 127)
xml.addLayer("Blue", 120, 32, 32, 127)
xml.addLayer("Preview", 120, 0, 0, 127)
# xml.layerSetLaserable( "Preview", 0 )

# Add Preview Line
p = blastpy.BpPolyline("PREVIEW01", 0, 1, "Standard")
p.setLayer("Preview")
p.line(1, 1)
xml.addPolyline(p)

# Add polylines
p = blastpy.BpPolyline("ARC01", 22, 60, "Standard")
p.setLayer("Purple")
p.arc(20, 62, 18, 60)
p.close()
p.setHp("Two_Pass")
p.setLp("Blastpit_LP_01")
xml.addPolyline(p)

p = blastpy.BpPolyline("ARC02", 10, 0, "Standard")
p.setLayer("Blue")
p.arcCentre(0, 0, 0, 10)
p.close()
p.setHp("Two_Pass")
p.setLp("Blastpit_LP_01")
xml.addPolyline(p)

p = blastpy.BpPolyline("POLYLINE0001", 25, 25, "Standard")
p.setLayer("Orange")
p.line(25, 35)
p.line(35, 35)
p.line(35, 25)
p.close()
p.setHp("Two_Pass")
p.setLp("Blastpit_LP_01")
xml.addPolyline(p)

p = blastpy.BpPolyline("POLYLINE0002", 65, 70, "Standard")
p.setLayer("Green")
p.line(70, 65)
p.line(70, 55)
p.line(65, 50)
p.line(55, 50)
p.line(50, 55)
p.line(50, 65)
p.arc(51.464, 68.536, 55, 70)
p.close()
p.setHp("Two_Pass")
p.setLp("MyQpSet")
xml.addPolyline(p)

# Connect to server
# bp = blastpy.Blastpit( "10.47.1.90", 1030 )
# bp = blastpy.Blastpit( "10.47.1.231", 1030 )
bp = blastpy.Blastpit("Seven", 1030)
# bp.readIOBit("DoorClosed")
# for i in range(0, 1):
#     bp.readByte(str(blastpy.InPortConstants.Port1C_In), "0")
#     time.sleep(1)
#bp.readIOBit(99, "DoorOpen")
# time.sleep(0.25)
# bp.loadVLM(4, "C:\\Users\\User\\Desktop\\Francessa_and_Freddy.VLM")
# bp.loadVLM(4, "C:\\Users\\User\\Desktop\\VLM_Shadows_12.VLM")
# bp.addQpSet(3, "MyQpSet", 11, 567, 34567)
bp.upload(2, xml.xml())
time.sleep(0.5)
bp.layerSetLaserable(4, "RofinStandard", 0)
time.sleep(5)
bp.layerSetVisible(4, "Green", 0)
# bp.cancelJob()
# bp.termMachine()
# time.sleep(0.25)

# bp.init(1)
# bp.selfTest(999)
