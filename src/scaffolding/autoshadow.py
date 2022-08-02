#!/usr/bin/env python
# coding=utf-8
#
# Copyright (C) 2021 Andrew Black, andrew@rfbevan.co.uk
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
autoshadow.py: Reads a VLM file and modifies shadow positions
"""

# Needed until blastpy is installed system-wide
import sys
import os.path
from os.path import expanduser

sys.path.append(expanduser("~") + "/projects/blastpit/build")
import blastpy

# Get our machine-specific constants
sys.path.append(expanduser("~") + "/projects/blastpit/.git/untracked")
try:
    import myconfig
except ModuleNotFoundError:
    sys.path.append(os.getcwd() + "/res/cfg")
    import myconfig

from io import StringIO  # To convert string to csv file
import csv
import itertools
import re
import xml.etree.ElementTree as ET
import getopt
import math

DEBUG = 0

#######################################################################
#                              READ THIS                              #
#######################################################################

# If autoshadow creates shadows that overlap each other and cannot be merged,
# then it should output multiple files that stagger each set of non-overlapping
# shadows

# Simple start algorithm without needing to break

# 1) Merge any overlapping geometries.

# 1) Find the largest geometry

# 2) If greater than max shadow size, remove it and create a shadow covering it

# 3) If not, look at neighbouring geometries and find max area that can be covered
#    to the left or right. Remove those geometries covered and create a shadow.

# 4) When all geometries have been processed, divide up any geometries larger
#    than the max shadow width and create shadows.


# 1) Calculate a shadow for each geometry

# 2) If the shadow is greater than the maxiumum shadow width,
#    break it at the most convenient point. Repeat if neccessary.

# 2a) For each path, discretize into points and calculate the tangent at
#     that point. Record this and create a histogram of the most popular
#     angles
#
# 2b) Draw a line from that point and calculate the length of cut that it
#     would make with the geometry, and the maximum width of the bounding box
#     of the geometries created by a cut at that point.
#
# 2c) Create a score based on the weight of the tangent and the weight
#     of the length of cut. Divide the object along that line.

# All paths are now less than or equal to the maximum shadow width

# 3) Starting from the first shadow, calculate the maximum area of shadows that
#     would be covered by a shadow of maximum width placed starting
#     at that shadow. Repeat this for every shadow. Once the maximum area
#     covered is found, merge all of the shadows within that area into a single
#     shadow and remove from the shadow list. Repeat until the list is empty.


# Move and resize shadows to suit
# print("Setting shadow position...")
# shadowWidth = 10
# blastpy.sendMessageAndWait(
#     self,
#     id,
#     "lmos",
#     "<command object=\"GalvoGroupShadow101\" x=\""
#     + str(shadowWidth) + "\" y=\"120\">"
#     + str(blastpy.kSetDimension) + "</command>",
#     WS_TIMEOUT_LONG)

# Delete extraneous shadows


# FORMAT OF LMOS RETURNED VALUES
#
# path name, x, y, width, height


class Autoshadow:
    """Automatically optimise shadow positions"""

    def __init__(self, maxwidth=1, tolerance=0):
        self.paths = []
        self.shadows = []
        self.moved_shadows = []
        self.inputfile = None
        self.maxwidth = maxwidth
        self.tolerance = tolerance

    def printUsage(self):
        print(
            "Usage: autoshadow.py -i <inputfile> -o <outputfile> -w <width> -s <server> -t <timeout>"
        )

    def parseArgs(self):
        try:
            opts, args = getopt.getopt(
                sys.argv[1:],
                "hi:o:w:s:t",
                ["ifile=", "ofile=", "width=", "server=", "timeout="],
            )
        except getopt.GetoptError:
            return False
        for opt, arg in opts:
            if opt in ("-i", "--ifile"):
                self.inputfile = arg
            elif opt in ("-o", "--ofile"):
                self.outputfile = arg
            elif opt in ("-w", "--width"):
                self.maxwidth = arg
            elif opt in ("-t", "--timeout"):
                self.timeout = arg
            elif opt in ("-s", "--server"):
                self.server = arg
        try:
            if self.outputfile is None:
                self.outputfile = self.inputfile
        except:
            self.outputfile = self.inputfile
        print("self.outputfile =", self.outputfile)

    def saveVLM(self):
        # print("Saving VLM file...")

        id = blastpy.BpQueueCommandArgs(
            self.blast,
            blastpy.kSaveVLM,
            "filename",
            self.outputfile,
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

    def connect(self):
        # Connect to server or bail
        self.blast = blastpy.blastpitNew()
        result = blastpy.connectToServer(
            self.blast, myconfig.WS_SERVER_REMOTE, myconfig.WS_TIMEOUT_SHORT
        )
        if result != blastpy.kSuccess:
            print("Can't connect to server (%d)" % result, file=sys.stderr)
            return False

    def showWindow(self):
        # Show lmos window
        blastpy.BpDisplayLmosWindow(self.blast, 1)
        blastpy.BpUploadQueuedMessages(self.blast)

    def setInputfile(self, filename):
        self.inputfile = filename

    def setOutputfile(self, filename):
        self.outputfile = filename

    def loadFile(self):
        id = blastpy.BpQueueCommandArgs(
            self.blast,
            blastpy.kLoadVLM,
            "filename",
            self.inputfile,
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
        return True if result.retval == blastpy.kSuccess else False

    def getObjectList(self):
        # Get the list of objects
        result = blastpy.BpQueueCommand(self.blast, blastpy.kGetGeoList)
        blastpy.BpUploadQueuedMessages(self.blast)
        result = blastpy.BpWaitForReplyOrTimeout(
            self.blast, result.id, myconfig.WS_TIMEOUT_LONG
        )
        if result.retval == blastpy.kSuccess:
            marking_objects = ET.fromstring(result.string)
            semicolon_string = marking_objects.text.replace(";", "\n")
            csvfile = StringIO(semicolon_string)
            reader = csv.reader(csvfile)
            shadow_pattern = re.compile("GalvoGroupShadow.*")
            rect_pattern = re.compile("rect.*")
            path_pattern = re.compile("path.*")
            circle_pattern = re.compile("circle.*")

            for row in reader:
                is_shadow = shadow_pattern.match(row[0])
                if is_shadow is not None:
                    self.shadows.append([row[0]])
                    continue
                is_path = path_pattern.match(row[0])
                is_rect = rect_pattern.match(row[0])
                is_circle = circle_pattern.match(row[0])
                if (
                    is_path is not None
                    or is_rect is not None
                    or is_circle is not None
                ):
                    self.paths.append(
                        [
                            row[0],
                            float(row[1]),
                            float(row[2]),
                            float(row[4]),
                            float(row[5]),
                        ]
                    )
                    # print("Found path", row[0], file=sys.stderr)
        else:
            print("kGetGeoList failed", file=sys.stderr)
            return False

    def mergeRawPaths(self, paths):
        # Merges any paths with the same name into one single path
        merged_paths = []
        basename = None
        x1 = 0
        x2 = 0
        pattern = re.compile(r"(.*)-(\d+)")
        for path in paths:
            match = re.search(pattern, path[0])
            if match:
                # print("Match found")
                if float(path[1]) < x1:
                    x1 = float(path[1])
                    # print("x1 moved to", x1, "by", path[0])
                if float(path[1]) + float(path[3]) > x2:
                    x2 = float(path[1]) + float(path[3])
                    # print("x2 moved to", x2, "by", path[0])
            else:
                # Dump any finished path block
                if basename is not None:
                    # print("Transferring path:", basename)
                    merged_paths.append([x1, x2 - x1])
                    basename = None
                # print("Found new base path:", path[0])
                basename = path[0]
                x1 = float(path[1])
                x2 = x1 + float(path[3])
        # print("Transferring final path:", basename)
        merged_paths.append([x1, x2 - x1])

        return merged_paths

    def getRawPathData(self):
        # TODO: Merge all paths with the same name
        raw = []
        for path in self.paths:
            raw.append([float(path[1]), float(path[3])])
        return raw

    def removeOversizePaths(self, paths):
        sizedPaths = []
        for path in paths:
            if float(path[1]) <= float(self.maxwidth):
                sizedPaths.append(path)

        return sizedPaths

    def round_up(self, number: float, decimals: int = 2):
        """Returns a value rounded up to a specific number of decimal places."""
        if not isinstance(decimals, int):
            raise TypeError("decimal places must be an integer")
        elif decimals < 0:
            raise ValueError("decimal places has to be 0 or more")
        elif decimals == 0:
            return math.ceil(number)

        factor = pow(10, decimals)
        return math.ceil((number + self.tolerance) * factor) / factor

    def round_down(self, number: float, decimals: int = 2):
        """
        Returns a value rounded down to a specific number of decimal places.
        """
        if not isinstance(decimals, int):
            raise TypeError("decimal places must be an integer")
        elif decimals < 0:
            raise ValueError("decimal places has to be 0 or more")
        elif decimals == 0:
            return math.floor(number)

        factor = pow(10, decimals)
        return math.floor((number - self.tolerance) * factor) / factor

    def roundPaths(self, paths):
        # Ensure that paths are rounded up to avoid cropping
        roundedPaths = []
        for path in paths:
            roundedPaths.append(
                [self.round_down(path[0], 2), self.round_up(path[1], 2)]
            )

        return roundedPaths

    def removeInternalPaths(self, paths):
        majorPaths = []
        for i in range(0, len(paths)):
            is_subpath = False
            for j in range(0, len(paths)):
                if i == j:
                    continue
                # if i exists within j then i should be dropped
                if (float(paths[i][0]) >= float(paths[j][0])) and (
                    float(paths[i][0]) + float(paths[i][1])
                ) <= (float(paths[j][0]) + float(paths[j][1])):
                    if paths[i] != paths[j]:
                        # print("Subpath", paths[i], "exists within", paths[j], file=sys.stderr)
                        is_subpath = True
                        break
                # else:
                #     print("Path", paths[i], "is not within", paths[j], "or is identical", file=sys.stderr)
            if is_subpath is False:
                majorPaths.append(paths[i])

        return majorPaths

    def removeOverlappingPaths(self, paths):
        try:
            nonOverlappingPaths = [paths[0]]
        except IndexError:
            return paths
        # print(paths, file=sys.stderr)
        for i in range(1, len(paths)):
            # print("Processing", i, file=sys.stderr)
            badshadow = False
            for j in range(0, len(nonOverlappingPaths)):
                # If start of j is before end of i
                # print("j:", j, paths[j], file=sys.stderr)
                # print(float(paths[j][0]), float(paths[i][0]), float(paths[i][1]), file=sys.stderr)
                if float(paths[i][0]) < (
                    float(nonOverlappingPaths[j][0])
                    + float(nonOverlappingPaths[j][1])
                ):
                    # If start position is before the end of the previous shadow, it's bad
                    badshadow = True
                    # print("Path", paths[i], "overlaps", paths[j], "(", float(paths[i][0]), "<", (float(nonOverlappingPaths[j][0]) + float(nonOverlappingPaths[j][1])), ")", file=sys.stderr)
                    break
            if badshadow is False:
                nonOverlappingPaths.append(paths[i])

        return nonOverlappingPaths

    def mergeNeighbouringPaths(self, paths):
        unduplicatePaths = self.removeDuplicates(paths)
        mergedPaths = []
        for i in range(0, len(unduplicatePaths)):
            idxBestMerge = -1
            valBestMerge = 0
            idxNextTry = -1
            for j in range(i + 1, len(unduplicatePaths)):
                width = self.totalWidth(
                    unduplicatePaths[i], unduplicatePaths[j]
                )
                if width > valBestMerge and width <= float(self.maxwidth):
                    # print("New best width of", unduplicatePaths[i], "and", unduplicatePaths[j], "is", width, file=sys.stderr)
                    # Found a new widest pair
                    idxBestMerge = i
                    valBestMerge = width
                    idxNextTry = j + 1
            if idxBestMerge > -1:
                mergedPaths.append(
                    [
                        float(unduplicatePaths[idxBestMerge][0]),
                        float(valBestMerge),
                    ]
                )
                i = idxNextTry
            else:
                mergedPaths.append(
                    [unduplicatePaths[i][0], unduplicatePaths[i][1]]
                )
                # print("Storing best path:", unduplicatePaths[idxBestMerge][0], float(valBestMerge), file=sys.stderr)

        return mergedPaths

    def calculateOptimalPaths(self, paths):

        # Sort paths by size
        sortedPaths = self.sortPathsByWidth(paths)

        if DEBUG == 1:
            print("Sorted paths:")
            print(sortedPaths)
            print()

        # REMOVAL OF OVERSIZE PATHS CURRENTLY DISABLED
        sizedPaths = sortedPaths

        # Remove paths larger than max
        # sizedPaths = self.removeOversizePaths(sortedPaths)



        # Merge all possible path combinations
        mergedPaths = self.removeDuplicates(
            self.mergeNeighbouringPaths(sizedPaths)
        )

        if DEBUG == 1:
            print("Merged neighbouring paths:", file=sys.stderr)
            print(self.sortPathsByStartPosition(mergedPaths), file=sys.stderr)
            print(file=sys.stderr)

        # Remove overlapping merged paths
        # nooverlapMergedPaths = self.removeOverlappingPaths(
        #     self.sortPathsByStartPosition(mergedPaths)
        # )
        # print("Non-overlapping merged paths:")
        # print(self.sortPathsByStartPosition(nooverlapMergedPaths))
        # print()

        # Remove paths that exist inside other paths
        majorPaths = self.roundPaths(
            self.removeInternalPaths(
                self.sortPathsByStartPosition(mergedPaths)
            )
        )

        if DEBUG == 1:
            print("Remove paths inside other paths:")
            print(self.sortPathsByStartPosition(majorPaths))
            print()

        return self.removeDuplicates(majorPaths)

    def totalWidth(self, path1, path2):
        paths = self.sortPathsByStartPosition([path1, path2])
        return float(paths[1][0]) + float(paths[1][1]) - float(paths[0][0])

    def moveShadows(self, newShadowPositions):
        i = 0
        for shadow in self.shadows:
            if i >= len(newShadowPositions):
                break
            id = blastpy.BpQueueCommandArgs(
                self.blast,
                blastpy.kSetDimension,
                "x",
                str(newShadowPositions[i][1]),
                "y",
                "120",
                "object",
                shadow[0],
                None,
                None,
                None,
            )
            id = blastpy.BpQueueCommandArgs(
                self.blast,
                blastpy.kSetPosValues,
                "x",
                str(newShadowPositions[i][0]),
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
        # Move all shadows to an invalid position to highlight unused shadows
        for s in self.shadows:
            id = blastpy.BpQueueCommandArgs(
                self.blast,
                blastpy.kSetPosValues,
                "x",
                "-10",
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

    def sortPathsByWidth(self, paths):
        def pathWidth(path):
            return float(path[1])

        paths.sort(key=pathWidth, reverse=True)
        return paths

    def sortPathsByStartPosition(self, paths):
        def pathStart(path):
            return float(path[0])

        paths.sort(key=pathStart)
        return paths

    def isPathWithinShadow(self, path, shadow):
        """Returns true if the path falls within the shadow"""

        return (shadow[0] <= path[0]) and (
            path[0] + path[1] <= shadow[0] + shadow[1]
        )

    def findPathsWithoutShadows(self, paths, shadows):
        """Return list of paths not covered by a shadow"""

        uncoveredPaths = []
        for path in paths:
            isWithinShadow = False
            for shadow in shadows:
                if self.isPathWithinShadow(path, shadow):
                    # print("Path", path, "is within shadow", shadow, file=sys.stderr)
                    isWithinShadow = True
                    break
            if isWithinShadow is False:
                uncoveredPaths.append(path)
                # print("Path", path, "has no available shadow", file=sys.stderr)

        return uncoveredPaths

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

    def getPaths(self):
        return self.paths

    def getShadows(self):
        return self.shadows

    def setPaths(self, paths):
        """Setter for the list of paths, mostly used for testing"""
        self.paths = paths

    def setShadows(self, shadows):
        """Setter for the list of shadows, mostly used for testing"""
        self.shadows = shadows

    def run(self):
        if self.parseArgs() is False:
            self.printUsage()
            sys.exit(1)
        if self.connect() is False:
            self.bail("Failed to connect to server")

        try:
            if self.inputfile is not None:
                if self.loadFile() is False:
                    print("Error loading VLM file", file=sys.stderr)
                    return False
        except AttributeError:
            pass

        if self.getObjectList() is False:
            self.bail("Failed to get objects/shadows from file")

        log = open("/tmp/autoshadow.log", "w")
        print(
            "Paths from VLM ({}):".format(len(self.paths)), self.paths, file=log
        )

        self.paths.sort()
        # existing_paths = self.mergeRawPaths(self.paths)
        # print("existing paths:", existing_paths, file=log)
        # new_paths = self.calculateOptimalPaths(existing_paths)
        new_paths = self.calculateOptimalPaths(self.getRawPathData())
        print(
            "Shadows from Autoshadow ({}):".format(len(new_paths)),
            new_paths,
            file=log,
        )

        if len(self.shadows) < len(new_paths):
            self.bail("Not enough shadows to cover generated paths")

        self.invalidateShadows()
        self.moveShadows(new_paths)

        print("Generated shadows:", new_paths, file=log)
        log.close()

        try:
            if self.outputfile is not None:
                self.save()
        except AttributeError:
            pass


if __name__ == "__main__":
    instance = Autoshadow()
    instance.run()

# vim: tabstop=8 expandtab shiftwidth=4 softtabstop=4 foldlevelstart=2
