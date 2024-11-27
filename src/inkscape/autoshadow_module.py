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
autoshadow_module.py: Inkscape extension to modify VLM shadow positions
"""

import colorsys
import datetime
import json
import math

# Needed until blastpy is installed system-wide
import sys
from os.path import expanduser

import inkex
from lxml import etree

sys.path.append(expanduser("~") + "/projects/blastpit/src/scaffolding")
from autoshadow import Autoshadow


class AutoshadowModule(inkex.EffectExtension):
    """Automatically optimise shadow positions"""

    def __init__(self):
        # We need to call the parent class (inkex) __init__
        super(AutoshadowModule, self).__init__()

        self.paths = []
        self.shadows = []
        self.moved_shadows = []
        self.inputfile = None
        self.maxwidth = 1.0
        self.diameter = 20.0

    def wipeShadows(self):
        # Delete any existing data labels
        # Returns the group for easier replacment with new shadows
        for group in self.document.getroot():
            if (
                "{http://www.inkscape.org/namespaces/inkscape}label"
                in group.attrib.keys()
            ):
                if (
                    "Shadow Layer"
                    in group.attrib[
                        "{http://www.inkscape.org/namespaces/inkscape}label"
                    ]
                ):
                    for element in group:
                        try:
                            if (
                                "Shadows"
                                in element.attrib[
                                    "{http://www.inkscape.org/namespaces/inkscape}label"
                                ]
                            ):
                                group.remove(element)
                                return group
                        except KeyError:
                            pass

    def colourShadows(self, layer, shadows):
        # Create layer element (or use self.current_layer)
        # layer = etree.SubElement(self.document.getroot(), "g")
        # layer.set(inkex.addNS("groupmode", "inkscape"), "layer")
        # layer.set(inkex.addNS("label", "inkscape"), "Shadow Layer")

        # Create group
        grp_name = "Shadows"
        grp_attribs = {
            inkex.addNS("label", "inkscape"): grp_name,
        }
        grp = etree.SubElement(layer, "g", grp_attribs)

        # Create coloured sectors
        y_offset = 50
        y_size = 20

        parent = grp
        for shadow in shadows:
            self.draw_rectangle(
                (shadow[1], y_size),
                (shadow[0], y_offset),
                float(shadow[0] * (1.0 / float(self.diameter * math.pi))),
                parent,
                "shadow" + str(shadow[0]).zfill(2),
            )

    def draw_rectangle(self, width_height, x_y, hue, parent, id):
        (w, h) = width_height
        (x, y) = x_y
        colours = colorsys.hls_to_rgb(hue, 0.80, 1.0)
        colstr = (
            "#"
            + format(int(colours[0] * 255), "02x")
            + format(int(colours[1] * 255), "02x")
            + format(int(colours[2] * 255), "02x")
        )

        style = {
            "stroke": "none",
            "stroke-width": "1",
            "fill": colstr,
            "fill-opacity": "0.8",
        }

        attribs = {
            "style": str(inkex.Style(style)),
            "height": str(h),
            "width": str(w),
            "x": str(x),
            "y": str(y),
            "id": id,
            inkex.addNS("label", "inkscape"): "shadow",
        }

        etree.SubElement(parent, inkex.addNS("rect", "svg"), attribs)

    def colourPaths(self, shadows):
        # For each path, determine which shadow it falls within and
        # give it the colour of that shadow
        return False

    def main(self):
        ash = Autoshadow(maxwidth=self.maxwidth)

        if ash.connect() is False:
            print("Failed to connect to server", file=sys.stderr)
            sys.exit(1)

        try:
            if self.inputfile is not None:
                ash.setInputfile(self.inputfile)
                if ash.loadFile() is False:
                    print(
                        "Error loading VLM file with filename",
                        self.inputfile,
                        file=sys.stderr,
                    )
                    return False
        except AttributeError:
            pass

        ash.showWindow()

        log = open("/tmp/autoshadow.log", "w")

        if ash.getObjectList() is False:
            self.bail("Failed to get objects/shadows from file")

        # print("Object list:", file=sys.stderr)
        # print(ash.paths, file=sys.stderr)

        ash.paths.sort()

        # Process the shadows
        existing_paths = ash.roundPaths(ash.mergeRawPaths(ash.getPaths()))
        new_paths = ash.roundPaths(ash.calculateOptimalPaths(existing_paths))

        # The new_paths variable can be considered as the calculated shadows
        # print("paths: ", existing_paths, file=sys.stderr)
        # print("shadows: ", new_paths, file=sys.stderr)
        # numUnconveredPaths = ash.findPathsWithoutShadows(
        # existing_paths, new_paths
        # )
        # if len(numUnconveredPaths) > 0:
        #     print(
        #         "Warning: failed to generate shadows for %s path(s)"
        #         % len(numUnconveredPaths),
        #         file=sys.stderr,
        #     )
        #     print("Paths without shadows:", numUnconveredPaths, file=sys.stderr)

        # print("Merge Raw Paths:")
        # print(existing_paths, file=log)
        # print("Final Calculated Paths:")
        # print(new_paths, file=log)

        # Show the new shadows in Inkscape
        shadowLayer = self.wipeShadows()
        if shadowLayer is None:
            shadowLayer = self.createShadowLayer()
        self.colourShadows(shadowLayer, new_paths)

        # Update LMOS shadows
        ash.invalidateShadows()
        ash.moveShadows(new_paths)

        try:
            if self.inputfile is not None:
                ash.setOutputfile(self.inputfile)
                ash.saveVLM()
        except AttributeError:
            pass

        ash.cleanup()
        log.close()

    def createShadowLayer(self):
        root = self.document.getroot()
        layer = etree.SubElement(root, "g")
        layer.set(inkex.addNS("groupmode", "inkscape"), "layer")
        layer.set(inkex.addNS("label", "inkscape"), "Shadow Layer")
        return layer

    def effect(self):
        # Get filename from document
        for group in self.document.getroot():
            for child in group:
                if (
                    "{http://www.inkscape.org/namespaces/inkscape}label"
                    in child.attrib.keys()
                ):
                    if (
                        "laserdata"
                        in child.attrib[
                            "{http://www.inkscape.org/namespaces/inkscape}label"
                        ]
                    ):
                        myjson = json.loads(child.text)
                        # print(myjson, file=sys.stderr)
                        if myjson is not None:
                            try:
                                if myjson["filename"] is not None:
                                    self.inputfile = myjson["filename"]
                                    # print("Saving filename", file=sys.stderr)
                            except BaseException:
                                pass
                            try:
                                if myjson["customer"] is not None:
                                    self.customer = myjson["customer"]
                            except BaseException:
                                pass
                            try:
                                if myjson["maxwidth"] is not None:
                                    self.maxwidth = float(myjson["maxwidth"])
                                    # print("Saving maxwidth", file=sys.stderr)
                            except BaseException:
                                pass
                            try:
                                if myjson["diameter"] is not None:
                                    self.diameter = float(myjson["diameter"])
                                    # print("Saving diameter", file=sys.stderr)
                            except BaseException:
                                pass
                            try:
                                if myjson["mode"] is not None:
                                    self.mode = float(myjson["mode"])
                            except BaseException:
                                pass

        try:
            if self.inputfile is not None and self.customer is not None:
                self.inputfile = (
                    "Z:\\drawings\\"
                    + str(datetime.date.today().year)
                    + "\\"
                    + self.customer
                    + "\\"
                    + self.inputfile
                    + ".VLM"
                )
            else:
                self.inputfile = "C:\\Rofin\\VisualLaserMarker\\MarkingFiles\\inkscape_export.VLM"

        except AttributeError:
            self.inputfile = None

        # Move shadows
        self.maxwidth = getattr(self, "maxwidth", 1.0)
        self.diameter = getattr(self, "diameter", 20.0)
        self.main()


if __name__ == "__main__":
    instance = AutoshadowModule()
    instance.run()

# vim: tabstop=8 expandtab shiftwidth=4 softtabstop=4
