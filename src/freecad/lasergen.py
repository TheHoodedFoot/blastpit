import blastpy
import sys
# import time

import Part
import DraftGeomUtils
# import FreeCADGui

import colorsys

from os.path import expanduser
sys.path.append(expanduser("~") + "/usr/src/blastpit/src/blastpy")


class LaserGeo:
    kStartX, kStartY, kEndX, kEndY, kMidX, kMidY, kHeightZ = range(0, 7)


def RoundGeos(Geos):
    Rounded = []
    for Geo in Geos:
        if Geo[4] is not None:
            Rounded.append([
                round(Geo[0], 6),
                round(Geo[1], 6),
                round(Geo[2], 6),
                round(Geo[3], 6),
                round(Geo[4], 6),
                round(Geo[5], 6),
                round(Geo[6], 6)])
        else:
            Rounded.append([
                round(Geo[0], 6),
                round(Geo[1], 6),
                round(Geo[2], 6),
                round(Geo[3], 6),
                None,
                None,
                round(Geo[6], 6)])
    return Rounded


def GenerateGeos(Selection, Geos):
    for Object in Selection:
        for Edge in Object.Shape.Edges:
            Curve = Edge.Curve

            if isinstance(Curve, Part.Line):
                Geos.append([
                    Edge.Vertexes[0].X,
                    Edge.Vertexes[0].Y,
                    Edge.Vertexes[1].X,
                    Edge.Vertexes[1].Y,
                    None,
                    None,
                    Edge.Vertexes[0].Z])

            if isinstance(Curve, Part.Circle):
                if len(Edge.Vertexes) == 2:
                    mid = DraftGeomUtils.findMidpoint(Edge)
                    Geos.append([
                        Edge.Vertexes[0].X,
                        Edge.Vertexes[0].Y,
                        Edge.Vertexes[1].X,
                        Edge.Vertexes[1].Y,
                        mid[0],
                        mid[1],
                        Edge.Vertexes[0].Z])
                else:
                    # Circle
                    circle = Curve.discretize(5)
                    Geos.append([
                        circle[0][0],
                        circle[0][1],
                        circle[2][0],
                        circle[2][1],
                        circle[1][0],
                        circle[1][1],
                        Edge.Vertexes[0].Z])
                    Geos.append([
                        circle[2][0],
                        circle[2][1],
                        circle[4][0],
                        circle[4][1],
                        circle[3][0],
                        circle[3][1],
                        Edge.Vertexes[0].Z])

            if isinstance(Curve, Part.BSplineCurve):
                try:
                    biArcs = Curve.toBiArcs(0.01)
                    for arc in biArcs:
                        SplineEdge = Part.Edge(arc)
                        if len(SplineEdge.Vertexes) == 2:
                            mid = DraftGeomUtils.findMidpoint(SplineEdge)
                            Geos.append([
                                SplineEdge.Vertexes[0].X,
                                SplineEdge.Vertexes[0].Y,
                                SplineEdge.Vertexes[1].X,
                                SplineEdge.Vertexes[1].Y,
                                mid[0],
                                mid[1],
                                SplineEdge.Vertexes[0].Z])
                        else:
                            # Circle
                            circle = Curve.discretize(5)
                            Geos.append([
                                circle[0][0],
                                circle[0][1],
                                circle[2][0],
                                circle[2][1],
                                circle[1][0],
                                circle[1][1],
                                SplineEdge.Vertexes[0].Z])
                            Geos.append([
                                circle[2][0],
                                circle[2][1],
                                circle[4][0],
                                circle[4][1],
                                circle[3][0],
                                circle[3][1],
                                SplineEdge.Vertexes[0].Z])
                except BaseException:
                    print("An exception occurred")
                    pass


def CreateLayers(geos, xml):
    zheights = []
    for geo in geos:
        zheights.append(round(geo[6], 1))

    zlayers = sorted(set(zheights), reverse=True)
    zmin = min(zlayers)
    zmax = max(zlayers)

    for i in zlayers:
        if zmin != zmax:
            colours = colorsys.hls_to_rgb(
                ((i - zmin) / (zmax - zmin)), 0.25, 0.8)
            xml.addLayer(
                str("%.1f" % i),
                colours[0] * 255,
                colours[1] * 255,
                colours[2] * 255,
                i)
        else:
            xml.addLayer(str("%.1f" % i), 0, 0, 0, i)


def GeneratePolylines(geos, xml):
    # print("GeneratePolylines: " + str(geos))
    pl = blastpy.BpPolyline("LINE", geos[0][0], geos[0][1])
    for geo in geos:
        if geo[4] is None:
            # Line
            pl.line(geo[2], geo[3])
        else:
            # Arc
            pl.arc(geo[4], geo[5], geo[2], geo[3])
    pl.setLayer(str("%.1f" % (round(geo[6], 1))))
    xml.addPolyline(pl)


def GeneratePolylinesG(geos, group, xml):
    # print("GeneratePolylinesG: " + str(geos))
    pl = blastpy.BpPolyline("LINE", geos[0][0], geos[0][1], "Undefined")
    for geo in geos:
        if geo[4] is None:
            # Line
            pl.line(geo[2], geo[3])
        else:
            # Arc
            pl.arc(geo[4], geo[5], geo[2], geo[3])
    # pl.setLayer(str("%.1f" % (round(geo[6], 2))))
    xml.addPolylineG(pl, group)


def removeSeparate(geos):
    # Find separate vertices
    validgeos = []
    for geo in geos:
        if geo[0] == geo[2] and geo[1] == geo[3] and geo[4] is None:
            print("Separate vertex: " + str(geo))
        else:
            validgeos.append(geo)
    return validgeos


def removeDuplicated(geos):
    validgeos = []
    for geo in geos:
        if geo not in validgeos:
            validgeos.append(geo)
    return validgeos


def join(geos):
    # print("join (start): " + str(geos))
    allpolys = []
    while geos:
        # print("Running join cycle ")
        # print("geos: " + str(geos))
        polygon = [geos[0]]
        geos = geos[1:]
        for i in range(0, len(geos)):
            for index, geo in enumerate(geos):
                if geo[0] == polygon[-1][2] and \
                    geo[1] == polygon[-1][3] and \
                        geo[6] == polygon[-1][6]:
                    polygon.append(geo)
                    del geos[index]
                    break
                elif geo[2] == polygon[-1][2] and \
                        geo[3] == polygon[-1][3] and \
                        geo[6] == polygon[-1][6]:
                    flip = [
                        geo[2],
                        geo[3],
                        geo[0],
                        geo[1],
                        geo[4],
                        geo[5],
                        geo[6]]
                    polygon.append(flip)
                    del geos[index]

        # print("Polygon: " + str(polygon))
        # print("geos: " + str(geos))
        if polygon[0][0] == polygon[-1][2] and \
                polygon[0][1] == polygon[-1][3] and \
                polygon[0][6] == polygon[-1][6]:
            allpolys.append(polygon)
    # print("join (end): " + str(allpolys))
    return allpolys


def process(xml, selection, lp=None, hp=None):
    # Get a handle to the currently selected object
    # sel = FreeCADGui.Selection.getSelection()

    geos = []
    GenerateGeos(selection, geos)
    geos = RoundGeos(geos)
    geos = removeDuplicated(geos)
    geos = removeSeparate(geos)
    polygons = join(geos)

    # xml = blastpy.BpXML()
    # CreateLayers(geos, xml)

    # print("Geos: " + str(geos))
    # sortedPolys = sorted(geos, key = operator.itemgetter(6))
    # print("Sortedpolys: " + str(sortedPolys))

    height = None
    group = None
    for polygon in polygons:
        # Is this a new height? If so,
        # create a layer and a group for it and put
        # all polygons of that height into that group
        if polygon is not None:
            if polygon[0][6] == height:
                GeneratePolylinesG(polygon, group, xml)
            else:
                height = polygon[0][6]
                group = xml.addGroup(
                    str(round(height, 1)),
                    str(round(height, 1)),
                    lp, hp)
                GeneratePolylinesG(polygon, group, xml)

    # print(xml.xml())
    return xml

    # Finish and save
    # print("Sending XML")
    # bp = blastpy.Blastpit("10.47.1.239", 1030)
    # bp = blastpy.Blastpit("10.53.69.11", 1030)
    # bp.init(1)
    # time.sleep(0.25)
    # bp.upload(2, xml.xml())
    # time.sleep(1)
    # bp.saveVLM("C:\\Users\\Rofin\\Desktop\\blastpit_save.VLM")


# export("/tmp/FreeCAD_Export.xml")


# Notes

# 1) Create a body containing a solid identical
# to the finished punch. Make it the actual height
# that it will finish at.
# 2) Create another body sitting on top of the punch body, completely
# encompassing the minimum area to be lasered away from the first body.
# 3) With the top (encompassing) body activated, select the final operation
# or pad in the model tree for that body (the top solid should turn green).
# In the part design workbench, select boolean, change the drop-down to cut,
# click 'add' (the top should vanish) then select the main punch body.
# The body should vanish and the top should reappear with the underside
# hollowed out. Alternatively, select boolean from the part workbench
# for a tree-style selection menu.
# 4) Select the boolean result in the explorer, go into the part workbench,
# select cross sections.

# vim: ft=python foldmethod=indent foldnestmax=2
