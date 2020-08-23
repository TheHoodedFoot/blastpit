#!/usr/bin/env python3

import argparse
import binascii
import math
import socket
import struct
import sys
import time

import xml.etree.ElementTree as ET

from PIL import Image

LISTEN_PORT_TCP = 1030
LISTEN_PORT_UDP = 1031


class bpHIDEvent:
    HID_AXIS_X, HID_AXIS_Y, HID_AXIS_ROT, HID_AXIS_JOG, HID_BUTTON = range(
        0, 5)


class BpCommand:
    kAddLocalQpSet, kBadCommand, kCancelJob, kExit, kImportXML, kInitMachine, kLayerSetHeightZAxis, kLayerSetLaserable, kMoveW, kMoveZ, kReadByte, kReadIOBit, kReference, kSaveVLM, kSetDimension, kSetMOLayer, kSetPosValues, kSetQualityParam, kStartMarking, kStartPosHelp, kStatus, kStopMarking, kStopPosHelp, kTermMachine, kWriteByte, kWriteIOBit = range(
        0, 26)


def headerCrc(id, size, command, csum):
    ba = bytearray(id.to_bytes(4, sys.byteorder))
    ba.extend(bytearray(size.to_bytes(4, sys.byteorder)))
    ba.extend(bytearray(command.to_bytes(4, sys.byteorder)))
    ba.extend(bytearray(csum.to_bytes(4, sys.byteorder)))
    return binascii.crc32(ba)


def sendBytes(id, cmd, data):
    dlen = len(data)
    ba = data
    crc = binascii.crc32(ba)
    s.send(
        struct.pack(
            'IIIII' + str(dlen) + 's',
            id, dlen, cmd, crc, headerCrc(id, dlen, cmd, crc), ba))


parser = argparse.ArgumentParser(description='Usage: send.py [server]')
parser.add_argument(
    '-D',
    '--dryrunoff',
    help='Deactivate positioning help',
    action='store_true')
parser.add_argument(
    '-L',
    '--laserable',
    help='Set the layer laserable parameter',
    nargs=1)
parser.add_argument(
    '-M',
    '--dimension',
    help='Set an object dimensions',
    nargs=2)
parser.add_argument('-S', '--save', help='Save VLM', nargs=1)
parser.add_argument(
    '-c',
    '--cancel',
    help='Cancel marking',
    action='store_true')
parser.add_argument(
    '-d',
    '--dryrun',
    help='Activate positioning help',
    action='store_true')
parser.add_argument('-H', '--height', help='Set the layer height', nargs=1)
parser.add_argument(
    '-i',
    '--init',
    help='Init Machine',
    required=False,
    action='store_true')
parser.add_argument(
    '-l',
    '--layer',
    nargs=1,
    help='Set the layer of a marking object')
parser.add_argument('-m', '--mark', help='Start Marking', action='store_true')
parser.add_argument('-b', '--bitmap', help='Upload bitmap', nargs=1)
parser.add_argument('-o', '--object', nargs=1, help='Select marking object')
parser.add_argument(
    '-p',
    '--position',
    help='Set an object position and rotation',
    nargs=3)
parser.add_argument('-q', '--qpset', help='Add a local QP set', nargs=4)
parser.add_argument(
    '-r',
    '--reference',
    help='Reference Axes',
    action='store_true')
parser.add_argument('-s', '--server', help='Server IP address', required=False)
parser.add_argument(
    '-u',
    '--upload',
    help='Upload Geometries',
    action='store_true')
parser.add_argument('-w', '--wmove', help='Move the w axis', nargs=1)
parser.add_argument(
    '-x',
    '--exit',
    help='Shutdown the blastpit server',
    action='store_true')
parser.add_argument('-z', '--zmove', help='Move the z axis', nargs=1)
args = parser.parse_args()

drawing = ET.Element("DRAWING")
drawing.set("UNIT", "MM")

#surface = ET.SubElement(drawing, "SURFACE")
#cylinder = ET.SubElement(surface, "CYLINDER")
#cylinder.set("RADIUS", "20")
# str(120/(2*math.pi)))
#cylinder.set("SECTOR", "60.0")
#axis = ET.SubElement(cylinder, "AXIS")
#axis.text = "1.0 0.0 0.0"
#origin = ET.SubElement(cylinder, "ORIGIN")
#origin.text = "0.0 0.0 -60.0"

root = ET.SubElement(drawing, "ROOT")
root.set("ID", "test")
root.set("WIDTH", "120.0")
root.set("HEIGHT", "120.0")

layer = ET.SubElement(root, "LAYER")
layer.set("ZCOLOR", "127,127,127")
layer.set("HEIGHT_Z_AXIS", "100")
layer.set("NAME", "mylayer")

#group = ET.SubElement(root, "GROUP")
#group.set("ID", "groupid")
#group.set("WIDTH", "50")
#group.set("HEIGHT", "50")
#group.set("OFFSET_X", "0")
#group.set("OFFSET_Y", "0")
#group.text = "POLYLINE0001"

# Geometries
arc = ET.SubElement(root, "ARC")
arc.set("ID", "ARC01")
arc.set("LAYER", "mylayer")

# Start Point
point = ET.SubElement(arc, "POINT")
point.text = "50.0 60.0"
# End Point
point = ET.SubElement(arc, "POINT")
point.text = "70.0 60.0"
# Centre Point
point = ET.SubElement(arc, "POINT")
point.text = "60.0 70.0"
direction = ET.SubElement(arc, "DIRECTION")
direction.text = "CW"

# polyline = ET.SubElement(root, "POLYLINE")
# polyline.set("ID", "POLYLINE0001")
# polyline.set("HATCH", "Y")
# #polyline.set("HATCH_ANGLE", "45")
# polyline.set("HP", "Blastpit")
# #polyline.set("DESCRIPTION", "mylayer[100.0]")
# polyline.set("LP", "Blastpit")

# polypoint = ET.SubElement(polyline, "POLYPOINT")
# polypoint.set("TYPE", "LINE")
# #polypoint.set("DIRECTION", "CW")
# polypoint.text = "25.0 25.0"
# polypoint = ET.SubElement(polyline, "POLYPOINT")
# polypoint.set("TYPE", "LINE")
# polypoint.text = "25.0 35.0"
# polypoint = ET.SubElement(polyline, "POLYPOINT")
# polypoint.set("TYPE", "LINE")
# polypoint.text = "35.0 35.0"
# polypoint = ET.SubElement(polyline, "POLYPOINT")
# polypoint.set("TYPE", "LINE")
# polypoint.text = "35.0 25.0"
# polypoint = ET.SubElement(polyline, "POLYPOINT")
# polypoint.set("TYPE", "LINE")
# polypoint.text = "25.0 25.0"

# Finish and save
XML = ET.tostring(drawing).decode("UTF-8")
XML = XML.replace("ZCOLOR", "COLOR")
XML = XML.encode("ascii")

# tree = ET.ElementTree(drawing)
#tree.write( "xml.xml" )

# drawing = ET.Element("svg")
# circle = ET.SubElement(drawing, "circle")
# circle.set("r", "10.0")
# circle.set("cy", "60.0")
# circle.set("cx", "60.0")
# circle.set("id", "circle0001")
# circle.set("style", "opacity:1;fill:none;fill-opacity:1;stroke:#000000;stroke-width:1.35399997;stroke-linecap:butt;stroke-linejoin:round;stroke-miterlimit:4;stroke-dasharray:none;stroke-dashoffset:0;stroke-opacity:1")
# svg = ET.tostring(drawing)

# with open('test.svg', 'rb') as file:
#     svg = file.read()

# Datadump
# if args.datadump:
# 	print("Receiving data...")

# 	HOST = ''                 # Symbolic name meaning all available interfaces
# 	PORT = 1030              # Arbitrary non-privileged port
# 	s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
# 	s.bind((HOST, PORT))
# 	s.listen(1)
# 	conn, addr = s.accept()
# 	print('Connected by', addr)
# 	while True:
# 		data = conn.recv(1024)
# 		if not data: break
# 		print(data)
# 	conn.close()
# 	sys.exit()

if args.server:
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.connect((args.server, LISTEN_PORT_TCP))

# Init Machine
if args.init:
    print("Initialising...")
    sendBytes(9876, BpCommand.kInitMachine, bytes())

# Reference Axes
if args.reference:
    print("Referencing axes...")
    s.send(struct.pack('iii', 1, BpCommand.kReference, 0))

# Layer
if args.object and args.layer:
    print("Setting marking object layer...")
    setmolayer = ET.Element("SetMOLayer")
    setmolayer.set("object", args.object[0])
    # RofinStandard RofinBackground Background Alternate
    setmolayer.set("layer", args.layer[0])
    tree = ET.ElementTree(setmolayer)
    root = tree.getroot()
    XML = ET.tostring(root, encoding='utf8', method='xml')

    print(XML)
    s.send(struct.pack('iii' + str(len(XML)) + 's',
                       1, BpCommand.kSetMOLayer, len(XML), XML))

# Dimension
if args.object and args.dimension:
    print("Setting marking object dimensions...")
    setdimension = ET.Element("SetDimension")
    setdimension.set("object", args.object[0])
    setdimension.set("x", args.dimension[0])
    setdimension.set("y", args.dimension[1])
    tree = ET.ElementTree(setdimension)
    root = tree.getroot()
    XML = ET.tostring(root, encoding='utf8', method='xml')

    print(XML)
    s.send(struct.pack('iii' + str(len(XML)) + 's',
                       1, BpCommand.kSetDimension, len(XML), XML))

# Position
if args.object and args.position:
    print("Setting marking object position...")
    setposvalues = ET.Element("SetPosValues")
    setposvalues.set("object", args.object[0])
    setposvalues.set("x", args.position[0])
    setposvalues.set("y", args.position[1])
    setposvalues.set("r", args.position[2])
    tree = ET.ElementTree(setposvalues)
    root = tree.getroot()
    XML = ET.tostring(root, encoding='utf8', method='xml')

    print(XML)
    s.send(struct.pack('iii' + str(len(XML)) + 's',
                       1, BpCommand.kSetPosValues, len(XML), XML))

# Upload test geometry
if args.upload:
    print(XML)
    print("Uploading XML...")
    sendBytes(1234, BpCommand.kImportXML, bytes(XML))

# Upload bitmap
if args.bitmap:
    im = Image.open(args.bitmap[0])
    # im = im.rotate(90)
    pixelMap = im.load()

    xfactor = 0.05
    yfactor = 0.05

    for x in range(im.size[0]):
        # Reset row
        lastYPixel = -1
        for y in range(im.size[1]):
            #print("Pixel: " + str(x) + "," + str(y))
            if pixelMap[x, y] == 1:
                if lastYPixel == -1:
                    lastYPixel = y
                if y == im.size[1] - 1:
                    # We have hit the bottom, so the line ends here
                    y1 = lastYPixel
                    y2 = y - 1
                    if y1 == y2:
                        # For single pixels, create a small line instead of a
                        # point
                        y1 = y1 - 0.25
                        y2 = y2 + 0.25
                    polyline = ET.SubElement(root, "POLYLINE")
                    #polyline.set("ID", "POLYLINE0001")
                    polypoint = ET.SubElement(polyline, "POLYPOINT")
                    polypoint.set("TYPE", "LINE")
                    polypoint.text = str(
                        round(x * xfactor, 4)) + " " + str(round(y1 * yfactor, 4))
                    polypoint = ET.SubElement(polyline, "POLYPOINT")
                    polypoint.set("TYPE", "LINE")
                    polypoint.text = str(
                        round(x * xfactor, 4)) + " " + str(round((y2 + 1) * yfactor, 4))
            else:
                if lastYPixel != -1:
                    y1 = lastYPixel
                    y2 = y - 1
                    if y1 == y2:
                        # For single pixels, create a small line instead of a
                        # point
                        y1 = y1 - 0.25
                        y2 = y2 + 0.25
                    polyline = ET.SubElement(root, "POLYLINE")
                    #polyline.set("ID", "POLYLINE0001")
                    polypoint = ET.SubElement(polyline, "POLYPOINT")
                    polypoint.set("TYPE", "LINE")
                    polypoint.text = str(
                        round(x * xfactor, 4)) + " " + str(round(y1 * yfactor, 4))
                    polypoint = ET.SubElement(polyline, "POLYPOINT")
                    polypoint.set("TYPE", "LINE")
                    polypoint.text = str(
                        round(x * xfactor, 4)) + " " + str(round((y2 - 1) * yfactor, 4))
                    lastYPixel = -1

    # Finish and save
    XML = ET.tostring(drawing).decode("UTF-8")
    XML = XML.replace("ZCOLOR", "COLOR")
    XML = XML.encode("ascii")

    tree = ET.ElementTree(drawing)
    print("Uploading XML...")
    s.send(struct.pack('iii' + str(len(XML)) + 's',
                       1, BpCommand.kImportXML, len(XML), XML))

    tree = ET.ElementTree(drawing)
    print("Saving XML...")
    tree.write("/tmp/xml.xml")


# if args.svg:
#     s.send( struct.pack( 'iii' + str(len(svg)) + 's', 1, BP_COMMAND.BP_LOAD_SVG, len(svg), svg ))

# if args.zoom:
#     s.send( struct.pack( 'iiicccc', 1, BP_COMMAND.BP_WINDOW_ZOOM, 4, bytes([40]), bytes([80]), bytes([80]), bytes([40]) ))

# if args.shadow:
#     s.send( struct.pack( 'iiicccc', 1, BP_COMMAND.BP_RESIZE_SHADOW, 4, bytes([int(args.shadow[0])]), bytes([int(args.shadow[1])]), bytes([80]), bytes([40]) ))

# Move Z
#s.send( struct.pack( 'iii3s', 1, 11, 3, b'60\0'))

# Start Positioning Help
if args.dryrun:
    s.send(struct.pack('iii', 1, BpCommand.kStartPosHelp, 0))

# Start Positioning Help
if args.dryrunoff:
    s.send(struct.pack('iii', 1, BpCommand.kStopPosHelp, 0))

# for x in range(0, 10):
    #s.send( struct.pack( 'iii', 1, 9, 0 ))
    # time.sleep(0.3)
# time.sleep(0.5)


# Next Positioning Help
# if args.next:
#     s.send( struct.pack( 'iii', 1, BP_COMMAND.BP_POSITIONING_HELP_NEXT, 0 ))

# Previous Positioning Help
# if args.previous:
#     s.send( struct.pack( 'iii', 1, BP_COMMAND.BP_POSITIONING_HELP_PREVIOUS, 0 ))

# Stop Positioning Help
#s.send( struct.pack( 'iii', 1, 12, 0 ))

# Start Marking
if args.mark:
    print("Start Marking")
    s.send(struct.pack('iii', 1, BpCommand.kStartMarking, 0))

# Save VLM
if args.save:
    print("Saving VLM...")
    time.sleep(0.5)
    filename = args.save[0].encode("ascii")
    s.send(struct.pack('iii' + str(len(filename)) + 's',
                       1, BpCommand.kSaveVLM, len(filename), filename))
    # s.send( struct.pack( 'iii', 1, BpCommand.kSaveVLM, 0 ))

# Z Axis
if args.zmove:
    print("Moving Z Axis...")
    time.sleep(0.5)

    movez = ET.Element("MoveZ")
    movez.set("z", args.zmove[0])
    tree = ET.ElementTree(movez)
    root = tree.getroot()
    XML = ET.tostring(root, encoding='utf8', method='xml')

    print(XML)
    s.send(struct.pack('iii' + str(len(XML)) + 's',
                       1, BpCommand.kMoveZ, len(XML), XML))

# W Axis
if args.wmove:
    print("Moving W Axis...")
    time.sleep(0.5)

    movew = ET.Element("MoveW")
    movew.set("w", args.wmove[0])
    tree = ET.ElementTree(movew)
    root = tree.getroot()
    XML = ET.tostring(root, encoding='utf8', method='xml')

    print(XML)
    s.send(struct.pack('iii' + str(len(XML)) + 's',
                       1, BpCommand.kMoveW, len(XML), XML))

# Shutdown
if args.exit:
    print("Shutting down blastpit...")
    sendBytes(9876, BpCommand.kTermMachine, bytes())

# Test
# if args.test:
#     print("Testing...")
#     s.send( struct.pack( 'iii', 1, BP_COMMAND.BP_TEST, 0 ))

# SVG
# if args.svg:
#     print("Uploading SVG...")
#     s.send( struct.pack( 'iii', 1, BP_COMMAND.BP_LOAD_SVG, 0 ))

# struct bpPacket {
    # uint32_t	id;
    # uint32_t	command;
    # uint32_t	length;
    # char		data[];
# };

# union bpHIDVal {
    # int	iVal;
    # float	fVal;
# };

# struct bpHID {
    # uint32_t	event;
    # union bpHIDVal	value;
# };
