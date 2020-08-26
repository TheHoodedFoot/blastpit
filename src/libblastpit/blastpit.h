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

#define BP_EMPTY_STRING "<empty string>"

enum BpReply {
	kNack,
	kAck,
};

/* The following preprocessor macros are used to generate
 * both the enums and the enum name strings. The #pragmas are used
 * to squash the unused variable compiler error. */

#define GENERATE_ENUM(ENUM) ENUM,
#define GENERATE_STRING(STRING) #STRING,

#define FOREACH_BPCOMMAND(COMMAND)   \
	COMMAND(kAddQpSet)           \
	COMMAND(kCancelJob)          \
	COMMAND(kClearLayout)        \
	COMMAND(kClearQpSets)        \
	COMMAND(kConnectSignals)     \
	COMMAND(kCreateLMOS)         \
	COMMAND(kDestroyLMOS)        \
	COMMAND(kDisconnectSignals)  \
	COMMAND(kEmergencyStop)      \
	COMMAND(kExit)               \
	COMMAND(kForceRedraw)        \
	COMMAND(kGetPng)             \
	COMMAND(kGetVersion)         \
	COMMAND(kHideLMOS)           \
	COMMAND(kImportXML)          \
	COMMAND(kInitMachine)        \
	COMMAND(kLayerSetExportable) \
	COMMAND(kLayerSetHeight)     \
	COMMAND(kLayerSetLaserable)  \
	COMMAND(kLayerSetVisible)    \
	COMMAND(kLoadJob)            \
	COMMAND(kLoadVLM)            \
	COMMAND(kMOSetLaserable)     \
	COMMAND(kMOUnsetLaserable)   \
	COMMAND(kMoveW)              \
	COMMAND(kMoveZ)              \
	COMMAND(kReadByte)           \
	COMMAND(kReadIOBit)          \
	COMMAND(kReference)          \
	COMMAND(kReply)              \
	COMMAND(kSaveVLM)            \
	COMMAND(kSelfTest)           \
	COMMAND(kSetDimension)       \
	COMMAND(kSetMOLayer)         \
	COMMAND(kSetPosValues)       \
	COMMAND(kSetQualityParam)    \
	COMMAND(kShowLMOS)           \
	COMMAND(kShowMarkingArea)    \
	COMMAND(kSignal)             \
	COMMAND(kStartMarking)       \
	COMMAND(kStartPosHelp)       \
	COMMAND(kStatus)             \
	COMMAND(kStopMarking)        \
	COMMAND(kStopPosHelp)        \
	COMMAND(kSuppressRedraw)     \
	COMMAND(kTermMachine)        \
	COMMAND(kWriteByte)          \
	COMMAND(kWriteIOBit)         \
	COMMAND(kZoomWindow)

#define FOREACH_RETVAL(RETVAL)     \
	RETVAL(kFailure)           \
	RETVAL(kSuccess)           \
	RETVAL(kAlarm)             \
	RETVAL(kAllocationFailure) \
	RETVAL(kAlreadyInUse)      \
	RETVAL(kBadCommand)        \
	RETVAL(kBadLogic)          \
	RETVAL(kCommandFailed)     \
	RETVAL(kConnectionFailure) \
	RETVAL(kCurrentChanged)    \
	RETVAL(kException)         \
	RETVAL(kFreqChanged)       \
	RETVAL(kImageBegin)        \
	RETVAL(kImageEnd)          \
	RETVAL(kImageEnd2)         \
	RETVAL(kInfo)              \
	RETVAL(kJobBegin)          \
	RETVAL(kJobEnd)            \
	RETVAL(kMessageMap)        \
	RETVAL(kMoBegin)           \
	RETVAL(kMoEnd)             \
	RETVAL(kNullResource)      \
	RETVAL(kPlcEvent)          \
	RETVAL(kReplyTimeout)      \
	RETVAL(kSetterFailure)     \
	RETVAL(kInvalid = -1)

#define FOREACH_DEBUGLEVEL(LEVEL) \
	LEVEL(kLvlOff)            \
	LEVEL(kLvlCritical)       \
	LEVEL(kLvlError)          \
	LEVEL(kLvlWarn)           \
	LEVEL(kLvlInfo)           \
	LEVEL(kLvlDebug)          \
	LEVEL(kLvlEverything)

// We cannot use negative numbers here, because it would generate
// negative array offsets for the string arrays.
enum BpCommand { FOREACH_BPCOMMAND(GENERATE_ENUM) };
enum bpRetval { FOREACH_RETVAL(GENERATE_ENUM) };
enum BpDebugLevel { FOREACH_DEBUGLEVEL(GENERATE_ENUM) };

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-variable"
static const char *bpCommandString[] = {FOREACH_BPCOMMAND(GENERATE_STRING)};
static const char *bpRetvalString[] = {FOREACH_RETVAL(GENERATE_STRING)};
static const char *bpDebugLevelString[] = {FOREACH_DEBUGLEVEL(GENERATE_STRING)};
#pragma GCC diagnostic pop
#define BPCOMMAND_MAX (int)(sizeof(bpCommandString) / sizeof(bpCommandString[0]))
#define RETVAL_MAX (int)(sizeof(bpRetvalString) / sizeof(bpRetvalString[0]))
#define DEBUGLEVEL_MAX (int)(sizeof(BpDebugLevelString) / sizeof(BpDebugLevelString[0]))

#ifdef DEBUG_LEVEL
#ifdef __linux__
#define LOG(level, fmt, ...)                                                                     \
	do                                                                                       \
		if (DEBUG_LEVEL >= level) {                                                      \
			{                                                                        \
				struct timespec now;                                             \
				clock_gettime(CLOCK_MONOTONIC_RAW, &now);                        \
				fprintf(stderr, "%ld.%04ld: ", now.tv_sec, now.tv_nsec % 10000); \
				fprintf(stderr, "\033[3%dm", level + 1);                         \
				fprintf(stderr, fmt, __VA_ARGS__);                               \
				fprintf(stderr, "\033[0m");                                      \
			}                                                                        \
		}                                                                                \
	while (0)
#else
#define LOG(level, fmt, ...)                                             \
	do                                                               \
		if (DEBUG_LEVEL >= level) {                              \
			{                                                \
				fprintf(stderr, "\033[3%dm", level + 1); \
				fprintf(stderr, fmt, __VA_ARGS__);       \
				fprintf(stderr, "\033[0m");              \
			}                                                \
		}                                                        \
	while (0)
#endif
#else
#define LOG(ignore, these, arguments) ((void)0)
#endif

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
#include "websocket.h"
// #include "xml.hpp"

// extern const void *Blastpit;

// TODO: Convert this to a void pointer so we don't need to include
// websocket.h in every file that includes blastpit.h
typedef struct Blastpit {
	t_Websocket *ws;
} t_Blastpit;

typedef struct bp_message {
	int length;
	char *data;
} t_bp_message;

#ifdef __cplusplus
extern "C" {
#endif

/* Methods */
char *bp_sendMessageAndWaitForString(t_Blastpit *self, int id, const char *message, int timeout);
char *bp_waitForString(t_Blastpit *self, int id, int timeout);
char *bp_waitForXml(t_Blastpit *self, int id, int timeout, int del);
t_Blastpit *blastpitNew();
int bp_getNextFreeID(t_Blastpit *self);
bool bp_isConnected(t_Blastpit *self);
int bp_sendCommandAndWait(t_Blastpit *self, int id, int command, int timeout);
int bp_sendMessage(t_Blastpit *self, int id, const char *message);
int bp_sendMessageAndWait(t_Blastpit *self, int id, const char *message, int timeout);
int bp_waitForSignal(t_Blastpit *self, int signal, int timeout); /* Waits for an Lmos signal */
int clearQPSets(t_Blastpit *);
int connectToServer(t_Blastpit *, const char *server, int timeout_ms);
int serverCreate(t_Blastpit *self, const char *address);
void serverDestroy(t_Blastpit *self);
void blastpitDelete(t_Blastpit *);
void disconnectFromServer(t_Blastpit *);
void registerCallback(t_Blastpit *, void (*)(void *));
void registerCallbackCpp(t_Blastpit *, void (*)(void *, void *));
void registerObject(t_Blastpit *, void *);
int sendCommand(t_Blastpit *self, int id, int command);
void pollMessages(t_Blastpit *self);
void sendClientMessage(t_Blastpit *self, const char *message);
void sendServerMessage(t_Blastpit *self, const char *message);
int waitForConnection(t_Blastpit *self, int timeout);
// void debugInfo(t_Blastpit *bp);
char *popMessage(t_Blastpit *self);
char *popMessageAt(t_Blastpit *self, int index);
int getMessageCount(t_Blastpit *self);
char *popMessageAt(t_Blastpit *self, int index);
char *readMessageAt(t_Blastpit *self, int index);
const char *bpCommandName(int command);
const char *bpRetvalName(int retval);
void stopLMOS(t_Blastpit *self);
void startLMOS(t_Blastpit *self);


#ifdef __cplusplus
}
#endif

//#endif

#define CLSID_LMOS "{18213698-A9C9-11D1-A220-0060973058F6}"
#endif /* end of include guard: BLASTPIT_H */
