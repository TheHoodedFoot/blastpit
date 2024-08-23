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

Note: When running this script from c using popen, the output needs to be
unbuffered. This can be acheived by running in unbuffered mode (python -u)
or by adding 'flush=True' to any print statements.
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
        self.timeout = None

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
        print("Waiting for imageEnd2 signal...", flush=True)
        id = blastpy.BpWaitForSignalOrTimeout(
            self.blast,
            -16,
            (
                self.timeout
                if self.timeout is not None
                else myconfig.WS_TIMEOUT_MARKING
            ),
        )

        if id.id == -16:  # imageEnd2?
            return blastpy.kSuccess
        else:
            print("BpWaitForSignalOrTimeout result:", id.id, id.string)
            return blastpy.kFailure

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
            print("Error saving VLM file", file=sys.stdout)

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
        print("Result of kLoadVLM:", result.retval, flush=True)

        id = blastpy.BpQueueCommand(self.blast, blastpy.kLoadJob)
        blastpy.BpUploadQueuedMessages(self.blast)
        result = blastpy.BpWaitForReplyOrTimeout(
            self.blast, id.id, myconfig.WS_TIMEOUT_LONG
        )
        print("Result of kLoadJob:", result.retval, flush=True)
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
            print("Can't connect to server (%d)" % result, file=sys.stdout)
            return False

    def showWindow(self):
        id = blastpy.BpDisplayLmosWindow(self.blast, 1)
        blastpy.BpUploadQueuedMessages(self.blast)

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
            print("kGetGeoList failed", file=sys.stdout)
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
        print(errorMessage, file=sys.stdout)
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

        if self.inputfile is not None:
            if self.loadVLM(self.inputfile) == 0:
                print("Error loading VLM file", file=sys.stdout)
                return False
        else:
            print("Please specify an input filename", file=sys.stdout)
            return False

        if self.outputfile is None:
            print("Please specify an output filename", file=sys.stdout)
            return False

        if self.getShadowList() is False:
            self.bail("Failed to get shadows from file")

        print("REMEMBER TO MAKE AN EXTRA SHADOW AT THE END")
        print()
        print("DELETE ANY NON-MARKED GEOMETRY USED TO CREATE THE EXTRA SHADOW")
        print()
        print("Shadow start position:", self.shadow_start_pos)
        print("Shadow width:", self.shadow_width)
        print("Number of passes:", int(self.num_passes))
        print(flush=True)

        # Ensure that there is a spare shadow after the last shadow

        self.showWindow()

        # For each offset amount:
        for i in range(int(self.num_passes)):
            offset = self.shadow_start_pos - i * (
                self.shadow_width / self.num_passes
            )
            print(
                "({0}/{1}): {2}".format(i + 1, self.num_passes, offset),
                flush=True,
            )

            # Move the shadows left by shadow width / num passes
            j = 0
            shadows = []
            for shadow in self.shadows:
                # print("Shadow", shadow, "move to", offset + (j * self.shadow_width))
                shadows.append(offset + (j * self.shadow_width))
                j = j + 1
            self.moveShadows(shadows)

            # Save with new part filename
            self.saveVLM(self.outputfile[:-4] + "." + str(i) + ".VLM")
            # time.sleep(0.5)

        # Run the programs automatically
        # self.initVLM()
        self.cancelJob()

        # for i in range(int(self.num_passes)):
        #     filename = self.outputfile[:-4] + "." + str(i) + ".VLM"
        #     print("Executing drawing", filename)
        #     result = self.loadAndExecuteVLM(filename)
        #     if result == blastpy.kFailure:
        #         print("Error executing drawing", filename)
        #         print("Bailing out.")
        #         return
        #     self.cancelJob()

        self.termMachine()


if __name__ == "__main__":
    instance = Multipass()
    instance.run()

# vim: tabstop=8 expandtab shiftwidth=4 softtabstop=4 foldlevelstart=2
