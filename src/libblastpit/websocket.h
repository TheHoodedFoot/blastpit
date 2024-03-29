#ifndef WEBSOCKET_H
#define WEBSOCKET_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "blastpit.h"
#include "mongoose.h"
#include <signal.h>
#include <stdbool.h>

#define WS_POLL_TIME_MS 1  // ms to wait before mongoose returns

	enum
	{  // Return codes
		WS_PORT_UNDEFINED = -1
	};

	typedef struct node
	{
		void*	     data;
		struct node* next;
	} t_Node;

	typedef struct
	{  // WebSocket struct
		struct mg_mgr	      mongoose;
		struct mg_connection* connection;
		sig_atomic_t	      s_signal_received;
		bool		      evloopIsRunning;
		bool		      isServer;
		bool		      isConnected;
		void ( *messageReceived )( void*, void* );  // Callback
		void*	object;				    // Reference to the c++ Parser:: instance
		t_Node* messageStack;			    // Linked list of received messages
	} t_Websocket;

	// Constructor and destructor
	t_Websocket* websocketNew( void );
	void	     websocketDelete( t_Websocket* websock );

	// Common functions
	void  wsSetMessageReceivedCallback( t_Websocket* self, void ( *callback )( void*, void* ) );
	void  wsSetMessageReceivedObject( t_Websocket* self, void* object );
	void  wsPushMessage( t_Websocket* self, void* data );
	void* wsPopMessage( t_Websocket* self );
	int   wsGetMessageCount( t_Websocket* self );
	void* wsPopMessageAt( t_Websocket* self, int index );
	void* wsReadMessageAt( t_Websocket* self, int index );
	void  wsFlushMessages( t_Websocket* websock );

	// Server functions
	int  wsServerCreate( t_Websocket* self, const char* listen_address );
	int  wsServerDestroy( t_Websocket* );
	void wsPoll( t_Websocket* self );
	void wsPollWithTimeout( t_Websocket* self, int timeout );
	int  wsClientSendMessage( t_Websocket* self, char* data );
	int  wsServerSendMessage( t_Websocket* self, char* data );
	int  Ignore( t_Websocket* );

	// int ws_server_main(void);

	// Client functions
	int  wsClientCreate( t_Websocket*, const char* address );
	int  wsClientDestroy( t_Websocket* );
	void broadcastClient( struct mg_connection* nc, const struct mg_str msg );
	void broadcastServer( struct mg_connection* nc, const struct mg_str msg );

	// Public functions
	WsMessage ExtractWsMessageData( void* ev_data );

	// int ws_client_main(int argc, char **argv);

	// bool netIsConnected(t_WebsocketClient *);
	// int netConnectToServer(t_WebsocketClient *);
	// int netDisconnectFromServer(t_WebsocketClient *);
	// int netMessageToServer(t_WebsocketClient *, const int, const char *);
	// void netSetOnReceiveCallback(t_WebsocketClient *, void (*)(void *));

#ifdef __cplusplus
}
#endif

#endif
