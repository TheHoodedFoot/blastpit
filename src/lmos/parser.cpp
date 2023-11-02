#include "parser.hpp"
#include "blastpit.h"
#include <QDateTime>
#include <QSettings>
#include <QTime>
#include <QtCore>
#include <cstring>
#include <iostream>
#include <sstream>
#include <string>

#define MUTEX_COUNT 10	// How many cycles to allow mutex to stay locked

Parser::Parser( QObject* parent )
	: QObject( parent )
{
	QObject::connect( &lmos, SIGNAL( log( QString ) ), this, SLOT( log( QString ) ) );
	QObject::connect(
		&lmos, SIGNAL( log( int, const char*, QString ) ), this, SLOT( log( int, const char*, QString ) ) );
	QObject::connect( &lmos, SIGNAL( ack( QString ) ), this, SLOT( ack( QString ) ) );
	QObject::connect( &lmos, SIGNAL( sendEvent( int, QString ) ), this, SLOT( SendSignal( int, QString ) ) );

	blast = blastpitNew();
	assert( blast );

	// Set the object
	// void (*func)(void *
	registerCallback( blast, &messageReceivedCallback );
	registerObject( blast, (void*)this );
	mutex = 0;

	wsConnect();

	timer = new QTimer( this );
	connect( timer, SIGNAL( timeout() ), this, SLOT( update() ) );

	// TODO: The timer amount has a huge effect on the responsiveness
	// when running under Wine. Find out why.
	timer->start( 100 );
}

Parser::~Parser()
{
	timer->stop();
	disconnectFromServer( blast );
	blastpitDelete( blast );
}

void
Parser::wsConnect()
{  // (Re)try connection to server

	static QDateTime last_connection_attempt = QDateTime::currentDateTime().addSecs( -WS_RETRY_SECONDS );
	if ( QDateTime::currentDateTime() < last_connection_attempt.addSecs( WS_RETRY_SECONDS ) ) {
		return;
	}
	last_connection_attempt = QDateTime::currentDateTime();

	QSettings traySettings( "Rfbevanco", "lmos-tray" );
	QString	  wsserver = traySettings.value( "wsServer" ).toString();

	if ( wsserver.size() < MIN_SERVER_STRING_LEN ) {
		return;
	}

	QByteArray  ba	   = wsserver.toLocal8Bit();
	const char* c_str2 = ba.data();

	log( "Parser::wsConnect : Trying to connect to server " + traySettings.value( "wsServer" ).toString() );
	connectToServer( blast, c_str2, 1000 );
}

void
Parser::messageReceivedCallback( void* ev_data, void* object )
{
	Parser* psr = (Parser*)object;

	psr->mutex = MUTEX_COUNT;

	// This is not null terminated
	WsMessage msg_data = ConvertCallbackData( ev_data );

	char* msg_data_string = (char*)malloc( msg_data.size + 1 );
	strncpy( msg_data_string, (const char*)msg_data.data, msg_data.size );
	*( msg_data_string + msg_data.size ) = 0;

	psr->ProcessMessageBlock( msg_data_string );
	free( msg_data_string );

	psr->mutex = 0;
}

void
Parser::ProcessMessageBlock( const char* msg_data_string )
{  // Extract and process individual messages from a string

	char* message	= NULL;
	int   msg_count = BpGetMessageCount( (const char*)msg_data_string );

	log( "Processing " + QString::number( msg_count ) + " messages" );

	for ( int i = 0; i < msg_count; i++ ) {
		message = BpGetMessageByIndex( (const char*)msg_data_string, i );

		// TODO: Loop through all depends comma-separated dependencies
		char* depends = BpGetMessageAttribute( message, "depends" );
		if ( !depends || BpQueryRetvalDb( blast, atoi( depends ) ) == kSuccess ) {
			parseCommand( message );
		} else {
			char* id = BpGetMessageAttribute( message, "id" );
			log( "Message with id " );
			log( id );
			log( " has failed dependency " );
			log( depends );
			ackReturn( atoi( id ), kFailure );
			SdsFree( id );
		};
		SdsFree( depends );
		SdsFree( message );
	}
	log( "Finished Parsing Messages" );
}

void
Parser::update()
{  // Poll for messages

	if ( mutex ) {
		mutex--;
		return;
	}

	// laserStatus:
	// 0 = no network, 1 = network no lmos loaded, 2 = net and lmos
	static int laserStatus = -1;


	if ( !bp_isConnected( ( blast ) ) ) {
		if ( laserStatus != 0 ) {
			emit seticon( "noconnection" );
			laserStatus = 0;

			lmos.DisconnectSignals();
			lmos.HideWindow();
			lmos.DestroyControl();
		}

		// disconnectFromServer(blast);
		this->wsConnect();

		// This is a static member function of QThread
		// QThread::sleep(1);

	} else {
		if ( laserStatus < 1 ) {
			log( "Parser::update : Connected." );
			QSettings traySettings( "Rfbevanco", "lmos-tray" );
			QString	  wsserver = "Connected to server " + traySettings.value( "wsServer" ).toString();
			emit	  settray( wsserver );
			emit	  seticon( "nolaser" );
			laserStatus = 1;

			lmos.CreateControl();
			// lmos.ShowWindow();
			lmos.ConnectSignals();
		}
	}

	if ( blast->message_queue ) {
		// This will automatically poll messages as well
		BpUploadQueuedMessages( blast );
	} else {
		pollMessages( blast );
	}
}

void
Parser::log( QString string )
{
	emit sendlog( string );
}

void
Parser::log( int level, const char* function, QString entry )
{
	QString time	= QTime::currentTime().toString( "hh:mm:ss.zzz" );
	QString log	= QString::number( level );
	QString logFunc = QString( function );
	emit	sendlog( time + ": (" + log + ") [" + logFunc + "] " + entry );
}

void
Parser::ack( QString message )
{ /* Send a network acknowledgment */

	log( "[ack]" );
	bp_sendMessage( blast, message.toStdString().c_str() );
}

void
Parser::ackReturn( int id, int retval )
{  // Send a network acknowledgment

	QString message = QString::number( retval );
	log( "[ackReturn] (" + QString::number( id ) + ") " + QString( bpRetvalName( retval ) ) );
	BpAddRetvalToDb( this->blast, ( IdAck ){ id, retval, NULL } );
	QueueAckRetval( blast, id, retval );
	BpUploadQueuedMessages( blast );
}

void
Parser::ReplyWithPayload( int id, int retval, const char* payload )
{  // Reply to a commmand with a payload string

	log( "[ReplyWithPayload] #" + QString::number( id ) + " : " + QString( payload ) );
	QueueReplyPayload( blast, id, retval, payload );
	BpUploadQueuedMessages( blast );
}

void
Parser::SendSignal( int signal, QString message )
{  // Used by Lmos to send a signal
	// Assume that message is not null terminated

	char* signal_string = (char*)alloca( 4 );
	snprintf( signal_string, 4, "%d", signal );
	char* message_string = (char*)malloc( message.size() + 1 );
	strncpy( message_string, message.toStdString().c_str(), message.size() );
	*( message_string + message.size() ) = 0;

	log( "Parser::SendSignal" );
	log( signal_string );
	log( message_string );
	BpQueueSignal( blast, signal, message_string );
	// QueueSignal(blast, signal, "TEST SIGNAL");
	BpUploadQueuedMessages( blast );
	free( message_string );
}

void
Parser::parseCommand( const char* xml )
{
	char*  id_string      = BpGetMessageAttribute( xml, "id" );
	int    id	      = atoi( id_string );
	char*  command_string = BpGetMessageAttribute( xml, "command" );
	int    command	      = atoi( command_string );
	char*  message_string = NULL;
	char * attr1 = NULL, *attr2 = NULL, *attr3 = NULL, *attr4 = NULL;
	int    retval_num;
	char*  payload = NULL;
	double retval_double;

	log( "(" + QTime::currentTime().toString( "hh:mm:ss.zzz" ) + ") #" + QString::number( id ) + ": " +
	     QString( bpCommandName( command ) ) );

	std::stringstream out;
	QString		  time = QTime::currentTime().toString( "hh:mm:ss.zzz" );
	QPixmap		  pixmap;
	QByteArray	  bArray;
	QBuffer		  buffer( &bArray );
	std::string	  stdString;

	QString	   result_qs;
	QByteArray result_ba;

	switch ( command ) {
		case kIsLmosRunning:
			ackReturn( id, kSuccess );
			break;
		case kGetVersion:
			/* Create an XML with the git version inside */
			// free(versionString);
			break;
		case kInitMachine:
			lmos.InitMachine();
			ackReturn( id, kSuccess );
			break;
		case kTermMachine:
			lmos.TermMachine();
			ackReturn( id, kSuccess );
			break;
		case kClearQpSets:
			lmos.ClearQPSets();
			lmos.SaveQPSets();
			ackReturn( id, kSuccess );
			break;
		case kCancelJob:
			ackReturn( id, lmos.CancelJob() );
			break;
		case kImportXML:
			message_string = BpGetChildNodeAsString( xml, "DRAWING" );
			if ( message_string ) {
				ackReturn( id,
					   lmos.LoadXML( QString::fromStdString( message_string ) ) ? kSuccess
												    : kFailure );
			} else {
				ackReturn( id, kBadParam );
			}
			break;
		case kStopPosHelp:
			lmos.StopPosHelp();
			ackReturn( id, kSuccess );
			break;
		case kStartMarking:
			ackReturn( id, lmos.StartMarking() );
			break;
		case kLoadJob:
			ackReturn( id, lmos.LoadJob() );
			break;
		case kStopMarking:
			lmos.StopMarking();
			ackReturn( id, kSuccess );
			break;
		case kMOSetLaserable:
			attr1 = BpGetMessageAttribute( xml, "object" );
			if ( attr1 ) {
				lmos.SetLaserable( attr1, true );
				ackReturn( id, kSuccess );
			} else {
				ackReturn( id, kBadParam );
			}
			break;
		case kMOUnsetLaserable:
			lmos.UnsetLaserableAllObjects();
			ackReturn( id, kSuccess );
			break;
		case kSelfTest:
			// lmos.Test();
			SendSignal( 999, "Test Signal 999" );
			ackReturn( id, kSuccess );
			break;
		case kStartPosHelp:
			attr1 = BpGetMessageAttribute( xml, "object" );
			if ( attr1 ) {
				lmos.StartPosHelp( attr1 );
				ackReturn( id, kSuccess );
			} else {
				ackReturn( id, kBadParam );
			}
			break;
		case kExit:
			ackReturn( id, kSuccess );
			emit finished();
			break;
		case kReference:
			lmos.Reference();
			ackReturn( id, kSuccess );
			break;
		case kSetMOLayer:
			// lmos.StartPosHelp(cmd.attribute("object").value());
			ackReturn( id, kFailure );
			break;
		case kLayerSetHeight:
			attr1 = BpGetMessageAttribute( xml, "layer" );
			attr2 = BpGetMessageAttribute( xml, "height" );
			if ( attr1 && attr2 ) {
				lmos.LayerSetHeightZAxis( attr1, strtof( attr2, NULL ) );
				ackReturn( id, kSuccess );
			} else {
				ackReturn( id, kBadParam );
			}
			break;
		case kLayerSetLaserable:
			attr1 = BpGetMessageAttribute( xml, "layer" );
			attr2 = BpGetMessageAttribute( xml, "laserable" );
			if ( attr1 && attr2 ) {
				lmos.LayerSetLaserable( attr1, strtod( attr2, NULL ) );
				ackReturn( id, kSuccess );
			} else {
				ackReturn( id, kBadParam );
			}
			break;
		case kLayerSetVisible:
			attr1 = BpGetMessageAttribute( xml, "layer" );
			attr2 = BpGetMessageAttribute( xml, "visible" );
			if ( attr1 && attr2 ) {
				lmos.LayerSetVisible( attr1, strtod( attr2, NULL ) );
				ackReturn( id, kSuccess );
			} else {
				ackReturn( id, kBadParam );
			}
			break;
		case kLayerSetExportable:
			attr1 = BpGetMessageAttribute( xml, "layer" );
			attr2 = BpGetMessageAttribute( xml, "exportable" );
			if ( attr1 && attr2 ) {
				lmos.LayerSetExportable( attr1, strtod( attr2, NULL ) );
				ackReturn( id, kSuccess );
			} else {
				ackReturn( id, kBadParam );
			}
			break;
		case kSetDimension:
			attr1 = BpGetMessageAttribute( xml, "object" );
			attr2 = BpGetMessageAttribute( xml, "x" );
			attr3 = BpGetMessageAttribute( xml, "y" );
			if ( attr1 && attr2 && attr3 ) {
				lmos.SetDimension( attr1, strtod( attr2, NULL ), strtod( attr3, NULL ) );
				ackReturn( id, kSuccess );
			} else {
				ackReturn( id, kBadParam );
			}
			break;
		case kSetPosValues:
			attr1 = BpGetMessageAttribute( xml, "object" );
			attr2 = BpGetMessageAttribute( xml, "x" );
			attr3 = BpGetMessageAttribute( xml, "y" );
			attr4 = BpGetMessageAttribute( xml, "r" );
			if ( attr1 && attr2 && attr3 && attr4 ) {
				lmos.SetPosValues(
					attr1, strtod( attr2, NULL ), strtod( attr3, NULL ), strtod( attr4, NULL ) );
				ackReturn( id, kSuccess );
			} else {
				ackReturn( id, kBadParam );
			}
			break;
		case kZoomWindow:
			attr1 = BpGetMessageAttribute( xml, "x1" );
			attr2 = BpGetMessageAttribute( xml, "y1" );
			attr3 = BpGetMessageAttribute( xml, "x2" );
			attr4 = BpGetMessageAttribute( xml, "y2" );
			if ( attr1 && attr2 && attr3 && attr4 ) {
				lmos.ZoomWindow( strtod( attr1, NULL ),
						 strtod( attr2, NULL ),
						 strtod( attr3, NULL ),
						 strtod( attr4, NULL ) );
				ackReturn( id, kSuccess );
			} else {
				ackReturn( id, kBadParam );
			}
			break;
		case kShowMarkingArea:
			lmos.ShowMarkingArea();
			ackReturn( id, kSuccess );
			break;
		case kAddQpSet:
			attr1 = BpGetMessageAttribute( xml, "name" );
			attr2 = BpGetMessageAttribute( xml, "current" );
			attr3 = BpGetMessageAttribute( xml, "speed" );
			attr4 = BpGetMessageAttribute( xml, "frequency" );
			if ( attr1 && attr2 && attr3 && attr4 ) {
				lmos.AddQPSet( attr1, atoi( attr2 ), atoi( attr3 ), atoi( attr4 ) );
				ackReturn( id, kSuccess );
			} else {
				ackReturn( id, kBadParam );
			}
			break;
		case kSaveQpSets:
			lmos.SaveQPSets();
			ackReturn( id, kSuccess );
			break;
		case kWriteIoBit:
			attr1 = BpGetMessageAttribute( xml, "bitfunction" );
			attr2 = BpGetMessageAttribute( xml, "value" );
			if ( attr1 && attr2 ) {
				lmos.WriteIOBit( attr1, atoi( attr2 ) );
				ackReturn( id, kSuccess );
			} else {
				ackReturn( id, kBadParam );
			}
			break;
		case kReadByte:
			attr1 = BpGetMessageAttribute( xml, "port" );
			attr2 = BpGetMessageAttribute( xml, "mask" );
			if ( attr1 && attr2 ) {
				retval_num = lmos.ReadByte( QString( attr1 ).toInt(), QString( attr2 ).toInt() );
				if ( retval_num == kInvalid ) {
					ackReturn( id, kFailure );
				} else {
					payload = SdsFromLong( retval_num );
					ReplyWithPayload( id, kSuccess, payload );
				}
			} else {
				ackReturn( id, kBadParam );
			}
			break;
		case kReadIOBit:
			attr1 = BpGetMessageAttribute( xml, "bitfunction" );
			if ( attr1 ) {
				retval_num = lmos.ReadIOBit( QString( attr1 ) );
				if ( retval_num == kInvalid ) {
					ackReturn( id, kFailure );
				} else {
					ReplyWithPayload( id, kSuccess, retval_num == 1 ? "1" : "0" );
				}
			} else {
				ackReturn( id, kBadParam );
			}
			break;
		case kGetW:
			retval_double = lmos.GetW();
			if ( retval_double == kInvalid ) {
				ackReturn( id, kFailure );
			} else {
				payload = SdsFromLong( retval_double );
				ReplyWithPayload( id, kSuccess, payload );
			}
			break;
		case kPatchFlexibleShadows:
			attr1 = BpGetMessageAttribute( xml, "filename" );
			if ( attr1 ) {
				const unsigned char findstring[]    = { 0xff, 0xfe, 0xff, 0x00, 0x01,
									0x00, 0x00, 0x00, 0x30 };
				const unsigned char replacestring[] = { 0xff, 0xfe, 0xff, 0x00, 0x01,
									0x00, 0x00, 0x00, 0xb0 };
				if ( lmos.PatchVLMFile( attr1, 9, findstring, replacestring ) == kSuccess ) {
					ackReturn( id, kSuccess );
				} else {
					ackReturn( id, kFailure );
				}
			} else {
				ackReturn( id, kFailure );
			}
			break;
		case kSaveVLM:
			attr1 = BpGetMessageAttribute( xml, "filename" );
			if ( attr1 ) {
				if ( lmos.SaveVLM( attr1 ) ) {
					emit settray( "Saved as " + QString( attr1 ) );
					ackReturn( id, kSuccess );
				} else {
					emit settray( "Failed to save VLM file" );
					ackReturn( id, kFailure );
				}
			} else {
				ackReturn( id, kFailure );
			}
			break;
		case kLoadVLM:
			attr1 = BpGetMessageAttribute( xml, "filename" );
			if ( attr1 ) {
				ackReturn( id, lmos.LoadVLM( attr1 ) ? kSuccess : kFailure );
			} else {
				ackReturn( id, kBadParam );
			}
			break;
		case kMoveZ:
			attr1 = BpGetMessageAttribute( xml, "height" );
			if ( attr1 ) {
				lmos.MoveZ( strtof( attr1, NULL ) );
				ackReturn( id, kSuccess );
			} else {
				ackReturn( id, kBadParam );
			}
			break;
		case kMoveW:
			attr1 = BpGetMessageAttribute( xml, "rotation" );
			if ( attr1 ) {
				lmos.MoveW( strtod( attr1, NULL ) );
				ackReturn( id, kSuccess );
			} else {
				ackReturn( id, kBadParam );
			}
			break;
		case kGetPng:
			pixmap = lmos.GrabWindow();
			buffer.open( QIODevice::WriteOnly );
			pixmap.save( &buffer, "PNG" );
			bArray = bArray.toBase64();
			/* ackMessage(id, bArray); */
			// QByteArray => std::string
			stdString = std::string( bArray.constData(), bArray.length() );

			// Don't send an ack - the returned image will be the reply
			// ackReturn(id, kSuccess);

			// TODO: Encode the id into the message
			ReplyWithPayload( id, kSuccess, stdString.c_str() );
			// bp_sendMessage(blast, stdString.c_str());
			// SendMessageBp(blast, "id", QString::number(id).toStdString().c_str(), stdString.c_str());
			/* #if DEBUG_LEVEL == 3 */
			/* log(stdString.c_str()); */
			qDebug() << "bArray size: " << bArray.size();
			qDebug() << "strlen: " << strlen( stdString.c_str() );
			log( "bArray size:" );
			log( QString::number( bArray.size() ) );
			log( "strlen:" );
			log( QString::number( strlen( stdString.c_str() ) ) );
			/* #endif */
			break;
		case kSuppressRedraw:
			// lmos.SuppressRedraw(QString(cmd.attribute("redraw").value()).toInt());
			ackReturn( id, kSuccess );
			break;
		case kForceRedraw:
			lmos.ForceRedraw();
			ackReturn( id, kSuccess );
			break;
		case kCreateLMOS:
			lmos.CreateControl();
			lmos.ConnectSignals();
			ackReturn( id, kSuccess );
			break;
		case kDestroyLMOS:
			lmos.DisconnectSignals();
			lmos.DestroyControl();
			ackReturn( id, kSuccess );
			break;
		case kConnectSignals:
			lmos.ConnectSignals();
			ackReturn( id, kSuccess );
			break;
		case kDisconnectSignals:
			lmos.DisconnectSignals();
			ackReturn( id, kSuccess );
			break;
		case kShowLMOS:
			lmos.ShowWindow();
			ackReturn( id, kSuccess );
			break;
		case kHideLMOS:
			lmos.HideWindow();
			ackReturn( id, kSuccess );
			break;
		case kClearLog:
			emit ClearLog();
			ackReturn( id, kSuccess );
			break;
		case kResetRetvalDb:
			BpFreeRetvalDb( this->blast );
			ackReturn( id, kSuccess );
			break;
		case kGetGeoList:
			result_qs = lmos.GetGeoList();
			result_ba = result_qs.toLocal8Bit();
			payload	  = SdsNew( result_ba.data() );
			ReplyWithPayload( id, kSuccess, payload );
			break;
		default:
			// We should not get here
			log( "Warning: default case label reached in Parser::parseCommand" );
			ackReturn( id, kFailure );
	}

	if ( command_string ) {
		SdsFree( command_string );
	}
	if ( id_string ) {
		SdsFree( id_string );
	}
	if ( message_string ) {
		SdsFree( message_string );
	}
	if ( payload ) {
		SdsFree( payload );
	}
	if ( attr1 ) {
		SdsFree( attr1 );
	}
	if ( attr2 ) {
		SdsFree( attr2 );
	}
	if ( attr3 ) {
		SdsFree( attr3 );
	}
	if ( attr4 ) {
		SdsFree( attr4 );
	}
}
