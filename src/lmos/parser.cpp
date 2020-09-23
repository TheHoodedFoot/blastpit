#include "parser.hpp"
#include <QDateTime>
#include <QSettings>
#include <QTime>
#include <QtCore>
#include <cstring>
#include <iostream>
#include <sstream>
#include <string>
#include "blastpit.h"

#define MUTEX_COUNT 10	// How many cycles to allow mutex to stay locked

Parser::Parser(QObject *parent) : QObject(parent)
{
	QObject::connect(&lmos, SIGNAL(log(QString)), this, SLOT(log(QString)));
	QObject::connect(&lmos, SIGNAL(log(int, const char *, QString)), this, SLOT(log(int, const char *, QString)));
	QObject::connect(&lmos, SIGNAL(ack(QString)), this, SLOT(ack(QString)));
	QObject::connect(&lmos, SIGNAL(sendEvent(int, QString)), this, SLOT(SendSignal(int, QString)));

	blast = blastpitNew();
	assert(blast);

	// Set the object
	// void (*func)(void *
	registerCallback(blast, &messageReceivedCallback);
	registerObject(blast, (void *)this);
	mutex = 0;

	wsConnect();

	timer = new QTimer(this);
	connect(timer, SIGNAL(timeout()), this, SLOT(update()));

	// TODO: The timer amount has a huge effect on the responsiveness
	// when running under Wine. Find out why.
	timer->start(100);
}

Parser::~Parser()
{
	timer->stop();
	disconnectFromServer(blast);
	blastpitDelete(blast);
}

void
Parser::wsConnect()
{  // (Re)try connection to server

	static QDateTime last_connection_attempt = QDateTime::currentDateTime().addSecs(-10);
	if (QDateTime::currentDateTime() < last_connection_attempt.addSecs(3))
		return;
	last_connection_attempt = QDateTime::currentDateTime();

	QSettings traySettings("Rfbevanco", "lmos-tray");
	QString	  wsserver = traySettings.value("wsServer").toString();

	if (wsserver.size() < MIN_SERVER_STRING_LEN)
		return;

	QByteArray  ba	   = wsserver.toLocal8Bit();
	const char *c_str2 = ba.data();

	log("Parser::wsConnect : Trying to connect to server " + traySettings.value("wsServer").toString());
	connectToServer(blast, c_str2, 1000);
}

void
Parser::messageReceivedCallback(void *ev_data, void *object)
{
	Parser *psr = (Parser *)object;

	psr->mutex = MUTEX_COUNT;

	// This is not null terminated
	WsMessage msg_data = ConvertCallbackData(ev_data);

	char *msg_data_string = (char *)alloca(msg_data.size + 1);
	strncpy(msg_data_string, (const char *)msg_data.data, msg_data.size);
	*(msg_data_string + msg_data.size + 1) = 0;

	psr->ProcessMessageBlock(msg_data_string);

	psr->mutex = 0;
}

void
Parser::ProcessMessageBlock(const char *msg_data_string)
{  // Extract and process individual messages from a string

	char *message	= SdsEmpty();
	int   msg_count = BpGetMessageCount((const char *)msg_data_string);

	log("Processing " + QString::number(msg_count) + " messages");

	for (int i = 0; i < msg_count; i++) {
		message = BpGetMessageByIndex((const char *)msg_data_string, i);

		// TODO: Loop through all depends comma-separated dependencies
		char *depends = BpGetMessageAttribute(message, "depends");
		if (!depends || BpQueryRetvalDb(blast, atoi(depends)) == kSuccess) {
			parseCommand(message);
		} else {
			char *id = BpGetMessageAttribute(message, "id");
			log("Message with id ");
			log(id);
			log(" has failed dependency ");
			log(depends);
			ackReturn(atoi(id), kFailure);
			SdsFree(id);
		};
		SdsFree(depends);
	}
	log("Finished Parsing Messages");

	SdsFree(message);
}

void
Parser::update()
{  // Poll for messages

	if (mutex) {
		mutex--;
		return;
	}

	// laserStatus:
	// 0 = no network, 1 = network no lmos loaded, 2 = net and lmos
	static int laserStatus = -1;


	if (!bp_isConnected((blast))) {
		if (laserStatus != 0) {
			emit seticon("noconnection");
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
		if (laserStatus < 1) {
			log("Parser::update : Connected.");
			QSettings traySettings("Rfbevanco", "lmos-tray");
			QString	  wsserver = "Connected to server " + traySettings.value("wsServer").toString();
			emit	  settray(wsserver);
			emit	  seticon("nolaser");
			laserStatus = 1;

			lmos.CreateControl();
			// lmos.ShowWindow();
			lmos.ConnectSignals();
		}
	}

	if (blast->message_queue) {
		// This will automatically poll messages as well
		BpUploadQueuedMessages(blast);
	} else {
		pollMessages(blast);
	}
}

void
Parser::log(QString string)
{
	emit sendlog(string);
}

void
Parser::log(int level, const char *function, QString entry)
{
	QString time	= QTime::currentTime().toString("hh:mm:ss.zzz");
	QString log	= QString::number(level);
	QString logFunc = QString(function);
	emit	sendlog(time + ": (" + log + ") [" + logFunc + "] " + entry);
}

void
Parser::ack(QString message)
{ /* Send a network acknowledgment */

	log("[ack]");
	bp_sendMessage(blast, message.toStdString().c_str());
}

void
Parser::ackReturn(int id, int retval)
{  // Send a network acknowledgment

	QString message = QString::number(retval);
	log("[ackReturn] (" + QString::number(id) + ") " + QString(bpRetvalName(retval)));
	BpAddRetvalToDb(this->blast, (IdAck){id, retval, NULL});
	QueueAckRetval(blast, id, retval);
	BpUploadQueuedMessages(blast);
}

void
Parser::ReplyWithPayload(int id, int retval, const char *payload)
{  // Reply to a commmand with a payload string

	log("[ReplyWithPayload] #" + QString::number(id) + " : " + QString(payload));
	QueueReplyPayload(blast, id, retval, payload);
	BpUploadQueuedMessages(blast);
}

void
Parser::SendSignal(int signal, QString message)
{  // Used by Lmos to send a signal
	// Assume that message is not null terminated

	char *signal_string = (char *)alloca(4);
	snprintf(signal_string, 4, "%d", signal);
	char *message_string = (char *)alloca(message.size() + 1);
	strncpy(message_string, message.toStdString().c_str(), message.size());
	*(message_string + message.size()) = 0;

	log("Parser::SendSignal");
	log(signal_string);
	log(message_string);
	QueueSignal(blast, signal, message_string);
	// QueueSignal(blast, signal, "TEST SIGNAL");
	BpUploadQueuedMessages(blast);
}

void
Parser::parseCommand(const char *xml)
{
	char *id_string	     = BpGetMessageAttribute(xml, "id");
	int   id	     = atoi(id_string);
	char *command_string = BpGetMessageAttribute(xml, "command");
	int   command	     = atoi(command_string);
	char *message_string;
	char *attr1, *attr2, *attr3, *attr4;
	int   retval_num;
	char *payload;

	log("(" + QTime::currentTime().toString("hh:mm:ss.zzz") + ") #" + QString::number(id) + ": " +
	    QString(bpCommandName(command)));

	std::stringstream out;
	QString		  time = QTime::currentTime().toString("hh:mm:ss.zzz");
	QPixmap		  pixmap;
	QByteArray	  bArray;
	QBuffer		  buffer(&bArray);
	std::string	  stdString;

	switch (command) {
		case kIsLmosRunning:
			ackReturn(id, kSuccess);
			break;
		case kGetVersion:
			/* Create an XML with the git version inside */
			// free(versionString);
			break;
		case kInitMachine:
			lmos.InitMachine();
			ackReturn(id, kSuccess);
			break;
		case kTermMachine:
			lmos.TermMachine();
			ackReturn(id, kSuccess);
			break;
		case kClearQpSets:
			lmos.ClearQPSets();
			lmos.SaveQPSets();
			ackReturn(id, kSuccess);
			break;
		case kCancelJob:
			ackReturn(id, lmos.CancelJob());
			break;
		case kImportXML:
			message_string = BpGetChildNodeAsString(xml, "DRAWING");
			ackReturn(id, lmos.LoadXML(QString::fromStdString(message_string)) ? kSuccess : kFailure);
			SdsFree(message_string);
			break;
		case kStopPosHelp:
			lmos.StopPosHelp();
			ackReturn(id, kSuccess);
			break;
		case kStartMarking:
			ackReturn(id, lmos.StartMarking());
			break;
		case kLoadJob:
			ackReturn(id, lmos.LoadJob());
			break;
		case kStopMarking:
			lmos.StopMarking();
			ackReturn(id, kSuccess);
			break;
		case kMOSetLaserable:
			// lmos.SetLaserable(cmd.attribute("object").value(), true);
			ackReturn(id, kSuccess);
			break;
		case kMOUnsetLaserable:
			lmos.UnsetLaserableAllObjects();
			ackReturn(id, kSuccess);
			break;
		case kSelfTest:
			lmos.Test();
			ackReturn(id, kSuccess);
			break;
		case kStartPosHelp:
			// lmos.StartPosHelp(cmd.attribute("object").value());
			ackReturn(id, kSuccess);
			break;
		case kExit:
			// emit finished();
			ackReturn(id, kSuccess);
			break;
		case kReference:
			lmos.Reference();
			ackReturn(id, kSuccess);
			break;
		case kSetMOLayer:
			// lmos.StartPosHelp(cmd.attribute("object").value());
			ackReturn(id, kSuccess);
			break;
		case kLayerSetHeight:
			// lmos.LayerSetHeightZAxis(cmd.attribute("layer").value(),
			// QString(cmd.attribute("height").value()).toFloat());
			ackReturn(id, kSuccess);
			break;
		case kLayerSetLaserable:
			// lmos.LayerSetLaserable(cmd.attribute("layer").value(),
			// QString(cmd.attribute("laserable").value()).toInt());
			ackReturn(id, kSuccess);
			break;
		case kLayerSetVisible:
			// lmos.LayerSetVisible(cmd.attribute("layer").value(),
			// QString(cmd.attribute("visible").value()).toInt());
			ackReturn(id, kSuccess);
			break;
		case kLayerSetExportable:
			// lmos.LayerSetExportable(cmd.attribute("layer").value(),
			// QString(cmd.attribute("exportable").value()).toInt());
			ackReturn(id, kSuccess);
			break;
		case kSetDimension:
			// lmos.SetDimension(cmd.attribute("object").value(),
			// QString(cmd.attribute("x").value()).toDouble(),
			// QString(cmd.attribute("y").value()).toDouble());
			ackReturn(id, kSuccess);
			break;
		case kSetPosValues:
			// lmos.SetPosValues(cmd.attribute("object").value(),
			// QString(cmd.attribute("x").value()).toDouble(),
			// QString(cmd.attribute("y").value()).toDouble(),
			// QString(cmd.attribute("r").value()).toDouble());
			ackReturn(id, kSuccess);
			break;
		case kZoomWindow:
			// lmos.ZoomWindow(QString(cmd.attribute("x1").value()).toInt(),
			// QString(cmd.attribute("y1").value()).toInt(),
			// QString(cmd.attribute("x2").value()).toInt(),
			// QString(cmd.attribute("y2").value()).toInt());
			ackReturn(id, kSuccess);
			break;
		case kShowMarkingArea:
			lmos.ShowMarkingArea();
			ackReturn(id, kSuccess);
			break;
		case kAddQpSet:
			attr1 = BpGetMessageAttribute(xml, "name");
			attr2 = BpGetMessageAttribute(xml, "current");
			attr3 = BpGetMessageAttribute(xml, "speed");
			attr4 = BpGetMessageAttribute(xml, "frequency");
			lmos.AddQPSet(attr1, atoi(attr2), atoi(attr3), atoi(attr4));
			lmos.SaveQPSets();
			ackReturn(id, kSuccess);
			SdsFree(attr4);
			SdsFree(attr3);
			SdsFree(attr2);
			SdsFree(attr1);
			break;
		case kWriteIoBit:
			attr1 = BpGetMessageAttribute(xml, "bitfunction");
			attr2 = BpGetMessageAttribute(xml, "value");
			lmos.WriteIOBit(attr1, atoi(attr2));
			ackReturn(id, kSuccess);
			break;
		case kReadByte:
			attr1	   = BpGetMessageAttribute(xml, "port");
			attr2	   = BpGetMessageAttribute(xml, "mask");
			retval_num = lmos.ReadByte(QString(attr1).toInt(), QString(attr2).toInt());
			if (retval_num == kInvalid) {
				ackReturn(id, kFailure);
			} else {
				payload = SdsFromLong(retval_num);
				ReplyWithPayload(id, kSuccess, payload);
				SdsFree(payload);
			}
			break;
		case kReadIOBit:
			attr1	   = BpGetMessageAttribute(xml, "bitfunction");
			retval_num = lmos.ReadIOBit(QString(attr1));
			if (retval_num == kInvalid) {
				ackReturn(id, kFailure);
			} else {
				ReplyWithPayload(id, kSuccess, retval_num == 1 ? "1" : "0");
			}
			break;
		case kSaveVLM:
			attr1 = BpGetMessageAttribute(xml, "filename");
			BpGetChildNodeAsString(xml, "DRAWING");
			if (attr1) {
				if (lmos.SaveVLM(attr1)) {
					emit settray("Saved as " + QString(attr1));
					ackReturn(id, kSuccess);
				} else {
					ackReturn(id, kFailure);
				}
				SdsFree(attr1);
			}
			break;
		case kLoadVLM:
			// lmos.LoadVLM(QString(cmd.attribute("filename").value()));
			ackReturn(id, kSuccess);
			break;
		case kMoveZ:
			// lmos.MoveZ(QString(cmd.attribute("height").value()).toFloat());
			ackReturn(id, kSuccess);
			break;
		case kMoveW:
			attr1 = BpGetMessageAttribute(xml, "rotation");
			if (attr1) {
				lmos.MoveW(strtod(attr1, NULL));
				SdsFree(attr1);
				ackReturn(id, kSuccess);
			} else {
				ackReturn(id, kFailure);
			}
			break;
		case kGetPng:
			pixmap = lmos.GrabWindow();
			buffer.open(QIODevice::WriteOnly);
			pixmap.save(&buffer, "PNG");
			bArray = bArray.toBase64();
			/* ackMessage(id, bArray); */
			// QByteArray => std::string
			stdString = std::string(bArray.constData(), bArray.length());

			// Don't send an ack - the returned image will be the reply
			// ackReturn(id, kSuccess);

			// TODO: Encode the id into the message
			ReplyWithPayload(id, kSuccess, stdString.c_str());
			// bp_sendMessage(blast, stdString.c_str());
			// SendMessageBp(blast, "id", QString::number(id).toStdString().c_str(), stdString.c_str());
			/* #if DEBUG_LEVEL == 3 */
			/* log(stdString.c_str()); */
			qDebug() << "bArray size: " << bArray.size();
			qDebug() << "strlen: " << strlen(stdString.c_str());
			log("bArray size:");
			log(QString::number(bArray.size()));
			log("strlen:");
			log(QString::number(strlen(stdString.c_str())));
			/* #endif */
			break;
		case kSuppressRedraw:
			// lmos.SuppressRedraw(QString(cmd.attribute("redraw").value()).toInt());
			ackReturn(id, kSuccess);
			break;
		case kForceRedraw:
			lmos.ForceRedraw();
			ackReturn(id, kSuccess);
			break;
		case kCreateLMOS:
			lmos.CreateControl();
			ackReturn(id, kSuccess);
			break;
		case kDestroyLMOS:
			lmos.DestroyControl();
			ackReturn(id, kSuccess);
			break;
		case kConnectSignals:
			lmos.ConnectSignals();
			ackReturn(id, kSuccess);
			break;
		case kDisconnectSignals:
			lmos.DisconnectSignals();
			ackReturn(id, kSuccess);
			break;
		case kShowLMOS:
			lmos.ShowWindow();
			ackReturn(id, kSuccess);
			break;
		case kHideLMOS:
			lmos.HideWindow();
			ackReturn(id, kSuccess);
			break;
		case kClearLog:
			emit ClearLog();
			ackReturn(id, kSuccess);
			break;
		case kResetRetvalDb:
			BpFreeRetvalDb(this->blast);
			ackReturn(id, kSuccess);
			break;
		default:
			// We should not get here
			log("Warning: default case label reached in Parser::parseCommand");
			ackReturn(id, kFailure);
	}

	SdsFree(command_string);
	SdsFree(id_string);
}
