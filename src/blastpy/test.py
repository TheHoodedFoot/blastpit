#!/usr/bin/env python3

import time
import blastpy as bpy

xml = bpy.BpXML()

ly = xml.addLayer("mylayer", 240, 200, 80, 10)

# pline = bpy.BpPolyline("pline01", 50, 50)
# pline.setLayer("mylayer")
# pline.line(50, 70)
# pline.line(70,70)
# pline.line(70,50)
# pline.close()

# xml.addPolyline(pline)

#bp = bpy.Blastpit("10.53.69.11", 1030)
bp = bpy.Blastpit("localhost", 1030)
# bp = bpy.Blastpit("10.47.1.239", 1030)
#bp = bpy.Blastpit("10.53.69.163", 1030)

bp.init(1)
time.sleep(0.1)
bp.selfTest(2)
# bp.upload(2, xml.xml())
# time.sleep(0.1)
# bp.setPosValues(3, "GalvoGroupShadow101", 10, 0, 0)
# time.sleep(0.1)
# bp.setDimension(4, "GalvoGroupShadow101", 30, 120)
# time.sleep(0.1)
# bp.saveVLM(5, "C:\\Users\\Rofin\\Desktop\\test_save_blastpit.VLM")
#time.sleep(0.1)
#bp.finish(6)
