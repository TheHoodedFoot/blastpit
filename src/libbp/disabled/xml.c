/*
 * =====================================================================================
 *
 *       Filename:  bpxml.c
 *
 *    Description:  Blastpit XML class
 *
 *        Version:  1.0
 *        Created:  12/01/19 18:42:03
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Andrew Black (), thf@thehoodedfoot.com
 *   Organization:
 *
 * =====================================================================================
 */

// class BpXML:
//     def __init__(self):
//         self.drawing = ET.Element("DRAWING")
//         self.drawing.set("UNIT", "MM")
//         self.root = ET.SubElement(self.drawing, "ROOT")
//         self.root.set("ID", "Blastpit")
//         self.root.set("WIDTH", "120.0")
//         self.root.set("HEIGHT", "120.0")

//     def axialAngle(self, radius, width):
//         # Find tilt (in degrees so we can round it next)
//         tilt = math.degrees(math.atan(float(width) / float(radius * 2)))

//         # Round up to nearest 5 degrees
//         if int(round(tilt)) != tilt:
//             tilt = int(round(tilt / 5) * 5)

//         return tilt

//     def axialVector(self, angle, convex=True):
//         # Axial vector is X=0 Y = cos(tilt) Z=sin(tilt)
//         if convex:
//             return "0.0 " + str(math.cos(angle)) + " " + \
//                 str(math.sin(angle))
//         else:
//             return "0.0 " + str(math.cos(angle)) + " " + \
//                 str(-math.sin(angle))

//     def setCylinder(self, radius, width, sector, convex=True):
//         surface = ET.SubElement(self.drawing, "SURFACE")
//         cylinder = ET.SubElement(surface, "CYLINDER")
//         cylinder.set("RADIUS", str(radius))
//         cylinder.set("SECTOR", str(sector))
//         axis = ET.SubElement(cylinder, "AXIS")
//         if convex:
//             cylinder.set("CONVEX", 'Y')
//             angle = 0
//         else:
//             cylinder.set("CONVEX", 'N')
//             surface.set("USE_SURFACE_CORRECTION", "Y")
//             angle = math.radians(self.axialAngle(radius, width))
//         axis.text = self.axialVector(angle, convex)
//         origin = ET.SubElement(cylinder, "ORIGIN")
//         origin.text = "0.0 0.0 -120.0"

//     def addLayer(self, name, height="120", r=0x80, g=0x80, b=0x80):
//         for layer in self.root.findall("LAYER"):
//             if layer.get("AAANAME") == name:
//                 return layer
//         layer = ET.SubElement(self.root, "LAYER")
//         layer.set("HEIGHT_Z_AXIS", str(height))
//         layer.set(
//             "COLOR",
//             str(r % 0x100) + "," + str(g % 0x100) + "," + str(b % 0x100))
//         layer.set("AAANAME", name)
//         return layer

//     def addPolyline(self, polyline):
//         p = ET.SubElement(self.root, "POLYLINE")
//         p.set("ID", polyline.getName())
//         if polyline.getLayer() is not None:
//             p.set("LAYER", polyline.getLayer())
//         if polyline.getLp() is not None:
//             p.set("LP", polyline.getLp())
//         if polyline.getHp() is not None:
//             p.set("HATCH", "Y")
//             p.set("HP", polyline.getHp())
//         pt = ET.SubElement(p, "POLYPOINT")
//         pt.set("TYPE", "LINE")
//         pt.text = str("{:.4f}").format(polyline.points[:1][0][0]) \
//             + " " + str("{:.4f}").format(polyline.points[:1][0][1])
//         for point in polyline.points[1:]:
//             pt = ET.SubElement(p, "POLYPOINT")
//             pt.text = str("{:.4f}").format(
//                 point[0]) + " " + str("{:.4f}").format(point[1])
//             if point[2] is BpPolypoint.kVertex:
//                 pt.set("TYPE", "LINE")
//             if point[2] == BpPolypoint.kArc:
//                 pt.set("TYPE", "ARC")
//             if point[2] == BpPolypoint.kBezier:
//                 pt.set("TYPE", "BEZIER")

//     def addPolylineG(self, polyline, group):
//         p = ET.SubElement(group, "POLYLINE")
//         p.set("ID", polyline.getName())
//         if polyline.getLayer() is not None:
//             p.set("LAYER", polyline.getLayer())
//         if polyline.getLp() is not None:
//             p.set("LP", polyline.getLp())
//         if polyline.getHp() is not None:
//             p.set("HATCH", "Y")
//             p.set("HP", polyline.getHp())
//         pt = ET.SubElement(p, "POLYPOINT")
//         pt.set("TYPE", "LINE")
//         pt.text = str(polyline.points[:1][0][0]) \
//             + " " + str(polyline.points[:1][0][1])
//         for point in polyline.points[1:]:
//             pt = ET.SubElement(p, "POLYPOINT")
//             pt.text = str(point[0]) + " " + str(point[1])
//             if point[2] is BpPolypoint.kVertex:
//                 pt.set("TYPE", "LINE")
//             if point[2] == BpPolypoint.kArc:
//                 pt.set("TYPE", "ARC")
//             if point[2] == BpPolypoint.kBezier:
//                 pt.set("TYPE", "BEZIER")

//     def addGroup(self, group, layer, lp, hp=None):
//         g = ET.SubElement(self.root, "GROUP")
//         g.set("ID", group)
//         g.set("USE_BOX", "N")
//         g.set("LAYER", layer)
//         g.set("LP", lp)
//         if hp is not None:
//             g.set("HATCH", "Y")
//             g.set("HP", hp)
//         return g

//     def xml(self):
//         # The Rofin XML import is broken and ignores any attributes
//         # if they are listed before the layer name attribute. Since pugixml
//         # generates them in alphabetical order, Our fix is to force the
//         layer # name to always be first by prefixing it with 'AAA' and then
//         # renaming it afterwards.
//         xml = ET.tostring(self.drawing).decode("UTF-8")
//         xml = xml.replace("AAANAME", "NAME")
//         xml = xml.encode("ascii")
//         return xml
