#!/usr/bin/env python3

import socket
import struct
import binascii
import sys
import time

LISTEN_PORT_TCP = 1030


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


s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.connect(("localhost", LISTEN_PORT_TCP))

sendBytes(1234, BpCommand.kInitMachine, bytes([0x01, 0x00, 0x00, 0x00]))
time.sleep(0.25)
sendBytes(5678, BpCommand.kStartMarking, bytes([0x0f]))
time.sleep(0.25)
sendBytes(9876, BpCommand.kTermMachine, bytes())
time.sleep(0.25)
sendBytes(3456, BpCommand.kExit, bytes())
