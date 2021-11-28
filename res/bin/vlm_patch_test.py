#!/usr/bin/env python3

import sys
from os.path import expanduser

sys.path.append(expanduser("~") + "/projects/blastpit/build")
import blastpy

# Get our machine-specific constants
sys.path.append(expanduser("~") + "/projects/blastpit/res/cfg")
import myconfig

from ctypes import *

blast = blastpy.blastpitNew()
result = blastpy.connectToServer(
    blast, "ws://localhost:8000", myconfig.WS_TIMEOUT_SHORT
)
if result != blastpy.kSuccess:
    print("Can't connect to server (%d)" % result, file=sys.stderr)
    sys.exit()


id = blastpy.BpQueueCommandArgs(
    blast,
    blastpy.kPatchFlexibleShadows,
    "filename",
    "C:\\Rofin\\VisualLaserMarker\\MarkingFiles\\inkscape_export.VLM",
    None,
    None,
    None,
    None,
    None,
    None,
    None,
)
blastpy.BpUploadQueuedMessages(blast)
blastpy.BpWaitForReplyOrTimeout(blast, id.id, myconfig.WS_TIMEOUT_SHORT)
if id.retval != blastpy.kSuccess:
    print("Error patching shadows", file=sys.stderr)
