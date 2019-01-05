#!/usr/bin/env python2

import sys
import time

from os.path import expanduser
sys.path.append(expanduser("~") + "/usr/src/blastpit/src/blastpy")
import blastpy

# The main XML object
xml = blastpy.BpXML()

# Connect to server
bp = blastpy.Blastpit( "localhost", 1030 )
bp.init(1)
time.sleep(0.25)
bp.closeDoor(2)
# time.sleep(5)
#bp.reference(3)
time.sleep(3)
bp.terminate(4)