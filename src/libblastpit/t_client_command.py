#!/usr/bin/env python3

import os.path
import sys

# Configuration variables
GIT_HIDDEN_CONFIG_DIR = os.getcwd() + "/.git/untracked"
DEFAULT_CONFIG_DIR = os.getcwd() + "/res/cfg"

if os.path.isfile(GIT_HIDDEN_CONFIG_DIR + "/myconfig.py"):
    # Use the existing custom config
    sys.path.append(GIT_HIDDEN_CONFIG_DIR)
    import myconfig
elif os.path.isfile(DEFAULT_CONFIG_DIR + "/myconfig.py"):
    # Use the default config
    sys.path.append(DEFAULT_CONFIG_DIR)
    import myconfig
else:
    print("The blastpit python config (myconfig.py) is not available.")
    exit(1)

if not os.path.isfile(os.getcwd() + "/build/_blastpy.so"):
    print("The blastpit library is not available.")
    exit(1)
sys.path.append(os.getcwd() + "/build")

import blastpy as bp
from myconfig import WS_TIMEOUT_SHORT

# The main Blastpit object
pit = bp.blastpitNew()

# bp.connectToServer(pit, "localhost:8080", BP_NET_TIMEOUT)

# Disabled due to segfault
# bp.sendCommand(pit, 123, bp.kStatus)

# result = bp.getReply(pit, 123)
# print("Result of getReply is %s" % result)

# bp.disconnectFromServer(pit)

bp.blastpitDelete(pit)
