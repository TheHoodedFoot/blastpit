#!/usr/bin/env python

# These two lines are only needed if you don't put the script directly into
# the installation directory
from simplestyle import formatStyle
import inkex
import sys
sys.path.append('/usr/share/inkscape/extensions')

# We will use the inkex module with the predefined Effect base class.
# The simplestyle module provides functions for style parsing.


class Laser(inkex.Effect):
    """
    Example Inkscape effect extension.
    Creates a new layer with a "Hello World!" text
    centered in the middle of the document.
    """

    def __init__(self):
        """
        Constructor.
        Defines the "--what" option of a script.
        """
        # Call the base class constructor.
        inkex.Effect.__init__(self)

        self.OptionParser.add_option(
            "-l",
            "--lparam",
            action="store",
            type="string",
            dest="lparam",
            default="dryrun",
            help="The laser parameter")

    def effect(self):
        """
        Effect behaviour.
        Overrides base class' method and inserts
        "Hello World" text into SVG document.
        """
        # Get access to main SVG document element and get its dimensions.
        svg = self.document.getroot()
        # or alternatively
        # svg = self.document.xpath('//svg:svg',namespaces=inkex.NSS)[0]

        # Again, there are two ways to get the attibutes:
        width = self.unittouu(svg.get('width'))
        height = self.unittouu(svg.attrib['height'])

        # Create a new layer.
        layer = inkex.etree.SubElement(svg, 'g')
        layer.set(
            inkex.addNS(
                'label',
                'inkscape'),
            'Hello %s Layer' %
            (self.options.lparam))
        layer.set(inkex.addNS('groupmode', 'inkscape'), 'layer')

        # Create text element
        text = inkex.etree.Element(inkex.addNS('text', 'svg'))
        text.text = 'Hello %s!' % (self.options.lparam)

        # Set text position to center of document.
        text.set('x', str(width / 2))
        text.set('y', str(height / 2))

        # Center text horizontally with CSS style.
        style = {'text-align': 'center', 'text-anchor': 'middle'}
        text.set('style', formatStyle(style))

        # Connect elements together.
        layer.append(text)

        inkex.debug(type(self.document.getroot()))


# Create effect instance and apply it.
effect = Laser()
effect.affect()

# To output a live Inkscape session, group together the items to be exported,
# save the file and use:
# inkscape --without-gui --export-text-to-path --export-plain-svg=<outfile>
#	 --file=<infile> --export-id=<groupid> --export-id-only
# with <groupid> being the id of the group we created
