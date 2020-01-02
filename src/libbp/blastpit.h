#ifndef BLASTPIT_H
#define BLASTPIT_H

#ifdef _MSC_VER
#include "stdint.h"
#else
#include <stdint.h>
#endif

#define LISTEN_PORT 1030

#define BP_PACKET_CHECKSUM_SIZE 5 * sizeof(uint32_t)
#define BP_DATA_CHECKSUM_SIZE sizeof(struct BpPacket)

enum BpReply {
	kNack,
	kAck,
};

enum BpCommand {
	kNoCommand = 0, /* For testing, always successful. */
	kAddQpSet,
	kCancelJob,
	kClearQpSets,
	kExit,
	kGetPng,
	kImportXML,
	kInitMachine,
	kLayerSetExportable,
	kLayerSetHeight,
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
	kShowMarkingArea,
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

/* static const char * BpCommandName[]={ */
/* 	"kNoCommand", */
/* 	"kAddQpSet" , */
/* }; */

enum BpPolyPoint {
	kVertex = 0,
	kArc,
	kBezier,
};

enum InPortConstants {
	kPort1B = 0,
	kPort1C_In,
	kPort1D,
	kPort1E,
};

enum BpLogResult {
	kSuccess = 0,
	kFail,

	kNoEntry,
	kRunning,
	kStatusUnknown,
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

/* We need to hide our c class code from the c++ compiler */
//#ifndef __cplusplus
#include "mqtt.h"

extern const void *Blastpit;

typedef struct Blastpit {
	const void *cls; /* Must be first */
	t_Mqtt *net;     /* TODO: Change to network class */
} t_Blastpit;

typedef struct bp_message {
	int length;
	char *data;
} t_bp_message;

#ifdef __cplusplus
extern "C" {
#endif

/* Methods */
bool bp_isConnected(t_Blastpit *self);
int bp_connectToServer(t_Blastpit *, const char *server, const char *id,
		       int timeout_ms);
int bp_getMessageCount(t_Blastpit *self);
int bp_sendCommand(t_Blastpit *self, int id, const char *topic, int command);
int bp_sendMessage(t_Blastpit *self, const char *topic, const char *message);
int bp_sendMessageAndWait(t_Blastpit *self, int id, const char *topic,
			  const char *message, int timeout);
int bp_sendCommandAndWait(t_Blastpit *self, int id, const char *topic,
			  int command, int timeout);
char *bp_waitForString(t_Blastpit *self, int id, int timeout);
int bp_subscribe(t_Blastpit *self, const char *topic, int timeout_ms);
int bp_unsubscribe(t_Blastpit *self, const char *topic, int timeout_ms);
int clearQPSets(t_Blastpit *);
t_Blastpit *bp_new();
t_bp_message bp_getNewestMessage(t_Blastpit *self);
void bp_delete(t_Blastpit *);
void bp_disconnectFromServer(t_Blastpit *);
void registerCallback(t_Blastpit *, void (*)(void *));
char *bp_waitForString(t_Blastpit *self, int id, int timeout);

#ifdef __cplusplus
}
#endif

//#endif

#define CLSID_LMOS "{18213698-A9C9-11D1-A220-0060973058F6}"
#endif /* end of include guard: BLASTPIT_H */
