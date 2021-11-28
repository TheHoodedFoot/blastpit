#!/usr/bin/env python

import sys

# import xml.dom.minidom
from io import StringIO
import csv
import re
import xml.etree.ElementTree as ET

# Needed until blastpy is installed system-wide
from os.path import expanduser

sys.path.append(expanduser("~") + "/projects/blastpit/build")
import blastpy

# Get our machine-specific constants
sys.path.append(expanduser("~") + "/projects/blastpit/res/cfg")
import myconfig


# xmlstring = open(sys.stdin, 'r').read()

# xmlfile = StringIO( sys.stdin )
# xml.dom.minidom.parse(xmlfile).toprettyxml()
# return

blast = blastpy.blastpitNew()
blastpy.connectToServer(
    blast, myconfig.WS_SERVER_REMOTE, myconfig.WS_TIMEOUT_SHORT
)
blastpy.BpDisplayLmosWindow(blast, 1)
id = blastpy.BpQueueCommandArgs(
    blast,
    blastpy.kImportXML,
    str(sys.stdin.read()),
    None,
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

# Get the list of objects
print("Obtaining objects...", file=sys.stderr)

result = blastpy.BpQueueCommand(blast, blastpy.kGetGeoList)
blastpy.BpUploadQueuedMessages(blast)
print("waiting", file=sys.stderr)
print(result.id, file=sys.stderr)
result = blastpy.BpWaitForReplyOrTimeout(
    blast, result.id, myconfig.WS_TIMEOUT_LONG
)
print("waited", file=sys.stderr)
if result.retval == blastpy.kSuccess:
    print("kGetGeoList result:", file=sys.stderr)
    marking_objects = ET.fromstring(result.string)
    semicolon_string = marking_objects.text.replace(";", "\n")
    csvfile = StringIO(semicolon_string)
    reader = csv.reader(csvfile)
    shadow_pattern = re.compile("GalvoGroupShadow.*")
    path_pattern = re.compile(".*")
    for row in reader:
        is_shadow = shadow_pattern.match(row[0])
        if is_shadow is not None:
            print(
                row[0],
                "x =",
                row[1],
                "y =",
                row[2],
                "width =",
                row[4],
                "height =",
                row[5],
                file=sys.stderr,
            )
        is_path = path_pattern.match(row[0])
        if is_path is not None:
            print(
                row[0],
                "x =",
                row[1],
                "y =",
                row[2],
                "width =",
                row[4],
                "height =",
                row[5],
                file=sys.stderr,
            )
else:
    print("kGetGeoList failed", file=sys.stderr)
    sys.exit()

blastpy.disconnectFromServer(blast)
blastpy.blastpitDelete(blast)


# vim: tabstop=8 expandtab shiftwidth=4 softtabstop=4
