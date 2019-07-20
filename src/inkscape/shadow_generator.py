#!/usr/bin/env python
# -*- coding: utf-8 -*-

import numpy
import math
import colorsys
import simplestyle
import inkex
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
# SAGITTA = 2.0  # Focal range
# ROTARY_Z_LEVEL = 77.0  # Height from table to centre of rotation
# ROTARY_OVERLAP = 2.0  # Shadow overlap


# Math functions
# https://en.wikipedia.org/wiki/Sagitta_(geometry)
# https://en.wikipedia.org/wiki/Circular_segment

def chordLength(radius, sagitta):
    return 2 * math.sqrt(2 * radius * sagitta - sagitta * sagitta)


def chordAngle(radius, sagitta):
    return 2 * math.asin(chordLength(radius, sagitta) / (2 * radius))


def chordArcLength(radius, sagitta):
    c = chordLength(radius, sagitta)
    hc = sagitta + ((c * c) / (4 * sagitta))
    return math.asin(c / hc) * hc


class ring_generator(inkex.Effect):
    def __init__(self):
        # Call the base class constructor.
        inkex.Effect.__init__(self)

        options = [
            ["d", "diameter", "float", "0", "Ring diameter"],
        ]
        for arg in options:
            self.OptionParser.add_option(
                "-" + arg[0],
                "--" + arg[1],
                action="store",
                dest=arg[1],
                type=arg[2],
                default=arg[3],
                help=arg[4])

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
        #     str("%.1f" % i),
        #     colours[0] * 255,
        #     colours[1] * 255,
        #     colours[2] * 255,
        #     i)

        style = {'stroke': 'none',
                 'stroke-width': '1',
                 'fill': colstr,
                 'fill-opacity': '0.8'
                 }

        attribs = {
            'style': simplestyle.formatStyle(style),
            'height': str(h),
            'width': str(w),
            'x': str(x),
            'y': str(y),
            'id': id,
            inkex.addNS('label', 'inkscape'): 'shadow'
        }

        inkex.etree.SubElement(parent, inkex.addNS('rect', 'svg'), attribs)

    def effect(self):

        root = self.document.getroot()

        sagitta = 1

        RADIUS = self.options.diameter / 2
        SEGMENTS = int(math.ceil((math.pi * self.options.diameter)
                                 / (chordArcLength(RADIUS, sagitta)
                                    )))
        SECTOR_WIDTH = (math.pi * self.options.diameter) / SEGMENTS

        # Resize the document to the area of the ring
        r_width = str(math.pi * self.options.diameter)
        root.attrib['width'] = r_width + 'mm'
        root.attrib['height'] = '120mm'
        root.attrib['viewBox'] = '0 0 ' + \
            r_width + ' 120'

        # Create layer element (or use self.current_layer)
        layer = inkex.etree.SubElement(root, 'g')
        layer.set(inkex.addNS('groupmode', 'inkscape'), 'layer')
        # layer_id = layer.get('id')
        layer.set(inkex.addNS('label', 'inkscape'), 'Shadow Layer')

        # Create group
        # centre = self.view_center  # Put in in the centre of the current view
        # grp_transform = 'translate' + str(centre)
        grp_transform = ''

        grp_name = 'Shadows'
        grp_attribs = {inkex.addNS('label', 'inkscape'): grp_name,
                       'transform': grp_transform}
        grp = inkex.etree.SubElement(
            layer,
            'g',
            grp_attribs)  # the group to put everything in

        # Create coloured sectors

        # Erase old guides
        for node in self.document.xpath(
            "//sodipodi:guide",
                namespaces=inkex.NSS):
            node.getparent().remove(node)

        # getting parent tag of the guides
        namedview = self.document.xpath(
            '/svg:svg/sodipodi:namedview',
            namespaces=inkex.NSS)[0]

        # Disable display of page border
        # namedview.attrib['showborder'] = 'false'

        y_offset = 77
        y_offset = 120 - y_offset

        parent = grp  # or parent = self.current_layer
        for hue in numpy.arange(0, 1, 1.0 / SEGMENTS):
            i = (hue / (1.0 / SEGMENTS))
            rot = 360 * hue
            sec_start = (SECTOR_WIDTH * i)
            sec_width = SECTOR_WIDTH

            # Create a sodipodi:guide node
            posx = str(sec_start) + ",0"
            orientationString = "1,0"
            colours = colorsys.hls_to_rgb(hue, 0.50, 1.0)
            red = format(int(colours[0] * 255), '02x')
            green = format(int(colours[1] * 255), '02x')
            blue = format(int(colours[2] * 255), '02x')
            colstr = "#" + red + green + blue
            inkex.etree.SubElement(namedview,
                                   '{http://sodipodi.sourceforge.net/DTD/sodipodi-0.dtd}guide',
                                   {'position': posx,
                                    'orientation': orientationString,
                                    '{http://www.inkscape.org/namespaces/inkscape}color': colstr,
                                    '{http://www.inkscape.org/namespaces/inkscape}label': str(round(rot,
                                                                                                    2))})

            # self.draw_rectangle((sec_width,
            #                      self.options.width),
            #                     (sec_start,
            #                      y_offset),
            #                     hue,
            #                     parent,
            #                     "shadow" + str(int(i)).zfill(2))

        # Create text element
        text = inkex.etree.Element(inkex.addNS('text', 'svg'))
        text.text = u'Ø' + str(round(self.options.diameter, 2)) + \
            ' x ' + str(round(10, 2)) + \
            ' Sectors ' + str(360.0 / SEGMENTS) + u'°' \
            ' (todo - add layer height here)'

        # Set text position to center of document.
        width = self.unittouu(root.attrib['width'])
        height = self.unittouu(root.attrib['height'])
        text.set('x', str(width / 2))
        text.set('y', str(height / 2))

        # Center text horizontally with CSS style.
        style = {'text-align': 'center',
                 'text-anchor': 'middle',
                 'font-family': 'Droid Sans',
                 'font-size': '3',
                 }
        text.set('style', simplestyle.formatStyle(style))

        # Connect elements together.
        layer.append(text)


effect = ring_generator()
effect.affect()
