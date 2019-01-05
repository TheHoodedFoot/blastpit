#!/usr/bin/env python2

import time

import sys
from os.path import expanduser
sys.path.append(expanduser("~") + "/usr/src/blastpit/src/blastpy")
import blastpy

# Connect to server
bp = blastpy.Blastpit( "10.53.69.163", 1030 )
# bp = blastpy.Blastpit( "10.47.1.231", 1030 )
# bp.init(998)
# time.sleep(1)
bp.moveW(999, 0);
time.sleep(3)
bp.moveW(999, 180);
# time.sleep(2)

