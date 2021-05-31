#!/usr/bin/env python3

# Needed until blastpy is installed system-wide
import sys
from os.path import expanduser

sys.path.append(expanduser("~") + "/projects/blastpit/build")
import blastpy

# Get our machine-specific constants
sys.path.append(expanduser("~") + "/projects/blastpit/contrib/cfg")
import myconfig

import xml.etree.ElementTree as ET
from io import StringIO  # To convert string to csv file
import csv
import re
import getopt
import itertools

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


class autoshadow:
    def __init__(self):
        self.paths = []
        self.shadows = []
        self.moved_shadows = []

    def printUsage(self):
        print("Usage: autoshadow.py -i <inputfile> -o <outputfile>")

    def parseArgs(self):
        try:
            opts, args = getopt.getopt(
                sys.argv[1:], "hi:o:w:", ["ifile=", "ofile=", "width="]
            )
        except getopt.GetoptError:
            return False
        # if len(sys.argv) < 4:
        #     return False
        for opt, arg in opts:
            if opt in ("-i", "--ifile"):
                self.inputfile = arg
            elif opt in ("-o", "--ofile"):
                self.outputfile = arg
            elif opt in ("-w", "--width"):
                self.maxwidth = arg
        try:
            if self.outputfile is None:
                self.outputfile = self.inputfile
        except:
            self.outputfile = self.inputfile
        print("self.outputfile =", self.outputfile)

    def save(self):
        print("Saving VLM file...")

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

    def zoom(self):
        blastpy.BpQueueCommandArgs(
            self.blast,
            blastpy.kZoomWindow,
            "x1",
            "30",
            "y1",
            "30",
            "x2",
            "90",
            "y2",
            "90",
            None,
        )
        blastpy.BpUploadQueuedMessages(self.blast)

    def connect(self):
        # Connect to server or bail
        self.blast = blastpy.blastpitNew()
        result = blastpy.connectToServer(
            self.blast, myconfig.WS_SERVER_LOCAL, myconfig.WS_TIMEOUT_SHORT
        )
        if result != blastpy.kSuccess:
            print("Can't connect to server (%d)" % result, file=sys.stderr)
            return False

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
                if is_path is not None or is_rect is not None or is_circle is not None:
                    self.paths.append(
                        [
                            row[0],
                            float(row[1]),
                            float(row[2]),
                            float(row[4]),
                            float(row[5]),
                        ]
                    )
        else:
            print("kGetGeoList failed")
            return False

    def mergeRawPaths(self):
        # Merges any paths with the same name into one single path
        merged_paths = []
        basename = None
        x1 = 0
        x2 = 0
        pattern = re.compile("(.*)-(\d+)")
        for path in self.paths:
            match = re.search(pattern, path[0])
            if match:
                print("Match found")
                if float(path[1]) < x1:
                    x1 = float(path[1])
                    print("x1 moved to", x1, "by", path[0])
                if float(path[1]) + float(path[3]) > x2:
                    x2 = float(path[1]) + float(path[3])
                    print("x2 moved to", x2, "by", path[0])
            else:
                # Dump any finished path block
                if basename is not None:
                    print("Transferring path:", basename)
                    merged_paths.append([x1, x2 - x1])
                    basename = None
                print("Found new base path:", path[0])
                basename = path[0]
                x1 = float(path[1])
                x2 = x1 + float(path[3])
        print("Transferring final path:", basename)
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

    def roundPaths(self, paths):
        roundedPaths = []
        for path in paths:
            roundedPaths.append([round(path[0], 2), round(path[1], 2)])

        return roundedPaths

    def removeInternalPaths(self, paths):
        majorPaths = []
        for path in paths:
            is_subpath = False
            for testPath in paths:
                if (float(testPath[0]) <= float(path[0])) and (
                    float(path[0]) + float(path[1])
                ) < (float(testPath[0]) + float(testPath[1])):
                    print("Subpath", path, "exists within", testPath)
                    is_subpath = True
                    break
            if is_subpath is False:
                majorPaths.append(path)

        return majorPaths

    def mergeNeighbouringPaths(self, paths):
        unduplicatePaths = self.removeDuplicates(paths)
        mergedPaths = []
        for i in range(0, len(unduplicatePaths) - 1):
            idxBestMerge = -1
            valBestMerge = 0
            for j in range(0, len(unduplicatePaths) - 1 - i):
                width = self.totalWidth(unduplicatePaths[i], unduplicatePaths[j])
                if width > valBestMerge and width <= float(self.maxwidth):
                    # Found a new widest pair
                    idxBestMerge = j
                    valBestMerge = width
            if idxBestMerge > -1:
                mergedPaths.append(
                    [float(unduplicatePaths[idxBestMerge][0]), float(valBestMerge)]
                )

        return mergedPaths + unduplicatePaths

    def calculateOptimalPaths(self, paths):

        # Sort paths by size
        sortedPaths = self.roundPaths(self.sortPathsByWidth(paths))
        print("Sorted paths:")
        print(sortedPaths)
        print()

        # Remove paths larger than max
        sizedPaths = self.removeOversizePaths(sortedPaths)
        print("Sized paths:")
        print(self.sortPathsByStartPosition(sizedPaths))
        print()

        # Merge all possible path combinations
        mergedPaths = self.removeDuplicates(
            self.roundPaths(self.mergeNeighbouringPaths(sizedPaths))
        )
        print("Merged paths:")
        print(self.sortPathsByStartPosition(mergedPaths))
        print()

        # Remove paths that exist inside other paths
        majorPaths = self.removeInternalPaths(mergedPaths)
        print("Major paths:")
        print(self.sortPathsByStartPosition(majorPaths))
        print()

        return sizedPaths

    def totalWidth(self, path1, path2):
        paths = self.sortPathsByStartPosition([path1, path2])
        return float(paths[1][0]) + float(paths[1][1]) - float(paths[0][0])

    def moveShadows(self, newShadowPositions):
        i = 0
        for shadow in self.shadows:
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
            # if i > 0:
            if i >= len(newShadowPositions):
                break

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

    def removeDuplicates(self, listOfLists):
        listOfLists.sort()
        return list(listOfLists for listOfLists, _ in itertools.groupby(listOfLists))

    def bail(self, errorMessage):
        print(errorMessage)
        sys.exit(1)

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
        self.paths.sort()
        existing_paths = self.mergeRawPaths()
        # print(existing_paths)
        new_paths = self.calculateOptimalPaths(existing_paths)
        self.invalidateShadows()
        self.moveShadows(new_paths)

        try:
            if self.outputfile is not None:
                self.save()
        except AttributeError:
            pass

    def cleanup(self):
        # Disconnect from server; Close open files
        blastpy.disconnectFromServer(self.blast)


if __name__ == "__main__":
    job = autoshadow()
    job.run()
