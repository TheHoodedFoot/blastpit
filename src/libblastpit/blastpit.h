#ifndef BLASTPIT_H
#define BLASTPIT_H

#ifdef __cplusplus
extern "C" {
#endif

#ifdef _MSC_VER
#include "stdint.h"
#else
#include <stdint.h>
#endif

#include <stdbool.h>

// Constants fixed by Rofin hardware
#define LMOS_CURRENT_MIN 0
#define LMOS_CURRENT_MAX 100
#define LMOS_SPEED_MIN 1
#define LMOS_SPEED_MAX 1000
#define LMOS_FREQUENCY_MIN 20000
#define LMOS_FREQUENCY_MAX 60000

// #define BP_PACKET_CHECKSUM_SIZE 5 * sizeof(uint32_t)
// #define BP_DATA_CHECKSUM_SIZE sizeof(struct BpPacket)

#define BP_EMPTY_STRING "<empty string>"

// enum BpReply {
// 	kNack,
// 	kAck,
// };

/* The following preprocessor macros are used to generate
 * both the enums and the enum name strings. The #pragmas are used
 * to squash the unused variable compiler error. */

#define GENERATE_ENUM(ENUM) ENUM,
#define GENERATE_STRING(STRING) #STRING,

// kInvalidCommand must be kept as the first item (index=0),
// so that bad strings, which evaluate to 0, will be discovered
#define FOREACH_BPCOMMAND(COMMAND)   \
	COMMAND(kInvalidCommand)     \
	COMMAND(kAddQpSet)           \
	COMMAND(kCancelJob)          \
	COMMAND(kClearLayout)        \
	COMMAND(kClearLog)           \
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
	COMMAND(kPhoto)              \
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
	COMMAND(kWriteIoBit)         \
	COMMAND(kZoomWindow)

#define FOREACH_RETVAL(RETVAL)     \
	RETVAL(kFailure)           \
	RETVAL(kSuccess)           \
	RETVAL(kAlarm)             \
	RETVAL(kAllocationFailure) \
	RETVAL(kAlreadyInUse)      \
	RETVAL(kBadCommand)        \
	RETVAL(kBadLogic)          \
	RETVAL(kBadVariadicParam)  \
	RETVAL(kBadXml)            \
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
	RETVAL(kQueued)            \
	RETVAL(kReplyTimeout)      \
	RETVAL(kSetterFailure)     \
	RETVAL(kInvalid = -1)      \
	RETVAL(kMultipleCommands = -2)

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

// struct BpPacket {
// 	uint32_t id;
// 	uint32_t size;
// 	uint32_t command;
// 	uint32_t parentid;
// 	uint32_t dataChecksum;
// 	uint32_t packetChecksum;
// };

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

// union psHeader {
// 	unsigned char bytes[sizeof(struct BpPacket)];
// 	struct BpPacket header;
// };

typedef struct Blastpit {
	// t_Websocket *ws;
	void *ws;
	int highest_id;	      // Highest id used (for auto generation)
	char *message_queue;  // Pointer to sds string holding queued messages
} t_Blastpit;

typedef struct bp_message {
	int length;
	char *data;
} t_bp_message;

typedef struct {  // Acknowledgement of send plus generated id
	int id;
	int retval;
	char *string;
} IdAck;

typedef struct {  // Decoded websocket_message
	int size;
	unsigned char *data;
} WsMessage;

/* Methods */
IdAck bp_sendMessageAndWaitForString(t_Blastpit *self, const char *message, int timeout);
IdAck bp_waitForString(t_Blastpit *self, int id, int timeout);
IdAck bp_waitForXml(t_Blastpit *self, int id, int timeout, int del);
t_Blastpit *blastpitNew();
bool bp_isConnected(t_Blastpit *self);
IdAck bp_sendCommandAndWait(t_Blastpit *self, int command, int timeout);
IdAck bp_sendMessage(t_Blastpit *self, const char *message);
IdAck bp_sendMessageAndWait(t_Blastpit *self, const char *message, int timeout);
int bp_waitForSignal(t_Blastpit *self, int signal, int timeout); /* Waits for an Lmos signal */
void clearQPSets(t_Blastpit *);
int connectToServer(t_Blastpit *, const char *server, int timeout_ms);
int serverCreate(t_Blastpit *self, const char *address);
void serverDestroy(t_Blastpit *self);
void blastpitDelete(t_Blastpit *);
void disconnectFromServer(t_Blastpit *);
void registerCallback(t_Blastpit *, void (*)(void *, void *));
void registerObject(t_Blastpit *, void *);
WsMessage ConvertCallbackData(void *ev_data);

// IdAck sendCommand(t_Blastpit *self, int command);

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
int AutoGenerateId(t_Blastpit *self);
void LayerSetLaserable(t_Blastpit *self, const char *layer, bool laserable);
void LayerSetHeight(t_Blastpit *self, const char *layer, int height);

IdAck SendMessageBp(t_Blastpit *self, ...);
IdAck SendCommand(t_Blastpit *self, int command);
IdAck SendAckRetval(t_Blastpit *self, int id, int retval);
int BpHasMultipleMessages(const char *xml);
char *BpGetMessageByIndex(const char *xml, int index);
char *BpGetMessageAttribute(const char *message, const char *attribute);

int BpQueueQpSet(t_Blastpit *self, char *name, int current, int speed, int frequency);
// Appends a command to an xml string for bulk upload
int BpQueueCommand(t_Blastpit *self, int command);
// Appends a multi-attribute message to an xml string
int BpQueueMessage(t_Blastpit *self, ...);
// Uploads a message to the server without touching it
IdAck BpUploadQueuedMessages(t_Blastpit *self);

char *SdsEmpty();
void SdsFree(char *string);

#define CLSID_LMOS "{18213698-A9C9-11D1-A220-0060973058F6}"

#ifdef __cplusplus
}
#endif

#endif /* end of include guard: BLASTPIT_H */
