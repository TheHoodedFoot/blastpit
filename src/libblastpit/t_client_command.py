#!/usr/bin/env python3

import os.path
import sys
# import unittest

# Configuration variables
if not os.path.isfile(os.path.expanduser("~")
                      + "/projects/blastpit/res/cfg/myconfig.py"):
    print("The blastpit myconfig.py file is not available.")
    exit(1)
sys.path.append(os.path.expanduser("~")
                + "/projects/blastpit/res/cfg")
import myconfig

if not os.path.isfile(os.path.expanduser("~")
                      + "/projects/blastpit/build/_blastpy.so"):
    print("The blastpit library is not available.")
    exit(1)
sys.path.append(os.path.expanduser("~")
                + "/projects/blastpit/build")

import blastpy as bp
from myconfig import WS_TEST_TIMEOUT

# The main Blastpit object
pit = bp.blastpitNew()

# bp.connectToServer(pit, "localhost:8080", BP_NET_TIMEOUT)

# Disabled due to segfault
# bp.sendCommand(pit, 123, bp.kStatus)

# result = bp.getReply(pit, 123)
# print("Result of getReply is %s" % result)

# bp.disconnectFromServer(pit)

bp.blastpitDelete(pit)
