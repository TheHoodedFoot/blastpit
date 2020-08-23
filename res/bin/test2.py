#!/usr/bin/env python3

import blastpit
import time

# The main XML object
xml = blastpit.BpXML()
#xml.setCylinder(20, 20)

# Add layers
xml.addLayer("Orange", "127,64,0", 100)
xml.addLayer("Green", "0,127,0", 100)
xml.addLayer("Purple", "127,0,127", 100)
xml.addLayer("Blue", "32,32,127", 100)
xml.addLayer("Preview", "0,0,127", 1)
#xml.layerSetLaserable( "Preview", 0 )

# Add Preview Line
p = blastpit.BpPolyline("PREVIEW01", 0, 1)
p.setLayer("Preview")
p.line(1, 1)
xml.addPolyline(p)

# Add polylines
p = blastpit.BpPolyline("ARC01", 62, 60)
p.setLayer("Purple")
p.arc(60, 62, 58, 60)
p.close()
p.setLp("Cleaning")
# xml.addPolyline(p)

p = blastpit.BpPolyline("ARC02", 10, 0)
p.setLayer("Blue")
p.arcCentre(0, 0, 0, 10)
p.close()
# xml.addPolyline(p)

p = blastpit.BpPolyline("POLYLINE0001", 25, 25)
p.setLayer("Orange")
p.line(25, 35)
p.line(35, 35)
#p.line( 35, 25 )
p.close()
# xml.addPolyline(p)

p = blastpit.BpPolyline("POLYLINE0002", 65, 70)
p.setLayer("Green")
p.line(70, 65)
p.line(70, 55)
p.line(65, 50)
p.line(55, 50)
p.line(50, 55)
p.line(50, 65)
p.arc(51.464, 68.536, 55, 70)
p.close()
p.setHatch("Two_Pass")
p.setLp("Silver")
# xml.addPolyline(p)

# Connect to server
#bp = blastpit.Blastpit( "10.47.1.239", 1030 )
bp = blastpit.Blastpit("10.53.69.107", 1030)
bp.init()
time.sleep(0.25)
# bp.reference()
time.sleep(5)
bp.upload(xml.xml())
time.sleep(0.25)
#bp.setDimension("GalvoGroupShadow101", 40, 120)
#bp.setPosValues("GalvoGroupShadow101", 55, 0, 0)
# time.sleep(0.25)
# for i in range(10, 110):
#     time.sleep(0.1)
#     bp.setPosValues( "ARC01", i, 60, i )
#     bp.startPosHelp("ARC01")

# bp.startPosHelp("ARC01")
# time.sleep(2)
# bp.stopPosHelp()
# bp.startMarking()

bp.layerSetHeight("Orange", 70)
# time.sleep(0.25)
# #bp.setPosValues( "ARC01", 60, 80, 45 )
time.sleep(0.25)

# #time.sleep(10)
# bp.readIOBit("OpenDoor")
# time.sleep(1)
bp.saveVLM("C:\\Users\\Rofin\\Desktop\\blastpit_save.VLM")
# bp.init()
# time.sleep(1)
# bp.startMarking()
# time.sleep(1)
# bp.stopPosHelp()
# time.sleep(1)

#bp.writeIOBit("OpenDoor", 0)
#bp.writeIOBit("CloseDoor", 1)
#bp.writeIOBit("Light", 1)

# bp.cancelJob()
time.sleep(1)
# bp.termMachine()
# time.sleep(0.25)
