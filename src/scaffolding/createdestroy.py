#!/usr/bin/env python
# coding=utf-8
#
# Copyright (C) 2022 Andrew Black, andrew@rfbevan.co.uk
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
#

"""
multipass.py: Converts a VLM file to multiple files with offset shadows
"""

# Needed until blastpy is installed system-wide
import sys
from os.path import expanduser

sys.path.append(expanduser("~") + "/projects/blastpit/build")
import blastpy

# Get our machine-specific constants
sys.path.append(expanduser("~") + "/projects/blastpit/.git/untracked")
import myconfig

from io import StringIO  # To convert string to csv file
import csv
import itertools
import re
import xml.etree.ElementTree as ET
import getopt
import time

# import math


class Multipass:
    """Automatically create multiple offset shadows"""

    def __init__(self):
        self.shadows = []
        self.moved_shadows = []
        self.inputfile = None
        self.shadow_width = 0
        self.shadow_start_pos = 1000000

    def printUsage(self):
        print(
            "Usage: multipass.py -i <inputfile> -o <outputfile> "
            "-p <passes> -s <server> -t <timeout>"
        )

    def parseArgs(self):
        try:
            opts, args = getopt.getopt(
                sys.argv[1:],
                "hi:o:p:s:t",
                ["ifile=", "ofile=", "passes=", "server=", "timeout="],
            )
        except getopt.GetoptError:
            return False
        for opt, arg in opts:
            if opt in ("-i", "--ifile"):
                self.inputfile = arg
            elif opt in ("-o", "--ofile"):
                self.outputfile = arg
            elif opt in ("-p", "--passes"):
                self.num_passes = int(arg)
            elif opt in ("-t", "--timeout"):
                self.timeout = arg
            elif opt in ("-s", "--server"):
                self.server = arg
        try:
            if self.outputfile is None:
                self.outputfile = self.inputfile
        except AttributeError:
            self.outputfile = self.inputfile
        print("self.outputfile =", self.outputfile)

    def loadAndExecuteVLM(self, filename):
        result = self.loadVLM(filename)
        if result != blastpy.kSuccess:
            return result

        # Execute the drawing
        result = self.executeVLM()
        if result != blastpy.kSuccess:
            return result

        # Return when the drawing has finished
        return

    def saveVLM(self, filename):
        id = blastpy.BpQueueCommandArgs(
            self.blast,
            blastpy.kSaveVLM,
            "filename",
            filename,
            None,
            None,
            None,
            None,
            None,
            None,
            None,
        )
        blastpy.BpUploadQueuedMessages(self.blast)
        result = blastpy.BpWaitForReplyOrTimeout(
            self.blast, id.id, myconfig.WS_TIMEOUT_LONG
        )
        if result.retval != blastpy.kSuccess:
            print("Error saving VLM file", file=sys.stderr)

    def loadVLM(self, filename):
        id = blastpy.BpQueueCommandArgs(
            self.blast,
            blastpy.kLoadVLM,
            "filename",
            filename,
            None,
            None,
            None,
            None,
            None,
            None,
            None,
        )
        blastpy.BpUploadQueuedMessages(self.blast)
        result = blastpy.BpWaitForReplyOrTimeout(
            self.blast, id.id, myconfig.WS_TIMEOUT_LONG
        )
        print("Result of kLoadVLM:", result.retval)

        id = blastpy.BpQueueCommand(self.blast, blastpy.kLoadJob)
        blastpy.BpUploadQueuedMessages(self.blast)
        result = blastpy.BpWaitForReplyOrTimeout(
            self.blast, id.id, myconfig.WS_TIMEOUT_LONG
        )
        print("Result of kLoadJob:", result.retval)
        if result.retval == 0:
            print(
                "kLoadJob has failed. This can be because the drawing is "
                "rotary but the rotary axis is not active."
            )

        return result.retval

    def initVLM(self):
        id = blastpy.BpQueueCommand(self.blast, blastpy.kInitMachine)
        blastpy.BpUploadQueuedMessages(self.blast)
        result = blastpy.BpWaitForReplyOrTimeout(
            self.blast, id.id, myconfig.WS_TIMEOUT_LONG
        )
        print("Result of kInitMachine:", result.retval)

        return result.retval

    def cancelJob(self):
        id = blastpy.BpQueueCommand(self.blast, blastpy.kCancelJob)
        blastpy.BpUploadQueuedMessages(self.blast)
        blastpy.BpWaitForReplyOrTimeout(
            self.blast, id.id, myconfig.WS_TIMEOUT_LONG
        )

    def termMachine(self):
        id = blastpy.BpQueueCommand(self.blast, blastpy.kTermMachine)
        blastpy.BpUploadQueuedMessages(self.blast)
        result = blastpy.BpWaitForReplyOrTimeout(
            self.blast, id.id, myconfig.WS_TIMEOUT_LONG
        )

        return result.retval

    def executeVLM(self):
        id = blastpy.BpQueueCommand(self.blast, blastpy.kStartMarking)
        blastpy.BpUploadQueuedMessages(self.blast)
        result = blastpy.BpWaitForReplyOrTimeout(
            self.blast, id.id, myconfig.WS_TIMEOUT_MARKING
        )
        print("Result of kStartMarking:", result.retval)

        return result.retval

    def connect(self):
        # Connect to server or bail
        self.blast = blastpy.blastpitNew()
        result = blastpy.connectToServer(
            self.blast, myconfig.WS_SERVER_REMOTE, myconfig.WS_TIMEOUT_SHORT
        )
        if result != blastpy.kSuccess:
            print("Can't connect to server (%d)" % result, file=sys.stderr)
            return False

    def getShadowList(self):
        # Get the list of objects
        result = blastpy.BpQueueCommand(self.blast, blastpy.kGetGeoList)
        blastpy.BpUploadQueuedMessages(self.blast)
        result = blastpy.BpWaitForReplyOrTimeout(
            self.blast, result.id, myconfig.WS_TIMEOUT_LONG
        )
        if result.retval == blastpy.kSuccess:
            marking_objects = ET.fromstring(result.string)
            if marking_objects is None:
                return False
            semicolon_string = marking_objects.text.replace(";", "\n")
            csvfile = StringIO(semicolon_string)
            reader = csv.reader(csvfile)
            shadow_pattern = re.compile("GalvoGroupShadow.*")

            for row in reader:
                is_shadow = shadow_pattern.match(row[0])
                if is_shadow is not None:
                    # Store the name of the shadow
                    self.shadows.append([row[0]])
                    if float(row[1]) < self.shadow_start_pos:
                        # Store the leftmost shadow x pos and width
                        self.shadow_start_pos = float(row[1])
                        self.shadow_width = float(row[4])
        else:
            print("kGetGeoList failed", file=sys.stderr)
            return False

    def moveShadows(self, newShadowPositions):
        # Takes a list of x coordinates and moves the shadows to suit
        i = 0
        for shadow in self.shadows:
            if i >= len(newShadowPositions):
                break
            id = blastpy.BpQueueCommandArgs(
                self.blast,
                blastpy.kSetPosValues,
                "x",
                str(newShadowPositions[i]),
                "y",
                "0",
                "object",
                shadow[0],
                "r",
                "0",
                None,
            )
            i = i + 1

        if i > 0:
            blastpy.BpUploadQueuedMessages(self.blast)
            result = blastpy.BpWaitForReplyOrTimeout(
                self.blast, id.id, myconfig.WS_TIMEOUT_LONG
            )
            if result.retval != blastpy.kSuccess:
                return False

    def invalidateShadows(self):
        # Move all shadows to an invalid position
        # to highlight unused shadows and prevent overlap
        for s in self.shadows:
            id = blastpy.BpQueueCommandArgs(
                self.blast,
                blastpy.kSetPosValues,
                "x",
                "150",
                "y",
                "0",
                "object",
                s[0],
                "r",
                "0",
                None,
            )
        blastpy.BpUploadQueuedMessages(self.blast)
        result = blastpy.BpWaitForReplyOrTimeout(
            self.blast, id.id, myconfig.WS_TIMEOUT_LONG
        )
        if result != blastpy.kSuccess:
            return False

    def removeDuplicates(self, listOfLists):
        listOfLists.sort()
        return list(
            listOfLists for listOfLists, _ in itertools.groupby(listOfLists)
        )

    def bail(self, errorMessage):
        print(errorMessage, file=sys.stderr)
        sys.exit(1)

    def cleanup(self):
        # Disconnect from server; Close open files
        blastpy.disconnectFromServer(self.blast)

    def getShadows(self):
        return self.shadows

    def setShadows(self, shadows):
        """Setter for the list of shadows, mostly used for testing"""
        self.shadows = shadows

    def setInputfile(self, filename):
        self.inputfile = filename

    def setOutputfile(self, filename):
        self.outputfile = filename

    def run(self):
        if self.parseArgs() is False:
            self.printUsage()
            sys.exit(1)
        if self.connect() is False:
            self.bail("Failed to connect to server")

        for i in range(10):
            id = blastpy.BpQueueCommand(self.blast, blastpy.kCreateLMOS)
            blastpy.BpUploadQueuedMessages(self.blast)
            result = blastpy.BpWaitForReplyOrTimeout(
                self.blast, id.id, myconfig.WS_TIMEOUT_SHORT
            )
            print("Result of kCreateLMOS:", result.retval)
            time.sleep(2)

            id = blastpy.BpQueueCommand(self.blast, blastpy.kShowLMOS)
            blastpy.BpUploadQueuedMessages(self.blast)
            result = blastpy.BpWaitForReplyOrTimeout(
                self.blast, id.id, myconfig.WS_TIMEOUT_SHORT
            )
            print("Result of kShowLMOS:", result.retval)
            time.sleep(2)

            id = blastpy.BpQueueCommand(self.blast, blastpy.kHideLMOS)
            blastpy.BpUploadQueuedMessages(self.blast)
            result = blastpy.BpWaitForReplyOrTimeout(
                self.blast, id.id, myconfig.WS_TIMEOUT_SHORT
            )
            print("Result of kHideLMOS:", result.retval)
            time.sleep(2)

            id = blastpy.BpQueueCommand(self.blast, blastpy.kDestroyLMOS)
            blastpy.BpUploadQueuedMessages(self.blast)
            result = blastpy.BpWaitForReplyOrTimeout(
                self.blast, id.id, myconfig.WS_TIMEOUT_SHORT
            )
            print("Result of kDestroyLMOS:", result.retval)
            time.sleep(2)


if __name__ == "__main__":
    instance = Multipass()
    instance.run()

# vim: tabstop=8 expandtab shiftwidth=4 softtabstop=4 foldlevelstart=2
