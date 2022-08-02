#!/usr/bin/env python

import os.path
import sys
import unittest

# Configuration variables
if not os.path.isfile(os.getcwd() + "/src/scaffolding/autoshadow.py"):
    print("The autoshadow module is not available.")
    exit(1)
sys.path.append(os.getcwd() + "/src/scaffolding")

import autoshadow


class t_Autoshadow(unittest.TestCase):
    def test_Simple(self):
        """Test discrete paths with no overlapping"""

        MAXWIDTH = 2.0
        TOLERANCE = 0

        # Path name, x, y, width, height
        rawpaths = [
            ["path01", 10, 99, 0.5, 99],
            ["path02", 11, 99, 1.0, 99],
            ["path02", 12, 99, 2.0, 99],
        ]

        expectedshadows = [[10.0, 2.0], [12.0, 2.0]]

        # shadows = [['GalvoGroupShadow101'],
        #            ['GalvoGroupShadow102'],
        #            ['GalvoGroupShadow103'],
        #            ['GalvoGroupShadow104'],
        #            ['GalvoGroupShadow105']]

        ash = autoshadow.Autoshadow(maxwidth=MAXWIDTH, tolerance=TOLERANCE)

        # Solve
        mergedpaths = ash.mergeRawPaths(rawpaths)
        solvedshadows = ash.calculateOptimalPaths(mergedpaths)

        self.assertEqual(solvedshadows, expectedshadows)

    def test_internalPaths(self):
        """Ensure paths within other paths are merged"""

        MAXWIDTH = 2.0
        TOLERANCE = 0

        # Path name, x, y, width, height
        rawpaths = [
            ["path01", 10, 99, 1.0, 99],
            ["path02", 10.2, 99, 0.8, 99],
            ["path02", 10.8, 99, 0.2, 99],
        ]

        expectedshadows = [[10.0, 1.0]]

        ash = autoshadow.Autoshadow(maxwidth=MAXWIDTH, tolerance=TOLERANCE)

        # Solve
        mergedpaths = ash.mergeRawPaths(rawpaths)
        solvedshadows = ash.calculateOptimalPaths(mergedpaths)

        self.assertEqual(solvedshadows, expectedshadows)

    def test_removeOversizePaths(self):
        """Test for rejection of oversize paths"""

        MAXWIDTH = 2.0
        TOLERANCE = 0

        # Path name, x, y, width, height
        rawpaths = [[10, 1.0]]

        expectedshadows = [[10.0, 1.0]]

        ash = autoshadow.Autoshadow(maxwidth=MAXWIDTH, tolerance=TOLERANCE)

        # Solve
        noOversizePaths = ash.removeOversizePaths(rawpaths)

        self.assertEqual(ash.roundPaths(noOversizePaths), expectedshadows)

    def test_mergeNeighbouringPaths(self):
        """Test that overlapping paths can be merged"""

        MAXWIDTH = 2.0
        TOLERANCE = 0

        # Path name, x, y, width, height
        rawpaths = [[10, 1.0]]

        expectedshadows = [[10.0, 1.0]]

        ash = autoshadow.Autoshadow(maxwidth=MAXWIDTH, tolerance=TOLERANCE)

        # Solve
        mergedPaths = ash.mergeNeighbouringPaths(rawpaths)

        self.assertEqual(ash.roundPaths(mergedPaths), expectedshadows)

    def test_removeOverlappingPaths(self):
        """What does this do?"""

        MAXWIDTH = 2.0
        TOLERANCE = 0

        # Path name, x, y, width, height
        rawpaths = [[10, 1.0], [10.2, 0.8], [10.8, 0.2]]

        expectedshadows = [[10.0, 1.0]]

        ash = autoshadow.Autoshadow(maxwidth=MAXWIDTH, tolerance=TOLERANCE)

        # Solve
        noOverlappingPaths = ash.removeOverlappingPaths(rawpaths)

        self.assertEqual(noOverlappingPaths, expectedshadows)

    def test_removeInternalPaths(self):
        """Ensure paths within other paths are merged"""

        MAXWIDTH = 3.0
        TOLERANCE = 0

        # Path name, x, y, width, height
        rawpaths = [[10, 1.0], [10.2, 0.8], [10.8, 0.2]]

        expectedshadows = [[10.0, 1.0]]

        ash = autoshadow.Autoshadow(maxwidth=MAXWIDTH, tolerance=TOLERANCE)

        # Solve
        noInternalPaths = ash.removeInternalPaths(rawpaths)

        self.assertEqual(noInternalPaths, expectedshadows)

    def test_warnOnMissingShadows(self):
        """Ensure user is warned when geometries have no shadow"""

        MAXWIDTH = 3.0
        TOLERANCE = 0

        # Path name, x, y, width, height
        rawpaths = [[10, 1.0], [10.2, 0.8], [10.8, 4], [14, 4], [17, 3.1]]

        ash = autoshadow.Autoshadow(maxwidth=MAXWIDTH, tolerance=TOLERANCE)

        # Solve
        self.assertEqual(
            0,
            len(
                ash.findPathsWithoutShadows(
                    rawpaths, ash.calculateOptimalPaths(rawpaths)
                )
            ),
        )

    # def test_overlappingPaths(self):
    #     """ Ensure overlapping paths are allocated shadows """

    #     MAXWIDTH = 1.5
    #     TOLERANCE = 0

    #     # Path name, x, y, width, height
    #     rawpaths = [[12.4671, 1.7018],
    #                 [13.0037, 0.6223],
    #                 [14.7341, 1.2414],
    #                 [13.7892, 1.3652]]

    #     expectedshadows = [[12.46, 2.69],
    #                        [14.73, 1.25]]

    #     ash = autoshadow.Autoshadow(maxwidth=MAXWIDTH, tolerance=TOLERANCE)

    #     # Solve
    #     overlappingPaths = ash.calculateOptimalPaths(rawpaths)

    #     self.assertEqual(overlappingPaths, expectedshadows)

    # def test_multipleShadows(self):
    #     """ Expect several show passes """

    #     MAXWIDTH = 1.5
    #     TOLERANCE = 0

    #     # Path name, x, y, width, height
    #     rawpaths = [[0.0, 1.0],
    #                 [0.5, 1.0],
    #                 [1.0, 1.0],
    #                 [1.5, 1.0],
    #                 [2.0, 2.0]]

    #     expectedshadows = [[[0.0, 1.5],
    #                         [1.5, 1.0]],
    #                        [[1.0, 1.0]],
    #                        [[2.0, 1.0],
    #                         [3.0, 1.0]]]

    #     ash = autoshadow.Autoshadow(maxwidth=MAXWIDTH, tolerance=TOLERANCE)

    #     # Solve
    #     overlappingPaths = ash.calculateOptimalPaths(rawpaths)

    #     self.assertEqual(overlappingPaths, expectedshadows)


if __name__ == "__main__":
    unittest.main(failfast=True)


# assertEqual(a, b) 	a == b
# assertNotEqual(a, b) 	a != b
# assertTrue(x) 	bool(x) is True
# assertFalse(x) 	bool(x) is False
# assertIs(a, b) 	a is b 	3.1
# assertIsNot(a, b) 	a is not b 	3.1
# assertIsNone(x) 	x is None 	3.1
# assertIsNotNone(x) 	x is not None 	3.1
# assertIn(a, b) 	a in b 	3.1
# assertNotIn(a, b) 	a not in b 	3.1
# assertIsInstance(a, b) 	isinstance(a, b) 	3.2
# assertNotIsInstance(a, b) 	not isinstance(a, b) 	3.2

# vim: foldlevelstart=2
