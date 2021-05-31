# coding=utf-8
"""
Test titlecase extension
"""

# Inkex is not in the default Python search path
import sys

# import io

sys.path.append("/usr/share/inkscape/extensions")

# Needed until blastpy is installed system-wide
from os.path import expanduser

sys.path.append(expanduser("~") + "/projects/blastpit/src/inkscape")

from inkex.tester import TestCase
from example_extension import SvgToVlm


class LaserText(TestCase):
    effect_class = SvgToVlm

    def equality_test(self, files):
        for svgfile, xmlfile in files:
            with open(xmlfile, "r") as xml:
                assert self.effect.unittest(svgfile) == xml.read()

    def test_geometry_simple(self):
        """Checks the simplest single geometry with no groups."""
        files = [
            (
                ["src/inkscape/tests/simple_test.svg"],
                "src/inkscape/tests/simple_test.xml",
            )
        ]
        self.equality_test(files)

    def test_geometry_simple_transformed(self):
        """Checks translation of a simple geometry with no container group."""
        files = [
            (
                ["src/inkscape/tests/geometry_translate_test.svg"],
                "src/inkscape/tests/simple_test.xml",
            )
        ]
        self.equality_test(files)

    def test_group(self):
        """Checks a simple group."""
        files = [
            (
                ["src/inkscape/tests/group_test.svg"],
                "src/inkscape/tests/group_test.xml",
            )
        ]
        self.equality_test(files)

    def test_group_transformed(self):
        """Checks a simple group."""
        files = [
            (
                ["src/inkscape/tests/group_translate_test.svg"],
                "src/inkscape/tests/group_translate_test.xml",
            )
        ]
        self.equality_test(files)

    def test_functions(self):
        self.assertEqual(5, 5)
