/*
 * =====================================================================================
 *
 *       Filename:  polyline.c
 *
 *    Description:  Blastpit polyline class
 *
 *        Version:  1.0
 *        Created:  12/01/19 18:40:51
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Andrew Black (), thf@thehoodedfoot.com
 *   Organization:
 *
 * =====================================================================================
 */

// class BpPolyline:

//     # Polylines
//     #
//     # An arc is constructed with three points: a start vertex (LINE), a
//     # midpoint (not centre) vertex (ARC), and an end vertex (LINE).
//     #
//     # A bezier is constructed with four points: a start vertex (LINE), two
//     # bezier points (BEZIER) and a closing vertex (LINE).

//     def __init__(self, name, x, y, lp, hp=None):
//         self.points = [(x, y, BpPolypoint.kVertex)]
//         self.name = name
//         self.lp = lp
//         if hp is not None:
//             self.hp = hp
//         else:
//             self.hp = None

//     def line(self, x, y=None):
//         # Allow single x,y pair or array of pairs
//         if y is None:
//             for cx, cy in x:
//                 self.points.extend([(cx, cy, BpPolypoint.kVertex)])
//         else:
//             self.points.extend([(x, y, BpPolypoint.kVertex)])

//     def arc(self, midx, midy, endx, endy):
//         self.points.extend([(midx, midy, BpPolypoint.kArc)])
//         self.points.extend([(endx, endy, BpPolypoint.kVertex)])

//     def arcCentre(self, cenx, ceny, endx, endy):
//         # Given the centre point, calculate the mid point

//         # Retrieve the last point (start point)
//         startx = self.points[-1][0]
//         starty = self.points[-1][1]

//         # Calculate the mid point
//         x10 = startx - cenx
//         y10 = starty - ceny
//         x20 = endx - cenx
//         y20 = endy - ceny
//         a = 0.5 * (math.atan2(y20, x20) - math.atan2(y10, x10) % 2 *
//         math.pi) midx = cenx + x10 * math.cos(a) - y10 * math.sin(a) midy =
//         ceny + y10 * math.cos(a) + x10 * math.sin(a)

//         self.arc(midx, midy, endx, endy)

//     def bezier(self, cp1x, cp1y, cp2x, cp2y, endx, endy):
//         self.points.extend([(cp1x, cp1y, BpPolypoint.kBezier)])
//         self.points.extend([(cp2x, cp2y, BpPolypoint.kBezier)])
//         self.points.extend([(endx, endy, BpPolypoint.kVertex)])

//     def close(self):
//         self.points.extend(self.points[:1])

//     def getName(self):
//         return str(self.name)

//     def getStart(self):
//         return self.points[0]

//     def setLayer(self, layer):
//         self.layer = layer

//     def getLayer(self):
//         if hasattr(self, 'layer'):
//             return str(self.layer)

//     def setHp(self, hp):
//         self.hp = hp

//     def getHp(self):
//         if self.hp is not None:
//             return str(self.hp)

//     def setLp(self, lp):
//         self.lp = lp

//     def getLp(self):
//         if self.lp is not None:
//             return str(self.lp)
