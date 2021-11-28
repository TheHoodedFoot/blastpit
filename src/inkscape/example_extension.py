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
laser.py: Exports Inkscape Geometries to LMOS
"""

from collections import namedtuple
from io import StringIO
import csv
import inkex
import re
import sys

# import os
import xml.dom.minidom
import xml.etree.ElementTree as ET
import webcolors
import logging


# Needed until blastpy is installed system-wide
from os.path import expanduser

sys.path.append(expanduser("~") + "/projects/blastpit/build")
import blastpy

# Get our machine-specific constants
sys.path.append(expanduser("~") + "/projects/blastpit/res/cfg")
import myconfig


def print_transform(transform):
    """Displays a transform to the user."""
    for tr in transform.to_hexad():
        print(tr, end=" ", file=sys.stderr)
    print(file=sys.stderr)


def closest_colour(requested_colour):
    min_colours = {}
    # for key, name in webcolors.css3_hex_to_names.items():
    for key, name in webcolors.CSS3_HEX_TO_NAMES.items():
        r_c, g_c, b_c = webcolors.hex_to_rgb(key)
        rd = (r_c - requested_colour[0]) ** 2
        gd = (g_c - requested_colour[1]) ** 2
        bd = (b_c - requested_colour[2]) ** 2
        min_colours[(rd + gd + bd)] = name
    return min_colours[min(min_colours.keys())]


def get_colour_name(requested_colour):
    """Returns the webcolour name closest to the hex value."""
    try:
        closest_name = webcolors.rgb_to_name(requested_colour)
    except ValueError:
        closest_name = closest_colour(requested_colour)
    return closest_name


def get_fill(element):
    """Returns the element fill colour, or None if unfilled."""
    try:
        return element.style["fill"].strip("#")
    except KeyError:
        return None


def rgb_from_string(colour):
    try:
        red = (int(colour[-6:], 16) >> 16) & 0xFF
        green = (int(colour[-6:], 16) >> 8) & 0xFF
        blue = (int(colour[-6:], 16)) & 0xFF
    except TypeError:
        logging.warn(
            "rgb_from_string: Error - no colour specified. Defaulting to black"
        )
        red = green = blue = 0

    return red, green, blue


def get_colour_from_string(colour):
    red, green, blue = rgb_from_string(colour)

    return get_colour_name((red, green, blue))


def addColourLayer(lmosxml, colour, height=120):
    red, green, blue = rgb_from_string(colour)
    lmosxml.addLayer(
        get_colour_name((red, green, blue)), round(height, 2), red, green, blue
    )

    return get_colour_name((red, green, blue))


def is_compound_path(path):
    """Determines if this path is compound."""

    subpath_count = 0
    for segment in path.to_superpath().to_path(curves_only=True):
        matches = re.match(".*M (.*) (.*)$", str(segment))
        if matches is not None:
            subpath_count += 1
            if subpath_count > 1:
                return True

    return False


def add_container_group(
    element, element_bbox, lmosxmlgroup, parent_xml, lmosxml
):
    """Creates a holding group for a compound path."""

    colour = get_colour_from_string(get_fill(element))

    lmosxmlgroup = lmosxml.addGroup(
        "g" + element.get_id(),
        colour,
        colour,
        "bp_0_01",
        parent_xml,
    )

    lmosxml.setGroupBoundingBox(
        lmosxmlgroup,
        element_bbox.centrex,
        element_bbox.centrey,
        element_bbox.width,
        element_bbox.height,
    )

    return lmosxmlgroup


class SvgToVlm(inkex.EffectExtension):
    """Convert Inkscape SVG to Rofin VLM"""

    def __init__(self):
        # We need to call the parent class (inkex) __init__
        super(SvgToVlm, self).__init__()

        # Holds the generated LMOS XML
        self.lmosxml = blastpy.BpXML()

    def print_info(self, element):
        """Prints useful information to stderr."""
        print(type(element), file=sys.stderr)
        print(
            "Element: ",
            element.get_id(),
            " has bounding box ",
            self.get_bbox(element),
            " and transform ",
            end=" ",
            file=sys.stderr,
        )
        print_transform(element.transform)

    def get_bbox(self, element, parent_transform=None):
        """Returns useful values of an element's bounding box.

        Returns the bounding coordinates, width, height, and centre.
        Also returns useful transforms that can move
        the element origin or centre to 0,0.
        """

        bbox1, bbox2 = element.bounding_box(transform=parent_transform)
        x1, x2 = bbox1
        y1, y2 = bbox2
        width = float(x2) - float(x1)
        height = float(y2) - float(y1)
        cenx = x1 + width / 2
        ceny = y2 - height / 2
        tr_centozero = inkex.Transform()
        tr_centozero.add_translate(-cenx, -ceny)
        tr_origintozero = inkex.Transform()
        tr_origintozero.add_translate(-x1, -y1)
        result = namedtuple(
            "BoundingBox",
            [
                "x1",
                "y1",
                "x2",
                "y2",
                "width",
                "height",
                "centrex",
                "centrey",
                "centretozerozero",
                "origintozerozero",
            ],
        )
        return result(
            x1,
            y1,
            x2,
            y2,
            width,
            height,
            cenx,
            ceny,
            tr_centozero,
            tr_origintozero,
        )

    def process_shape(self, element_array):
        """Convert the transformed element to cubic paths.

        element[0] : Inkscape element
        element[1] : Parent of Inkscape element
        element[2] : Parent XML node
        element[3] : Cumulative parent transform
        """

        element = element_array[0]
        parent_element = element_array[1]
        parent_xml = element_array[2]
        # parent_transform = element_array[3]

        element_bbox = None
        path = None
        lmosxmlgroup = None
        # fill = get_fill(element)
        polyline = None

        #
        # Check if a compound
        #
        # If so, create a group and then set the group as element[1]
        #
        # Then continue as normal (but don't create another group)

        if is_compound_path(element.path) is True:

            logging.debug("Processing compound path " + element.get_id())

            path_bbox = self.get_bbox(element)
            element_bbox = self.get_bbox(
                element, inkex.Transform(matrix=((1, 0, 0), (0, -1, 120)))
            )

            # print("Creating a compound group", file=sys.stderr)
            # if fill is not None:
            #     polyline.setLayer(
            #         addColourLayer(self.lmosxml, get_fill(element))
            #     )

            if lmosxmlgroup is None:
                logging.debug("Adding container group g" + element.get_id())
                lmosxmlgroup = add_container_group(
                    element,
                    element_bbox,
                    lmosxmlgroup,
                    parent_xml,
                    self.lmosxml,
                )

            # If compound, this will end up inside a group, so the individual paths
            # need to be translated around 0,0
            logging.debug(
                "Transforming compound path "
                + element.get_id()
                + " to zero zero"
            )
            path = element.path.transform(
                inkex.Transform(matrix=((1, 0, 0), (0, -1, path_bbox.height)))
                * path_bbox.origintozerozero
                * element.transform,
                inplace=False,
            )

        elif parent_xml is None:
            logging.debug(
                "Transforming compound path "
                + element.get_id()
                + " with no parent xml"
            )
            # print("process_shape: not part of group", file=sys.stderr)
            # We are not part of a group, so this must be a single element
            # on the root layer.
            # Get our bounding box and add the standard Inkscape->LMOS transform,
            # plus the element transform if one exists.
            # path = element.path.copy()
            element_bbox = self.get_bbox(
                element, inkex.Transform(matrix=((1, 0, 0), (0, -1, 120)))
            )
            # path.transform(inkex.Transform(matrix=((1, 0, 0), (0, -1, 120)))
            # * element_bbox.origintozerozero)
            # path.transform(inkex.Transform(matrix=((1, 0, 0), (0, -1, 120))))
            # print(element_bbox, file=sys.stderr)
            # print("setting path with no parent xml", file=sys.stderr)
            path = element.path.transform(
                inkex.Transform(matrix=((1, 0, 0), (0, -1, 120)))
                * element.transform
                * inkex.Transform(
                    matrix=(
                        (1, 0, element_bbox.width / 2),
                        (0, 1, -element_bbox.height / 2),
                    )
                ),
                inplace=False,
            )
        else:
            logging.debug(
                "Transforming compound path "
                + element.get_id()
                + " with parent xml"
            )
            # print("process_shape: part of group", file=sys.stderr)
            # We are inside a group, so we must translate to 0,0 and then
            # flip according to the parent's bounding box

            # Temporarily disable the parent's transform to prevent it
            # from additionally transforming the children
            parent_copy = parent_element
            parent_copy.transform = inkex.Transform()
            element_bbox = self.get_bbox(parent_copy)
            # print("setting path with parent xml", file=sys.stderr)
            path = element.path.transform(
                inkex.Transform(
                    matrix=((1, 0, 0), (0, -1, element_bbox.height))
                )
                * element_bbox.origintozerozero,
                inplace=False,
            )
            # print_transform(element_bbox.origintozerozero)

        # The group transform should be
        # relative to the centre of the group bounding box, not the
        # element bounding box. This maintains the positions of the
        # group elements respective to one another.

        # Start a new polyline
        x = y = None
        # lmosxmlgroup = None

        # tr = inkex.Transform()
        # # tr.add_matrix((1, 0, 0), (0, -1, 120))
        # path = path.transform(tr)
        for segment in path.to_superpath().to_path(curves_only=True):

            matches = re.match(".*M (.*) (.*)$", str(segment))
            if matches is not None:
                x = float(matches.group(1))
                y = float(matches.group(2))

                if polyline is not None:
                    # If there is already a polyline, this must be a compound path,
                    # so add it to a container group

                    self.lmosxml.addPolyline(polyline, lmosxmlgroup)

                # Prepare a new polyline
                polyline = blastpy.BpPolyline(element.get_id(), x, y)

            matches = re.match(
                ".*C (.*) (.*) (.*) (.*) (.*) (.*)$", str(segment)
            )
            if matches is not None:
                polyline.bezier(
                    float(matches.group(1)),
                    float(matches.group(2)),
                    float(matches.group(3)),
                    float(matches.group(4)),
                    float(matches.group(5)),
                    float(matches.group(6)),
                )

                # End of segment becomes start of next
                x = float(matches.group(5))
                y = float(matches.group(6))

        polyline.setLayer(addColourLayer(self.lmosxml, get_fill(element)))
        # addColourLayer(self.lmosxml, get_fill(element))
        # polyline.setLayer("bp_" + get_fill(element))
        if lmosxmlgroup is None:
            polyline.setHp(get_fill(element))
            if parent_xml is None:
                logging.debug(
                    "Adding polyline with no lmosxmlgroup or parent xml"
                )
                self.lmosxml.addPolyline(polyline, None, True)
            else:
                logging.debug(
                    "Adding polyline with parent xml but no lmosxmlgroup"
                )
                self.lmosxml.addPolyline(
                    polyline, parent_xml
                )  # , lmosxmlgroup)
        else:
            logging.debug("Adding polyline with lmosxmlgroup and parent xml")
            self.lmosxml.addPolyline(polyline, lmosxmlgroup)  # , lmosxmlgroup)

    def process_group(self, group):
        """Recurse into groups"""

        # If a group, we need to get the bounding box, and then create
        # an xml group of the same size, offset from its parent bounding box
        # so that its centre is at bbx + width/2, bby + height/2

        # The problem is that get_bounding_box will only get the x and y coordinates
        # with respect to the global 0,0, so we need to be able to get both our
        # raw coordinates and the raw coordinates of our parent, so the parent_element
        # must be supplied by the caller. For the root element, this can be a dummy
        # 120x120 rectangle that can be deleted at the end.

        # Therefore we need to know the parent's bounding box

        # If this group is a hidden layer, abort
        # If not hidden, just process its children.
        if group[0].get("inkscape:groupmode") == "layer":
            style = group[0].get("style")
            if style is not None:
                if style == "display:none":
                    return
            for subelement in group[0]:
                self.process_element((subelement, group[0], group[2], group[3]))
            return

        # Create the XML group using width, height, centrex and centrey
        existing_transform = group[0].transform
        if group[1] is not None:
            if group[1].get("inkscape:groupmode") != "layer":
                # print("Has parent: applying transform", file=sys.stderr)
                # print(group[1].get_id(), file=sys.stderr)
                # print(
                #     "current bbox:",
                #     SvgToVlm.get_bbox(group[0], parent_transform=group[3]),
                #     file=sys.stderr,
                # )
                group[0].transform *= self.get_bbox(group[1]).origintozerozero
                # print(
                #     "new bbox:",
                #     SvgToVlm.get_bbox(group[0], parent_transform=group[3]),
                #     file=sys.stderr,
                # )
        group_bbox = self.get_bbox(group[0], parent_transform=group[3])

        hp = get_fill(group[0])
        lmosxmlgroup = None
        if hp is None:
            lmosxmlgroup = self.lmosxml.addGroup(
                group[0].get_id(), "RofinStandard", "Standard", None, group[2]
            )
        else:
            colour = get_colour_from_string(hp)
            lmosxmlgroup = self.lmosxml.addGroup(
                group[0].get_id(), colour, colour, "bp_0_01", group[2]
            )

        logging.debug(
            "Setting group "
            + group[0].get_id()
            + " bounding box to "
            + str(group_bbox.centrex)
            + ", "
            + str(group_bbox.centrey)
        )
        self.lmosxml.setGroupBoundingBox(
            lmosxmlgroup,
            group_bbox.centrex,
            group_bbox.centrey,
            group_bbox.width,
            group_bbox.height,
        )

        # group[0].transform = existing_transform
        # group[0].transform = inkex.Transform()
        # print_transform(group[0].transform)

        for subelement in group[0]:
            self.process_element(
                (subelement, group[0], lmosxmlgroup, group[0].transform)
            )
        group[0].transform = existing_transform

    def process_element(self, bpelement):
        """Process elements or groups.

        Takes a tuple (bpelement):

        bpelement[0] : Inkscape element
        bpelement[1] : Parent of Inkscape element
        bpelement[2] : Parent XML node
        bpelement[3] : Cumulative parent transform (not needed?)
        """

        # Every element, whether geometry or group,
        # must be mirrored in the y axis and then
        # moved up by the height of its parent bounding box.

        if isinstance(
            bpelement[0],
            (
                inkex.elements._polygons.PathElement,
                inkex.elements._polygons.Rectangle,
                inkex.elements._polygons.Ellipse,
                inkex.elements._polygons.Circle,
            ),
        ):
            logging.debug("Processing shape " + bpelement[0].get_id())
            self.process_shape(bpelement)

        if isinstance(bpelement[0], inkex.elements._groups.Group):
            logging.debug("Processing group " + bpelement[0].get_id())
            self.process_group(bpelement)

    def unittest(self, args):
        """Run effect and return custom data for testing."""

        self.we_are_testing = True
        self.run(args)
        return self.testresult

    def effect(self):

        logging.basicConfig(level=logging.DEBUG)

        transform = inkex.Transform()
        paths = None
        if len(self.svg.selected) <= 0:
            paths = self.svg
            transform = inkex.Transform(matrix=((1, 0, 0), (0, -1, 120)))
        else:
            transform = inkex.Transform(matrix=((1, 0, 0), (0, -1, 120)))
            paths = self.svg.selected

        for element in paths:
            self.process_element((element, None, None, transform))

        xmlfile = StringIO(self.lmosxml.xml().decode("utf-8"))
        xmlpretty = xml.dom.minidom.parse(xmlfile).toprettyxml()

        # Stash the generated LMOS XML for unit testing
        self.testresult = xmlpretty

        # We don't want to go any further than this point when testing
        if hasattr(self, "we_are_testing"):
            return

        # When debugging, show the xml
        logging.info(xmlpretty)
        # self.msg(xmlpretty)
        # return

        blast = blastpy.blastpitNew()
        blastpy.connectToServer(
            blast, myconfig.WS_SERVER_REMOTE, myconfig.WS_TIMEOUT_SHORT
        )
        blastpy.BpDisplayLmosWindow(blast, 1)
        id = blastpy.BpQueueCommandArgs(
            blast,
            blastpy.kImportXML,
            str(self.lmosxml.xml()),
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

        filename = (
            "C:\\Rofin\\VisualLaserMarker\\MarkingFiles\\inkscape_export.VLM"
        )
        id = blastpy.BpQueueCommandArgs(
            blast,
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
        blastpy.BpUploadQueuedMessages(blast)
        blastpy.BpWaitForReplyOrTimeout(blast, id.id, myconfig.WS_TIMEOUT_LONG)

        blastpy.disconnectFromServer(blast)
        blastpy.blastpitDelete(blast)

        return

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


if __name__ == "__main__":
    SvgToVlm().run()

# vim: tabstop=8 expandtab shiftwidth=4 softtabstop=4
