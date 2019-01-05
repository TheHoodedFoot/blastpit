import binascii
import math
import socket
import struct
import time
import xml.etree.ElementTree as ET
import sys


class BpCommand:
    kNoCommand, \
        kBadCommand, kAddQpSet, kCancelJob, kClearQpSets, kExit, kImportXML, kInitMachine, kLayerSetExportable, kLayerSetHeightZAxis, kLayerSetLaserable, kLayerSetVisible, kLoadVLM, kMoveW, kMoveZ, kReadByte, kReadIOBit, kReference, kSaveVLM, kSelfTest, kSetDimension, kSetMOLayer, kSetPosValues, kSetQualityParam, kStartMarking, kStartPosHelp, kStatus, kStopMarking, kStopPosHelp, kTermMachine, kWriteByte, kWriteIOBit, kZoomWindow = range(0, 33)


class BpHIDEvent:
    HID_AXIS_X, HID_AXIS_Y, HID_AXIS_ROT, HID_AXIS_JOG, HID_BUTTON \
        = range(0, 5)


class BpPolypoint:
    kVertex, kArc, kBezier = range(0, 3)


class InPortConstants:
    Port1B, Port1C_In, Port1D, Port1E = range(0, 4)


class BpPolyline:

    # Polylines
    #
    # An arc is constructed with three points: a start vertex (LINE), a
    # midpoint (not centre) vertex (ARC), and an end vertex (LINE).
    #
    # A bezier is constructed with four points: a start vertex (LINE), two
    # bezier points (BEZIER) and a closing vertex (LINE).

    def __init__(self, name, x, y, lp, hp=None):
        self.points = [(x, y, BpPolypoint.kVertex)]
        self.name = name
        self.lp = lp
        if hp is not None:
            self.hp = hp
        else:
            self.hp = None

    def line(self, x, y=None):
        # Allow single x,y pair or array of pairs
        if y is None:
            for cx, cy in x:
                self.points.extend([(cx, cy, BpPolypoint.kVertex)])
        else:
            self.points.extend([(x, y, BpPolypoint.kVertex)])

    def arc(self, midx, midy, endx, endy):
        self.points.extend([(midx, midy, BpPolypoint.kArc)])
        self.points.extend([(endx, endy, BpPolypoint.kVertex)])

    def arcCentre(self, cenx, ceny, endx, endy):
        # Given the centre point, calculate the mid point

        # Retrieve the last point (start point)
        startx = self.points[-1][0]
        starty = self.points[-1][1]

        # Calculate the mid point
        x10 = startx - cenx
        y10 = starty - ceny
        x20 = endx - cenx
        y20 = endy - ceny
        a = 0.5 * (math.atan2(y20, x20) - math.atan2(y10, x10) % 2 * math.pi)
        midx = cenx + x10 * math.cos(a) - y10 * math.sin(a)
        midy = ceny + y10 * math.cos(a) + x10 * math.sin(a)

        self.arc(midx, midy, endx, endy)

    def bezier(self, cp1x, cp1y, cp2x, cp2y, endx, endy):
        self.points.extend([(cp1x, cp1y, BpPolypoint.kBezier)])
        self.points.extend([(cp2x, cp2y, BpPolypoint.kBezier)])
        self.points.extend([(endx, endy, BpPolypoint.kVertex)])

    def close(self):
        self.points.extend(self.points[:1])

    def getName(self):
        return str(self.name)

    def getStart(self):
        return self.points[0]

    def setLayer(self, layer):
        self.layer = layer

    def getLayer(self):
        if hasattr(self, 'layer'):
            return str(self.layer)

    def setHp(self, hp):
        self.hp = hp

    def getHp(self):
        if self.hp is not None:
            return str(self.hp)

    def setLp(self, lp):
        self.lp = lp

    def getLp(self):
        if self.lp is not None:
            return str(self.lp)


class BpXML:
    def __init__(self):
        self.drawing = ET.Element("DRAWING")
        self.drawing.set("UNIT", "MM")
        self.root = ET.SubElement(self.drawing, "ROOT")
        self.root.set("ID", "Blastpit")
        self.root.set("WIDTH", "120.0")
        self.root.set("HEIGHT", "120.0")

    def axialAngle(self, radius, width):
        # Find tilt (in degrees so we can round it next)
        tilt = math.degrees(math.atan(float(width) / float(radius * 2)))

        # Round up to nearest 5 degrees
        if int(round(tilt)) != tilt:
            tilt = int(round(tilt / 5) * 5)

        return tilt

    def axialVector(self, angle, convex=True):
        # Axial vector is X=0 Y = cos(tilt) Z=sin(tilt)
        if convex:
            return "0.0 " + str(math.cos(angle)) + " " + \
                str(math.sin(angle))
        else:
            return "0.0 " + str(math.cos(angle)) + " " + \
                str(-math.sin(angle))

    def setCylinder(self, radius, width, sector, convex=True):
        surface = ET.SubElement(self.drawing, "SURFACE")
        cylinder = ET.SubElement(surface, "CYLINDER")
        cylinder.set("RADIUS", str(radius))
        cylinder.set("SECTOR", str(sector))
        axis = ET.SubElement(cylinder, "AXIS")
        if convex:
            cylinder.set("CONVEX", 'Y')
            angle = 0
        else:
            cylinder.set("CONVEX", 'N')
            surface.set("USE_SURFACE_CORRECTION", "Y")
            angle = math.radians(self.axialAngle(radius, width))
        axis.text = self.axialVector(angle, convex)
        origin = ET.SubElement(cylinder, "ORIGIN")
        origin.text = "0.0 0.0 -120.0"

    def addLayer(self, name, height="120", r=0x80, g=0x80, b=0x80):
        for layer in self.root.findall("LAYER"):
            if layer.get("AAANAME") == name:
                return layer
        layer = ET.SubElement(self.root, "LAYER")
        layer.set("HEIGHT_Z_AXIS", str(height))
        layer.set(
            "COLOR",
            str(r % 0x100) + "," + str(g % 0x100) + "," + str(b % 0x100))
        layer.set("AAANAME", name)
        return layer

    def addPolyline(self, polyline):
        p = ET.SubElement(self.root, "POLYLINE")
        p.set("ID", polyline.getName())
        if polyline.getLayer() is not None:
            p.set("LAYER", polyline.getLayer())
        if polyline.getLp() is not None:
            p.set("LP", polyline.getLp())
        if polyline.getHp() is not None:
            p.set("HATCH", "Y")
            p.set("HP", polyline.getHp())
        pt = ET.SubElement(p, "POLYPOINT")
        pt.set("TYPE", "LINE")
        pt.text = str("{:.4f}").format(polyline.points[:1][0][0]) \
            + " " + str("{:.4f}").format(polyline.points[:1][0][1])
        for point in polyline.points[1:]:
            pt = ET.SubElement(p, "POLYPOINT")
            pt.text = str("{:.4f}").format(
                point[0]) + " " + str("{:.4f}").format(point[1])
            if point[2] is BpPolypoint.kVertex:
                pt.set("TYPE", "LINE")
            if point[2] == BpPolypoint.kArc:
                pt.set("TYPE", "ARC")
            if point[2] == BpPolypoint.kBezier:
                pt.set("TYPE", "BEZIER")

    def addPolylineG(self, polyline, group):
        p = ET.SubElement(group, "POLYLINE")
        p.set("ID", polyline.getName())
        if polyline.getLayer() is not None:
            p.set("LAYER", polyline.getLayer())
        if polyline.getLp() is not None:
            p.set("LP", polyline.getLp())
        if polyline.getHp() is not None:
            p.set("HATCH", "Y")
            p.set("HP", polyline.getHp())
        pt = ET.SubElement(p, "POLYPOINT")
        pt.set("TYPE", "LINE")
        pt.text = str(polyline.points[:1][0][0]) \
            + " " + str(polyline.points[:1][0][1])
        for point in polyline.points[1:]:
            pt = ET.SubElement(p, "POLYPOINT")
            pt.text = str(point[0]) + " " + str(point[1])
            if point[2] is BpPolypoint.kVertex:
                pt.set("TYPE", "LINE")
            if point[2] == BpPolypoint.kArc:
                pt.set("TYPE", "ARC")
            if point[2] == BpPolypoint.kBezier:
                pt.set("TYPE", "BEZIER")

    def addGroup(self, group, layer, lp, hp=None):
        g = ET.SubElement(self.root, "GROUP")
        g.set("ID", group)
        g.set("USE_BOX", "N")
        g.set("LAYER", layer)
        g.set("LP", lp)
        if hp is not None:
            g.set("HATCH", "Y")
            g.set("HP", hp)
        return g

    def xml(self):
        # The Rofin XML import is broken and ignores any attributes
        # if they are listed before the layer name attribute. Since pugixml
        # generates them in alphabetical order, Our fix is to force the layer
        # name to always be first by prefixing it with 'AAA' and then
        # renaming it afterwards.
        xml = ET.tostring(self.drawing).decode("UTF-8")
        xml = xml.replace("AAANAME", "NAME")
        xml = xml.encode("ascii")
        return xml


class Blastpit:
    def __init__(self, server, port):
        self.server = server
        self.port = port
        self.sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.sock.settimeout(1)
        self.sock.connect((server, port))

    def headerCrc(self, id, size, command, parentid, csum):
        if sys.version_info[0] < 3:
            # Python2
            ba = bytearray(struct.unpack('4B', struct.pack('i', id)))
            ba.extend(struct.unpack('4B', struct.pack('i', size)))
            ba.extend(struct.unpack('4B', struct.pack('i', command)))
            ba.extend(struct.unpack('4B', struct.pack('i', parentid)))
            ba.extend(struct.unpack('4B', struct.pack('i', csum)))
        else:
            # Python3
            ba = bytearray(id.to_bytes(4, sys.byteorder))
            ba.extend(bytearray(size.to_bytes(4, sys.byteorder)))
            ba.extend(bytearray(command.to_bytes(4, sys.byteorder)))
            ba.extend(bytearray(parentid.to_bytes(4, sys.byteorder)))
            ba.extend(bytearray(csum.to_bytes(4, sys.byteorder)))
        return binascii.crc32(ba)

    def sendTree(self, id, var, command):
        tree = ET.ElementTree(var)
        root = tree.getroot()
        XML = ET.tostring(root, encoding='ascii', method='xml')
        self.sendBytes(id, command, bytes(XML))

    def sendBytes(self, id, cmd, data, parent=0):
        dlen = len(data)
        ba = data
        crc = binascii.crc32(ba)
        if sys.version_info[0] < 3:
            # Python2
            totalsent = 0
            msg = struct.pack(
                'iiiiii' + str(dlen) + 's',
                id, dlen, cmd, parent, crc,
                self.headerCrc(id, dlen, cmd, parent, crc), ba)
            while totalsent < len(msg):
                sent = self.sock.send(msg[totalsent:])
                if sent == 0:
                    raise RuntimeError("socket connection broken")
                totalsent = totalsent + sent
        else:
            # Python3
            self.sock.sendall(
                struct.pack(
                    'IIIIII' + str(dlen) + 's',
                    id, dlen, cmd, parent, crc,
                    self.headerCrc(id, dlen, cmd, parent, crc), ba))

        # Receive
        self.sock.settimeout(1)
        MSGLEN = len(struct.pack('@II', 0, 0))
        ba = bytearray()
        while len(ba) < MSGLEN:
            chunk = self.sock.recv(min(MSGLEN - len(ba), 2048))
            if chunk == '':
                raise RuntimeError("socket connection broken")
            ba.extend(bytearray(chunk))
        if len(ba) == 8:
            di, res = struct.unpack('@II', ba)
            return [di, res]
        return False

    def flush(self):
        self.sock.close
        self.sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.sock.connect((self.server, self.port))

    def upload(self, id, xml, parent=None):
        upload = ET.Element("Blastpit")
        upload.set("drawing", xml)
        if parent is not None:
            upload.set("parent", parent)
        self.sendTree(id, upload, BpCommand.kImportXML)

    def init(self, id):
        self.sendBytes(id, BpCommand.kInitMachine, bytes())

    def finish(self, id):
        self.sendBytes(id, BpCommand.kExit, bytes())

    def reference(self, id):
        self.sendBytes(id, BpCommand.kReference, bytes())

    def terminate(self, id):
        self.sendBytes(id, BpCommand.kTermMachine, bytes())

    def cancelJob(self, id):
        self.sendBytes(id, BpCommand.kCancelJob, bytes())

    def stopPosHelp(self, id):
        self.sendBytes(id, BpCommand.kStopPosHelp, bytes())

    def startMarking(self, id, parent=None):
        startmarking = ET.Element("Blastpit")
        if parent is not None:
            startmarking.set("parent", parent)
        self.sendTree(id, startmarking, BpCommand.kStartMarking)

    def stopMarking(self, id):
        self.sendBytes(id, BpCommand.kStopMarking, bytes())

    def selfTest(self, id):
        self.sendBytes(id, BpCommand.kSelfTest, bytes())

    def saveVLM(self, id, filename, parent=None):
        savevlm = ET.Element("Blastpit")
        savevlm.set("filename", filename)
        if parent is not None:
            savevlm.set("parent", parent)
        self.sendTree(id, savevlm, BpCommand.kSaveVLM)

    def loadVLM(self, id, filename):
        loadvlm = ET.Element("Blastpit")
        loadvlm.set("filename", filename)
        self.sendTree(id, loadvlm, BpCommand.kLoadVLM)

    def startPosHelp(self, id, obj=None):
        startposhelp = ET.Element("Blastpit")
        if obj is not None:
            startposhelp.set("object", obj)
        self.sendTree(id, startposhelp, BpCommand.kStartPosHelp)

    def moveZ(self, id, height):
        movez = ET.Element("Blastpit")
        movez.set("height", str(height))
        self.sendTree(id, movez, BpCommand.kMoveZ)

    def moveW(self, id, rotation):
        movew = ET.Element("Blastpit")
        movew.set("rotation", str(rotation))
        self.sendTree(id, movew, BpCommand.kMoveW)

    def setMOLayer(self, id, obj, layer):
        setmolayer = ET.Element("Blastpit")
        setmolayer.set("object", obj)
        setmolayer.set("layer", layer)
        self.sendTree(id, setmolayer, BpCommand.kSetMOLayer)

    def setDimension(self, id, obj, x, y, parent=None):
        setdimension = ET.Element("Blastpit")
        setdimension.set("object", obj)
        setdimension.set("x", str(x))
        setdimension.set("y", str(y))
        if parent is not None:
            setdimension.set("parent", str(parent))
        self.sendTree(id, setdimension, BpCommand.kSetDimension)

    def setPosValues(self, id, obj, x, y, r, parent=None):
        setposvalues = ET.Element("Blastpit")
        setposvalues.set("object", obj)
        setposvalues.set("x", str(x))
        setposvalues.set("y", str(y))
        setposvalues.set("r", str(r))
        if parent is not None:
            setposvalues.set("parent", str(parent))
        self.sendTree(id, setposvalues, BpCommand.kSetPosValues)

    def addQpSet(self, id, name, current, speed, frequency, parent=None):
        addqpset = ET.Element("Blastpit")
        addqpset.set("name", name)
        addqpset.set("current", str(current))
        addqpset.set("speed", str(speed))
        addqpset.set("frequency", str(frequency))
        if parent is not None:
            addqpset.set("parent", str(parent))
        self.sendTree(id, addqpset, BpCommand.kAddQpSet)

    def clearQpSets(self, id):
        self.sendBytes(id, BpCommand.kClearQpSets, bytes())

    def layerSetLaserable(self, id, layer, val, parent=None):
        layersetlaserable = ET.Element("Blastpit")
        layersetlaserable.set("layer", layer)
        layersetlaserable.set("laserable", str(val))
        if parent is not None:
            layersetlaserable.set("parent", str(parent))
        self.sendTree(id, layersetlaserable, BpCommand.kLayerSetLaserable)

    def layerSetExportable(self, id, layer, val):
        layersetexportable = ET.Element("Blastpit")
        layersetexportable.set("layer", layer)
        layersetexportable.set("exportable", str(val))
        self.sendTree(id, layersetexportable, BpCommand.kLayerSetExportable)

    def layerSetVisible(self, id, layer, val):
        layersetvisible = ET.Element("Blastpit")
        layersetvisible.set("layer", layer)
        layersetvisible.set("visible", str(val))
        self.sendTree(id, layersetvisible, BpCommand.kLayerSetVisible)

    def layerSetHeight(self, id, layer, height, parent=None):
        layersetheight = ET.Element("Blastpit")
        layersetheight.set("layer", layer)
        layersetheight.set("height", str(height))
        if parent is not None:
            layersetheight.set("parent", str(parent))
        self.sendTree(id, layersetheight, BpCommand.kLayerSetHeightZAxis)

    def readIOBit(self, id, bitfunction):
        readiobit = ET.Element("Blastpit")
        readiobit.set("bitfunction", bitfunction)
        self.sendTree(id, readiobit, BpCommand.kReadIOBit)

    def writeIOBit(self, id, bitfunction, value):
        writeiobit = ET.Element("Blastpit")
        writeiobit.set("bitfunction", bitfunction)
        writeiobit.set("value", str(value))
        self.sendTree(id, writeiobit, BpCommand.kWriteIOBit)

    def readByte(self, id, port, mask):
        readbyte = ET.Element("Blastpit")
        readbyte.set("port", port)
        readbyte.set("mask", mask)
        self.sendTree(id, readbyte, BpCommand.kReadByte)

    def openDoor(self, id):
        self.writeIOBit(id, "CloseDoor", 0)
        time.sleep(0.25)
        self.writeIOBit(id, "OpenDoor", 1)

    def closeDoor(self, id):
        self.writeIOBit(id, "OpenDoor", 0)
        time.sleep(0.25)
        self.writeIOBit(id, "CloseDoor", 1)

    def lightOn(self, id):
        self.writeIOBit(id, "Light", 1)

    def lightOff(self, id):
        self.writeIOBit(id, "Light", 0)

    def zoomWindow(self, id, x1, y1, x2, y2):
        setposvalues = ET.Element("Blastpit")
        setposvalues.set("x1", str(x1))
        setposvalues.set("y1", str(y1))
        setposvalues.set("x2", str(x2))
        setposvalues.set("y2", str(y2))
        self.sendTree(id, setposvalues, BpCommand.kZoomWindow)


# vim: ft=python foldmethod=indent foldlevel=1 foldnestmax=2