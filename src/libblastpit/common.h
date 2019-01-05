#ifndef BPCOMMON_H
#define BPCOMMON_H

#ifdef _MSC_VER
#include "stdint.h"
#else
#include <stdint.h>
#endif

#define LISTEN_PORT 1030

enum BpReply {
	kNack,
	kAck,
};

enum BpCommand {
	kNoCommand = 0, /* For testing, always successful. */
	kBadCommand,    /* For testing, always fails. */

	kAddQpSet,
	kCancelJob,
	kClearQpSets,
	kExit,
	kImportXML,
	kInitMachine,
	kLayerSetExportable,
	kLayerSetHeightZAxis,
	kLayerSetLaserable,
	kLayerSetVisible,
	kLoadVLM,
	kMoveW,
	kMoveZ,
	kReadByte,
	kReadIOBit,
	kReference,
	kSaveVLM,
	kSelfTest,
	kSetDimension,
	kSetMOLayer,
	kSetPosValues,
	kSetQualityParam,
	kStartMarking,
	kStartPosHelp,
	kStatus,
	kStopMarking,
	kStopPosHelp,
	kTermMachine,
	kWriteByte,
	kWriteIOBit,
	kZoomWindow,
};

enum BpLogResult {
	kSuccess = 0,
	kFail,

	kNoEntry,
	kRunning,
	kStatusUnknown,
};

const int BpPriorityCmd[] = {
	kCancelJob, kStatus, kStopMarking, kStopPosHelp, kTermMachine,
};

enum BpHIDEvent {
	HID_AXIS_X,
	HID_AXIS_Y,
	HID_AXIS_ROT,
	HID_AXIS_JOG,
	HID_BUTTON,
};

enum BpLinetype {
	LineCONT = 0,
	LineDASH = 1,
	LineDOT = 2,
	LineDASHDOT = 3,
	LineCENTER = 4,
	LinePHANTOM = 5,
};

enum BpJustificationType {
	justifyLeft,
	justifyRight,
	justifyCentre,
};

enum BpError {
	kOK = 0,
	kErrXmlLoadFailed,
};

enum BpLog {
	kOff = 0,
	kCritical = 1,
	kError = 2,
	kWarn = 3,
	kInfo = 4,
	kDebug = 5
};

struct BpPacket {
	uint32_t id;
	uint32_t size;
	uint32_t command;
	uint32_t parentid;
	uint32_t dataChecksum;
	uint32_t packetChecksum;
};

struct BpLogEntry {
	uint32_t id;
	uint32_t command;
	uint32_t result;
};

#define BP_PACKET_CHECKSUM_SIZE 5 * sizeof(uint32_t)
#define BP_DATA_CHECKSUM_SIZE sizeof(struct BpPacket)

union BpHIDVal {
	int iVal;
	float fVal;
};

struct BpHID {
	uint32_t event;
	union BpHIDVal value;
};

union psHeader {
	unsigned char bytes[sizeof(struct BpPacket)];
	struct BpPacket header;
};

#define CLSID_LMOS "{18213698-A9C9-11D1-A220-0060973058F6}"
#endif /* end of include guard: BPCOMMON_H */
