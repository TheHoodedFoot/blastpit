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
#include <time.h>

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
	COMMAND(kResetRetvalDb)      \
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

#define FOREACH_RETVAL(RETVAL)         \
	RETVAL(kFailure)               \
	RETVAL(kSuccess)               \
	RETVAL(kAlarm)                 \
	RETVAL(kAllocationFailure)     \
	RETVAL(kAlreadyInUse)          \
	RETVAL(kBadCommand)            \
	RETVAL(kBadLogic)              \
	RETVAL(kBadVariadicParam)      \
	RETVAL(kBadXml)                \
	RETVAL(kCommandFailed)         \
	RETVAL(kConnectionFailure)     \
	RETVAL(kCurrentChanged)        \
	RETVAL(kException)             \
	RETVAL(kFreqChanged)           \
	RETVAL(kImageBegin)            \
	RETVAL(kImageEnd)              \
	RETVAL(kImageEnd2)             \
	RETVAL(kInfo)                  \
	RETVAL(kJobBegin)              \
	RETVAL(kJobEnd)                \
	RETVAL(kMessageMap)            \
	RETVAL(kMessageQueued)         \
	RETVAL(kMoBegin)               \
	RETVAL(kMoEnd)                 \
	RETVAL(kNullResource)          \
	RETVAL(kPlcEvent)              \
	RETVAL(kQueued)                \
	RETVAL(kReplyTimeout)          \
	RETVAL(kSetterFailure)         \
	RETVAL(kInvalid = -1)          \
	RETVAL(kMultipleCommands = -2) \
	RETVAL(kNotFound = -3)

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

typedef struct t_RetvalDb {
	int id;
	int retval;
	struct t_RetvalDb *next;
} RetvalDb;

typedef struct Blastpit {
	void *ws;	      // Our websocket (void* to avoid depending on websocket.h)
	int highest_id;	      // Highest id used (for auto generation)
	char *message_queue;  // Pointer to sds string holding queued messages
	RetvalDb *retval_db;  // List of all completed jobs
} t_Blastpit;

typedef struct {  // Acknowledgement of send plus generated id
	int id;
	int retval;
	char *string;
} IdAck;

typedef struct {  // Decoded websocket_message
	int size;
	unsigned char *data;
} WsMessage;

// Methods

// IdAck sendCommand(t_Blastpit *self, int command);
// char *BpGetMessageString(const char* message);
// int bp_waitForSignal(t_Blastpit *self, int signal, int timeout); /* Waits for an Lmos signal */
// void debugInfo(t_Blastpit *bp);
int AutoGenerateId(t_Blastpit *self);
int BpGetMessageCount(const char *xml);
int connectToServer(t_Blastpit *, const char *server, int timeout_ms);
int getMessageCount(t_Blastpit *self);
int serverCreate(t_Blastpit *self, const char *address);
int waitForConnection(t_Blastpit *self, int timeout);
void blastpitDelete(t_Blastpit *);
bool bp_isConnected(t_Blastpit *self);
void clearQPSets(t_Blastpit *);
void disconnectFromServer(t_Blastpit *);
void LayerSetHeight(t_Blastpit *self, const char *layer, int height);
void LayerSetLaserable(t_Blastpit *self, const char *layer, bool laserable);
void pollMessages(t_Blastpit *self);
void registerCallback(t_Blastpit *, void (*)(void *, void *));
void registerObject(t_Blastpit *, void *);
void sendClientMessage(t_Blastpit *self, const char *message);
void sendServerMessage(t_Blastpit *self, const char *message);
void serverDestroy(t_Blastpit *self);
void startLMOS(t_Blastpit *self);
void stopLMOS(t_Blastpit *self);
// IdAck bp_sendCommandAndWait(t_Blastpit *self, int command, int timeout);
IdAck bp_sendMessage(t_Blastpit *self, const char *message);
IdAck bp_sendMessageAndWait(t_Blastpit *self, const char *message, int timeout);
char *BpGetChildNodeAsString(const char *message, const char *child_name);
char *BpGetMessageAttribute(const char *message, const char *attribute);
char *BpGetMessageByIndex(const char *xml, int index);
char *BpSdsToString(char *string);
IdAck BpWaitForReplyOrTimeout(t_Blastpit *self, int id, int timeout);
const char *bpCommandName(int command);
const char *bpRetvalName(int retval);
char *popMessage(t_Blastpit *self);
char *popMessageAt(t_Blastpit *self, int index);
IdAck QueueAckRetval(t_Blastpit *self, int id, int retval);
IdAck QueueReplyPayload(t_Blastpit *self, int id, const char *payload);
char *readMessageAt(t_Blastpit *self, int index);
// IdAck SendCommand(t_Blastpit *self, int command);
// IdAck SendMessageBp(t_Blastpit *self, ...);
WsMessage ConvertCallbackData(void *ev_data);
t_Blastpit *blastpitNew();
int BpAddRetvalToDb(t_Blastpit *self, IdAck record);
// IdAck SendAckRetval(t_Blastpit *self, int id, int retval);

IdAck BpQueueQpSet(t_Blastpit *self, char *name, int current, int speed, int frequency);
// Appends a command to an xml string for bulk upload
IdAck BpQueueCommand(t_Blastpit *self, int command);
// Appends a multi-attribute message to an xml string
IdAck BpQueueMessage(t_Blastpit *self, ...);
// Uploads a message to the server without touching it
IdAck BpUploadQueuedMessages(t_Blastpit *self);
IdAck BpQueueCommandArgs(t_Blastpit *self, int command, const char *attr1, const char *val1, const char *attr2,
			 const char *val2, const char *attr3, const char *val3, const char *attr4, const char *val4,
			 const char *payload);
int BpQueryRetvalDb(t_Blastpit *self, int id);

char *SdsEmpty();
char *SdsFromLong(long number);
void SdsFree(char *string);
// void BpQueueDrawing(t_Blastpit *self, char *drawing);
void BpPrintQueue(t_Blastpit *self);
void BpFreeRetvalDb(t_Blastpit *self);

#define CLSID_LMOS "{18213698-A9C9-11D1-A220-0060973058F6}"

#ifdef __cplusplus
}
#endif

#endif /* end of include guard: BLASTPIT_H */
