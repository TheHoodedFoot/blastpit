#!/usr/bin/env python
# -*- coding: utf-8 -*-

import numpy
import math
import colorsys
import simplestyle
import inkex

from lxml import etree

'''
Copyright (C) 2009 Richard Querin, screencasters@heathenx.org
Copyright (C) 2009 heathenx, screencasters@heathenx.org
Modified from an extension distributed with JessyInk (code.google.com/p/jessyink).

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see http://www.gnu.org/licenses/.
'''

# These lines are only needed if you don't put the script directly into
# the installation directory
import sys
# Unix
sys.path.append('/usr/share/inkscape/extensions')


# Laser constants
# SAGITTA = 2.0	 # Focal range
# ROTARY_Z_LEVEL = 77.0	 # Height from table to centre of rotation
# ROTARY_OVERLAP = 2.0	# Shadow overlap


# Math functions
# https://en.wikipedia.org/wiki/Sagitta_(geometry)
# https://en.wikipedia.org/wiki/Circular_segment

def chordLength(radius, sagitta):
    return 2 * math.sqrt(2 * float(radius) * float(sagitta) -
                         float(sagitta) * float(sagitta))


def chordAngle(radius, sagitta):
    return 2 * math.asin(chordLength(float(radius),
                                     float(sagitta)) / (2 * float(radius)))


def chordArcLength(radius, sagitta):
    c = chordLength(float(radius), float(sagitta))
    hc = float(sagitta) + ((c * c) / (4 * float(sagitta)))
    return math.asin(c / hc) * hc


def getHeightAtAngle(height, angle):
    return math.sin(math.radians(90 - angle)) * height


def axialAngle(radius, width, overrideMin=False):
    # Find tilt (in degrees so we can round it next)
    tilt = math.degrees(math.atan(float(width) / float(radius * 2)))

    # Round up to nearest 5 degrees
    if int(round(tilt)) != tilt:
        tilt = int(round(tilt / 5) * 5)

    # The angle must be at least 20 degrees to avoid the mounts
    if int(tilt) < 20 and overrideMin == "false":
        tilt = 20

    #print( tilt, radius, width, overrideMin, file=sys.stderr)
    return tilt


class ring_generator(inkex.Effect):
    def __init__(self):
        # Call the base class constructor.
        inkex.Effect.__init__(self)

        options = [
            ["a", "name", "", "Unknown"],
            ["d", "diameter", "0", "Ring diameter"],
            ["l", "holder", "", "Ring holder type"],
            ["o", "offset", "0", "Ring holder offset"],
            ["t", "ring_type", "", "Concave or convex ring type"],
            ["w", "width", "6.0", "Ring width"],
            ["s", "sagitta", "1.0", "Focal range"],
            ["v", "overlap", "2.0", "Shadow overlap"],
            ["f", "face", "76.0", "Ring holder face offset"],
            ["x", "axisheight", "77.0", "Height of rotary axis"],
            ["r", "override", "False", "Override the 20 degree limit"],
        ]

        for arg in options:
            self.arg_parser.add_argument(
                "-" + arg[0], "--" + arg[1], default=arg[2], help=arg[3])
        # for arg in options:
        #	  self.OptionParser.add_option(
        #		  "-" + arg[0],
        #		  "--" + arg[1],
        #		  action="store",
        #		  dest=arg[1],
        #		  type=arg[2],
        #		  default=arg[3],
        #		  help=arg[4])

    def draw_rectangle(
            self, width_height, x_y,
            hue,
            parent,
            id):

        (w, h) = width_height
        (x, y) = x_y
        colours = colorsys.hls_to_rgb(
            hue, 0.80, 1.0)
        colstr = "#" + format(int(colours[0] * 255),
                              '02x') + format(int(colours[1] * 255),
                                              '02x') + format(int(colours[2] * 255),
                                                              '02x')

        # print >> sys.stderr, colstr
        # xml.addLayer(
        #	  str("%.1f" % i),
        #	  colours[0] * 255,
        #	  colours[1] * 255,
        #	  colours[2] * 255,
        #	  i)

        style = {'stroke': 'none',
                 'stroke-width': '1',
                 'fill': colstr,
                 'fill-opacity': '0.8'
                 }

        attribs = {
            'style': str(inkex.Style(style)),
            'height': str(h),
            'width': str(w),
            'x': str(x),
            'y': str(y),
            'id': id,
            inkex.addNS('label', 'inkscape'): 'shadow'
        }

        etree.SubElement(parent, inkex.addNS('rect', 'svg'), attribs)

    def effect(self):

        root = self.document.getroot()

        r_id = 1

        RADIUS = float(self.options.diameter) / 2
        SEGMENTS = int(math.ceil((math.pi *
                                  float(self.options.diameter)) /
                                 (chordArcLength(RADIUS, self.options.sagitta) -
                                  2 *
                                  float(self.options.overlap))))
        SECTOR_WIDTH = (math.pi * float(self.options.diameter)) / SEGMENTS

        # Resize the document to the area of the ring
        r_width = str(math.pi * float(self.options.diameter))
        root.attrib['width'] = r_width + 'mm'
        root.attrib['height'] = '120mm'
        root.attrib['viewBox'] = '0 0 ' + \
            r_width + ' 120'

        # Delete any existing data labels
        for group in self.document.getroot():
            if '{http://www.inkscape.org/namespaces/inkscape}label' in group.attrib.keys():
                if "Shadow Layer" in group.attrib['{http://www.inkscape.org/namespaces/inkscape}label']:
                    root = self.document.getroot()
                    root.remove(group)
                    break
            for child in group:
                if '{http://www.inkscape.org/namespaces/inkscape}label' in child.attrib.keys():
                    if "laserdata" in child.attrib['{http://www.inkscape.org/namespaces/inkscape}label']:
                        group.remove(child)
                        break
                    if "ringdata" in child.attrib['{http://www.inkscape.org/namespaces/inkscape}label']:
                        group.remove(child)
                        break

        # Create layer element (or use self.current_layer)
        layer = etree.SubElement(root, 'g')
        layer.set(inkex.addNS('groupmode', 'inkscape'), 'layer')
        # layer_id = layer.get('id')
        layer.set(inkex.addNS('label', 'inkscape'), 'Shadow Layer')

        # Create group
        # centre = self.view_center	 # Put in in the centre of the current view
        # grp_transform = 'translate' + str(centre)
        grp_transform = ''

        grp_name = 'Shadows'
        grp_attribs = {inkex.addNS('label', 'inkscape'): grp_name,
                       'transform': grp_transform}
        grp = etree.SubElement(
            layer,
            'g',
            grp_attribs)  # the group to put everything in

        # Create coloured sectors
        y_offset = float(self.options.face)
        if self.options.holder == "topoffset":
            y_offset -= float(self.options.offset)
        elif self.options.holder == "centreoffset":
            y_offset -= float(self.options.offset) - \
                float(self.options.width) / 2
        y_offset = 120 - y_offset

        parent = grp  # or parent = self.current_layer
        for hue in numpy.arange(0, 1, 1.0 / SEGMENTS):
            i = (hue / (1.0 / SEGMENTS))
            sec_start = (SECTOR_WIDTH * i) - float(self.options.overlap) / 2
            sec_width = SECTOR_WIDTH + float(self.options.overlap)
            self.draw_rectangle((sec_width,
                                 float(self.options.width)),
                                (sec_start,
                                 y_offset),
                                hue,
                                parent,
                                "shadow" + str(int(hue * (SEGMENTS + 1))).zfill(2))

        layerHeight = 10
        axisAngle = 0
        # Calculate height
        if self.options.ring_type == "convex":
            layerHeight = float(self.options.axisheight) + \
                float(self.options.diameter) / 2
        else:
            axisAngle = axialAngle(
                float(self.options.diameter) / 2,
                float(self.options.width),
                self.options.override)
            layerHeight = float(self.options.axisheight) - \
                getHeightAtAngle(float(self.options.diameter) / 2, axisAngle)

        # Create text element
        text = etree.Element(inkex.addNS('text', 'svg'))
        text.set(inkex.addNS('label', 'inkscape'), 'laserdata')
        text.text = '{ "diameter": "' 
        text.text += str(round(float(self.options.diameter), 2))
        text.text += '",' + '"width": "' 
        text.text += str(round(float(self.options.width), 2))
        text.text += '",' + '"sectors": "' 
        text.text += str(round(360.0 / SEGMENTS, 2)) 
        text.text += '",' + '"height": "' 
        text.text += str((round(layerHeight, 1))) 
        text.text += '",' + '"angle": "' 
        text.text += str(int(axisAngle)) 
        text.text += '" }'

        # Set text position to center of document.
        width = self.svg.unittouu(root.attrib['width'])
        height = self.svg.unittouu(root.attrib['height'])
        text.set('x', str(width / 2))
        text.set('y', str(height / 2))

        # Center text horizontally with CSS style.
        style = {'text-align': 'center',
                 'text-anchor': 'middle',
                 'font-family': 'Droid Sans',
                 'font-size': '1',
                 }
        text.set('style', str(inkex.Style(style)))

        # Connect elements together.
        layer.append(text)

        for child in root:
            if '{http://www.inkscape.org/namespaces/inkscape}label' in child.attrib.keys():
                if "Shadow Layer" in child.attrib['{http://www.inkscape.org/namespaces/inkscape}label']:
                    shadowlayer = child
                    root.remove(child)
                    root.insert(3, shadowlayer)
                    break

        # Lock layer
        # layer.set(inkex.addNS('insensitive', 'sodipodi'), 'true')


effect = ring_generator()
effect.run()
