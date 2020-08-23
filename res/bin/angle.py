#!/usr/bin/env python2

import math
import sys

DIAMETER = sys.argv[1]
WIDTH = sys.argv[2]

print("Diameter: " + str(DIAMETER))
print("Width: " + str(WIDTH))

a = math.atan(float(WIDTH) / float(DIAMETER))
b = math.degrees(a)

if int(round(b)) == b:
    result = b
else:
    result = int(round(b / 5) * 5)

print "a = " + str(a)
print "b = " + str(b)
print "Angle = " + str(result)

c = math.radians(result)
e = math.sin(c)
d = math.cos(c)

print "d = " + str(d)
print "e = " + str(e)

print("Concave Axial vector: X: 0.0, Y: " +
      str(round(d, 3)) + ", Z: " + str(round(e, 3)))
print("Convex Axial vector: X: 0.0, Y: " +
      str(round(d, 3)) + ", Z: " + str(round(-e, 3)))
