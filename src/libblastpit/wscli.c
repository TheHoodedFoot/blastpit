#include <getopt.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "blastpit.h"

enum timings
{
	SERVER_POLL_TIMEOUT_MS	  = 1000,
	SERVER_CONNECT_TIMEOUT_MS = 1000,
	HANDSHAKE_TIMEOUT_MS	  = 10000,
};

/* Flag set by ‘--verbose’. */
static int verbose_flag;	      // NOLINT
static int exit_client_loop = false;  // NOLINT

int callbackCount = 0;	// NOLINT

void
messageReceivedCallback( void* ev_data, void* object )
{
	(void)object;

	callbackCount++;
	// fprintf(stderr, "messageReceivedCallback was called %d times\n", callbackCount);

	if ( verbose_flag ) {
		WsMessage data = ConvertCallbackData( ev_data );
		// This string may not be null terminated, so we supply a max length.
		printf( "%.*s\n", data.size, data.data );
	}
}

////////////////////////////////////////////////////////////////////////
//                              Signals                               //
////////////////////////////////////////////////////////////////////////

void
HandleSIGINT( int dummy )
{  // Catch ctrl-c to end the client loop

	(void)dummy;

	// Reset the handler so any further signals are default
	(void)signal( SIGINT, SIG_DFL );

	exit_client_loop = true;
}

void
HandleSIGPIPE( int dummy )
{  // Catch broken pipe caused by LMOS crashing during upload

	(void)dummy;
}

void
server( const char* port )
{  // Start a server at localhost:8000

	t_Blastpit* simpleserver = blastpitNew();

	(void)fprintf( stderr, "wscli: Running listening server on port %s\n", port );

	// Set the callback
	registerCallback( simpleserver, &messageReceivedCallback );

	// Prevent broken pipes from crashing the server
	(void)signal( SIGPIPE, HandleSIGPIPE );

	serverCreate( simpleserver, port );

	while ( true ) {
		pollMessagesWithTimeout( simpleserver, SERVER_POLL_TIMEOUT_MS );
	}

	// Tidy up
	serverDestroy( simpleserver );
	blastpitDelete( simpleserver );
}

void
client( const char* server, const char* message )
{  // Connect to running server

	(void)message;

	// Create a client
	t_Blastpit* client = blastpitNew();

	printf( "wscli: Connecting to server at address %s\n", server );

	// Set the callback
	// registerCallback(client, &messageReceivedCallback);

	// Connect to the server
	int result = connectToServer( client, server, SERVER_CONNECT_TIMEOUT_MS );
	if ( result == kConnectionFailure ) {
		printf( "Failed to connect\n" );
		return;
	}

	// Wait for handshake
	printf( "%d\n", waitForConnection( client, HANDSHAKE_TIMEOUT_MS ) );

	// Send lines from STDIN as xml messages, ctrl-c to exit
	char*  line = NULL;
	size_t len  = 0;

	// signal(SIGINT, HandleSIGINT);

	while ( ( getline( &line, &len, stdin ) != -1 ) && !exit_client_loop ) {
		printf( "(wscli client) Sending message: %s\n", line );
		sendClientMessage( client, line );
		pollMessages( client );
	}

	// Wait for any messages
	// Tidy up
	disconnectFromServer( client );
	blastpitDelete( client );
}

int
main( int argc, char** argv )
{
	int count = 0;

	while ( 1 ) {
		static struct option long_options[] = { /* These options set a flag. */
							{ "verbose", no_argument, &verbose_flag, 1 },
							{ "brief", no_argument, &verbose_flag, 0 },
							/* These options don't set a flag.
							   We distinguish them by their indices. */
							{ "server", no_argument, 0, 's' },
							{ "client", required_argument, 0, 'c' },
							{ 0, 0, 0, 0 }
		};
		/* getopt_long stores the option index here. */
		int option_index = 0;

		count = getopt_long( argc, argv, "s:c:", long_options, &option_index );	 // NOLINT

		/* Detect the end of the options. */
		if ( count == -1 ) {
			break;
		}

		switch ( count ) {
			case 0:
				/* If this option set a flag, do nothing else now. */
				if ( long_options[option_index].flag != 0 ) {
					break;
				}
				printf( "option %s", long_options[option_index].name );
				if ( optarg ) {
					printf( " with arg %s", optarg );
				}
				printf( "\n" );
				break;

			case 's':
				server( optarg );
				break;

			case 'c':
				client( optarg, argv[optind] );
				break;

			case '?':
			default:
				printf( "wscli usage:\n-s <port> Run listening server on localhost\n-c <address> "
					"Connect to server at address, e.g. ws://127.0.0.1:8000\n" );
				/* getopt_long already printed an error message. */
				break;
		}
	}

	/* Instead of reporting ‘--verbose’
	   and ‘--brief’ as they are encountered,
	   we report the final status resulting from them. */
	if ( verbose_flag ) {
		puts( "verbose flag is set" );
	}

	/* Print any remaining command line arguments (not options). */
	if ( optind < argc ) {
		printf( "non-option ARGV-elements: " );
		while ( optind < argc ) {
			printf( "%s ", argv[optind++] );
		}
		putchar( '\n' );
	}
}
