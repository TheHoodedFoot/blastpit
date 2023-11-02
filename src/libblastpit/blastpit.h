#ifndef BLASTPIT_H
#define BLASTPIT_H

#ifdef __cplusplus
extern "C"
{
#endif

#ifdef _MSC_VER
#include "stdint.h"
#else
#include <stdint.h>
#endif

#include <stdbool.h>
#include <time.h>

// Constants fixed by Rofin hardware
#define LMOS_CURRENT_MIN   0
#define LMOS_CURRENT_MAX   100
#define LMOS_SPEED_MIN	   1
#define LMOS_SPEED_MAX	   1000
#define LMOS_FREQUENCY_MIN 20000
#define LMOS_FREQUENCY_MAX 60000

#define BP_ISLMOSUP_TIMEOUT 1200
#define BP_SHORT_TIMEOUT    1000

#define BP_TEMP_VLMFILE "outfile.VLM"

#define BP_EMPTY_STRING "<empty string>"

	enum BpCommand
	{
		kInvalidCommand,
		kAddQpSet,
		kCancelJob,
		kClearLayout,
		kClearLog,
		kClearQpSets,
		kConnectSignals,
		kCreateLMOS,
		kDestroyLMOS,
		kDisconnectSignals,
		kEmergencyStop,
		kExit,
		kForceRedraw,
		kGetGeoList,
		kGetPng,
		kGetVersion,
		kGetW,
		kHideLMOS,
		kImportXML,
		kInitMachine,
		kIsLmosRunning,
		kLayerSetExportable,
		kLayerSetHeight,
		kLayerSetLaserable,
		kLayerSetVisible,
		kLoadJob,
		kLoadVLM,
		kMOSetLaserable,
		kMOUnsetLaserable,
		kMoveW,
		kMoveZ,
		kPatchFlexibleShadows,
		kPhoto,
		kReadByte,
		kReadIOBit,
		kReference,
		kReply,
		kResetRetvalDb,
		kSaveQpSets,
		kSaveVLM,
		kSelfTest,
		kSetDimension,
		kSetMOLayer,
		kSetPosValues,
		kSetQualityParam,
		kShowLMOS,
		kShowMarkingArea,
		kSignal,
		kStartMarking,
		kStartPosHelp,
		kStatus,
		kStopMarking,
		kStopPosHelp,
		kSuppressRedraw,
		kTermMachine,
		kWriteByte,
		kWriteIoBit,
		kZoomWindow,
	};
	enum bpRetval
	{
		kFailure,
		kSuccess,
		kAlarm,
		kAllocationFailure,
		kAlreadyInUse,
		kBadCommand,
		kBadLogic,
		kBadParam,
		kBadXml,
		kCommandFailed,
		kConnectionFailure,
		kCurrentChanged,
		kException,
		kFreqChanged,
		kImageBegin,
		kImageEnd,
		kImageEnd2,
		kInfo,
		kJobBegin,
		kJobEnd,
		kMessageMap,
		kMessageQueued,
		kMoBegin,
		kMoEnd,
		kNullResource,
		kPlcEvent,
		kQueued,
		kReplyTimeout,
		kSetterFailure,
		kInvalid	  = -1,
		kMultipleCommands = -2,
		kNotFound	  = -3,
	};
	enum BpDebugLevel
	{
		kLvlOff,
		kLvlCritical,
		kLvlError,
		kLvlWarn,
		kLvlInfo,
		kLvlDebug,
		kLvlEverything,
	};

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-variable"

	static const char* bpCommandString[] = {
		"kInvalidCommand",
		"kAddQpSet",
		"kCancelJob",
		"kClearLayout",
		"kClearLog",
		"kClearQpSets",
		"kConnectSignals",
		"kCreateLMOS",
		"kDestroyLMOS",
		"kDisconnectSignals",
		"kEmergencyStop",
		"kExit",
		"kForceRedraw",
		"kGetGeoList",
		"kGetPng",
		"kGetVersion",
		"kGetW",
		"kHideLMOS",
		"kImportXML",
		"kInitMachine",
		"kIsLmosRunning",
		"kLayerSetExportable",
		"kLayerSetHeight",
		"kLayerSetLaserable",
		"kLayerSetVisible",
		"kLoadJob",
		"kLoadVLM",
		"kMOSetLaserable",
		"kMOUnsetLaserable",
		"kMoveW",
		"kMoveZ",
		"kPatchFlexibleShadows",
		"kPhoto",
		"kReadByte",
		"kReadIOBit",
		"kReference",
		"kReply",
		"kResetRetvalDb",
		"kSaveQpSets",
		"kSaveVLM",
		"kSelfTest",
		"kSetDimension",
		"kSetMOLayer",
		"kSetPosValues",
		"kSetQualityParam",
		"kShowLMOS",
		"kShowMarkingArea",
		"kSignal",
		"kStartMarking",
		"kStartPosHelp",
		"kStatus",
		"kStopMarking",
		"kStopPosHelp",
		"kSuppressRedraw",
		"kTermMachine",
		"kWriteByte",
		"kWriteIoBit",
		"kZoomWindow",
	};
	static const char* bpRetvalString[] = {
		"kFailure",
		"kSuccess",
		"kAlarm",
		"kAllocationFailure",
		"kAlreadyInUse",
		"kBadCommand",
		"kBadLogic",
		"kBadParam",
		"kBadXml",
		"kCommandFailed",
		"kConnectionFailure",
		"kCurrentChanged",
		"kException",
		"kFreqChanged",
		"kImageBegin",
		"kImageEnd",
		"kImageEnd2",
		"kInfo",
		"kJobBegin",
		"kJobEnd",
		"kMessageMap",
		"kMessageQueued",
		"kMoBegin",
		"kMoEnd",
		"kNullResource",
		"kPlcEvent",
		"kQueued",
		"kReplyTimeout",
		"kSetterFailure",
		"kInvalid = -1",
		"kMultipleCommands = -2",
		"kNotFound = -3",
	};
	static const char* bpDebugLevelString[] = {
		"kLvlOff", "kLvlCritical", "kLvlError", "kLvlWarn", "kLvlInfo", "kLvlDebug", "kLvlEverything",
	};

#pragma GCC diagnostic pop

#define BPCOMMAND_MAX  (int)( sizeof( bpCommandString ) / sizeof( bpCommandString[0] ) )
#define RETVAL_MAX     (int)( sizeof( bpRetvalString ) / sizeof( bpRetvalString[0] ) )
#define DEBUGLEVEL_MAX (int)( sizeof( BpDebugLevelString ) / sizeof( BpDebugLevelString[0] ) )

#ifdef DEBUG_LEVEL
#ifdef __linux__
#define BPLOG( level, fmt, ... )                                                                                       \
	do                                                                                                             \
		if ( DEBUG_LEVEL >= level ) {                                                                          \
			{                                                                                              \
				struct timespec now;                                                                   \
				clock_gettime( CLOCK_REALTIME, &now );                                                 \
				(void)fprintf( stderr, "%ld.%04ld: ", now.tv_sec, now.tv_nsec % 10000 );               \
				(void)fprintf( stderr, "\033[3%dm", level + 1 );                                       \
				(void)fprintf( stderr, fmt, __VA_ARGS__ );                                             \
				(void)fprintf( stderr, "\033[0m" );                                                    \
			}                                                                                              \
		}                                                                                                      \
	while ( 0 )
#else
#define BPLOG( level, fmt, ... )                                                                                       \
	do                                                                                                             \
		if ( DEBUG_LEVEL >= level ) {                                                                          \
			{                                                                                              \
				(void)fprintf( stderr, "\033[3%dm", level + 1 );                                       \
				(void)fprintf( stderr, fmt, __VA_ARGS__ );                                             \
				(void)fprintf( stderr, "\033[0m" );                                                    \
			}                                                                                              \
		}                                                                                                      \
	while ( 0 )
#endif
#else
#define BPLOG( ignore, these, arguments ) ( (void)0 )
#endif

	enum BpPolyPoint
	{
		kVertex = 0,
		kArc,
		kBezier,
	};

	enum InPortConstants
	{
		kPort1B = 0,
		kPort1C_In,
		kPort1D,
		kPort1E,
	};

	enum BpHIDEvent
	{
		HID_AXIS_X,
		HID_AXIS_Y,
		HID_AXIS_ROT,
		HID_AXIS_JOG,
		HID_BUTTON,
	};

	enum BpLinetype
	{
		LineCONT    = 0,
		LineDASH    = 1,
		LineDOT	    = 2,
		LineDASHDOT = 3,
		LineCENTER  = 4,
		LinePHANTOM = 5,
	};

	enum BpJustificationType
	{
		justifyLeft,
		justifyRight,
		justifyCentre,
	};

	struct BpLogEntry
	{
		uint32_t id;
		uint32_t command;
		uint32_t result;
	};

	union BpHIDVal
	{
		int   iVal;
		float fVal;
	};

	struct BpHID
	{
		uint32_t       event;
		union BpHIDVal value;
	};

	typedef struct t_RetvalDb
	{
		int		   id;
		int		   retval;
		struct t_RetvalDb* next;
	} RetvalDb;

	typedef struct Blastpit
	{
		void*	  ws;		   // Our websocket (void* to avoid depending on websocket.h)
		int	  highest_id;	   // Highest id used (for auto generation)
		char*	  message_queue;   // Pointer to sds string holding queued messages
		RetvalDb* retval_db;	   // List of all completed jobs
		bool	  light_is_on;	   // Last known light state
		int	  door_is_closed;  // Last known door state
	} t_Blastpit;

	typedef struct
	{  // Acknowledgement of send plus generated id
		int   id;
		int   retval;
		char* string;
	} IdAck;

	typedef struct
	{  // Decoded websocket_message
		int	    size;
		const char* data;
	} WsMessage;

	// Methods

	// IdAck sendCommand(t_Blastpit *self, int command);
	// char *BpGetMessageString(const char* message);
	// int bp_waitForSignal(t_Blastpit *self, int signal, int timeout); /* Waits for an Lmos signal */
	// void debugInfo(t_Blastpit *bp);
	int  AutoGenerateId( t_Blastpit* self );
	void BpFreePayload( IdAck ack );
	int  BpGetMessageCount( const char* xml );
	int  connectToServer( t_Blastpit*, const char* server, int timeout_ms );
	int  getMessageCount( t_Blastpit* self );
	int  serverCreate( t_Blastpit* self, const char* address );
	int  waitForConnection( t_Blastpit* self, int timeout );
	void blastpitDelete( t_Blastpit* );
	bool bp_isConnected( t_Blastpit* self );
	void clearQPSets( t_Blastpit* );
	void disconnectFromServer( t_Blastpit* );
	void LayerSetHeight( t_Blastpit* self, const char* layer, int height );
	void LayerSetLaserable( t_Blastpit* self, const char* layer, bool laserable );
	void pollMessages( t_Blastpit* self );
	void pollMessagesWithTimeout( t_Blastpit* self, int timeout );
	void registerCallback( t_Blastpit*, void ( * )( void*, void* ) );
	void registerObject( t_Blastpit*, void* );
	void sendClientMessage( t_Blastpit* self, const char* message );
	void sendServerMessage( t_Blastpit* self, const char* message );
	void serverDestroy( t_Blastpit* self );
	void startLMOS( t_Blastpit* self );
	void stopLMOS( t_Blastpit* self );
	// IdAck bp_sendCommandAndWait(t_Blastpit *self, int command, int timeout);
	IdAck	    bp_sendMessage( t_Blastpit* self, const char* message );
	IdAck	    bp_sendMessageAndWait( t_Blastpit* self, const char* message, int timeout );
	char*	    BpGetChildNodeAsString( const char* message, const char* child_name );
	char*	    BpGetMessageAttribute( const char* message, const char* attribute );
	char*	    BpGetMessageByIndex( const char* xml, int index );
	char*	    BpSdsToString( char* string );
	IdAck	    BpWaitForReplyOrTimeout( t_Blastpit* self, int id, int timeout );
	IdAck	    BpWaitForSignalOrTimeout( t_Blastpit* self, int sigtype, int timeout );
	const char* bpCommandName( int command );
	const char* bpRetvalName( int retval );
	char*	    popMessage( t_Blastpit* self );
	char*	    popMessageAt( t_Blastpit* self, int index );
	IdAck	    QueueAckRetval( t_Blastpit* self, int id, int retval );
	IdAck	    QueueReplyPayload( t_Blastpit* self, int id, int retval, const char* payload );
	IdAck	    BpQueueSignal( t_Blastpit* self, int signal, const char* payload );
	char*	    readMessageAt( t_Blastpit* self, int index );
	// IdAck SendCommand(t_Blastpit *self, int command);
	// IdAck SendMessageBp(t_Blastpit *self, ...);
	WsMessage   ConvertCallbackData( void* ev_data );
	t_Blastpit* blastpitNew( void );
	int	    BpAddRetvalToDb( t_Blastpit* self, IdAck record );
	// IdAck SendAckRetval(t_Blastpit *self, int id, int retval);

	IdAck BpQueueQpSet( t_Blastpit* self, char* name, int current, int speed, int frequency );
	// Appends a command to an xml string for bulk upload
	IdAck BpQueueCommand( t_Blastpit* self, int command );
	// Appends a multi-attribute message to an xml string
	IdAck BpQueueMessage( t_Blastpit* self, ... );
	// Uploads a message to the server without touching it
	IdAck BpUploadQueuedMessages( t_Blastpit* self );
	IdAck BpQueueCommandArgs( t_Blastpit* self,
				  int	      command,
				  const char* attr1,
				  const char* val1,
				  const char* attr2,
				  const char* val2,
				  const char* attr3,
				  const char* val3,
				  const char* attr4,
				  const char* val4,
				  const char* payload );
	int   BpQueryRetvalDb( t_Blastpit* self, int id );

	char* SdsEmpty( void );
	char* SdsNew( const char* string );
	char* SdsFromLong( long number );
	void  SdsFree( char* string );
	// void BpQueueDrawing(t_Blastpit *self, char *drawing);
	void BpPrintQueue( t_Blastpit* self );
	void BpFreeRetvalDb( t_Blastpit* self );
	int  BpIsLmosUp( t_Blastpit* self );
	void BpSetLightState( t_Blastpit* self, bool state );
	void BpSetDoorState( t_Blastpit* self, bool state );
	void BpMoveW( t_Blastpit* self, int angle );
	void BpInitMachine( t_Blastpit* self );
	void BpTermMachine( t_Blastpit* self );
	void BpDisplayLmosWindow( t_Blastpit* self, int visibility );
	void BpSaveQpSets( t_Blastpit* self );

#define CLSID_LMOS "{18213698-A9C9-11D1-A220-0060973058F6}"

#ifdef __cplusplus
}
#endif

#endif /* end of include guard: BLASTPIT_H */
