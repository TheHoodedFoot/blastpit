#include "parser.hpp"
#include <math.h>
#include <QSettings>
#include <QTime>
#include <QtCore>
/* #include "network.hpp" */
// #include "../../res/cfg/t_common.h"
#include "mongoose.h"
#include "pugixml.hpp"

//#include <unistd.h>
#include <sstream>

Parser::Parser(QObject *parent)
	: QObject(parent)
{
	QObject::connect(&lmos, SIGNAL(log(QString)), this, SLOT(log(QString)));
	QObject::connect(&lmos, SIGNAL(log(int, const char *, QString)), this, SLOT(log(int, const char *, QString)));
	QObject::connect(&lmos, SIGNAL(ack(QString)), this, SLOT(ack(QString)));
	QObject::connect(&lmos, SIGNAL(sendEvent(int, QString)), this, SLOT(ackMessage(int, QString)));

	blast = blastpitNew();
	assert(blast);

	// Set the object
	// void (*func)(void *
	registerCallbackCpp(blast, &messageReceivedCallback);
	registerObject(blast, (void *)this);

	wsConnect();

	timer = new QTimer(this);
	connect(timer, SIGNAL(timeout()), this, SLOT(update()));
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

	QSettings traySettings("Rfbevanco", "lmos-tray");
	QString wsserver = traySettings.value("wsServer").toString();

	if (wsserver.size() < MIN_SERVER_STRING_LEN)
		return;

	QByteArray ba = wsserver.toLocal8Bit();
	const char *c_str2 = ba.data();

	connectToServer(blast, c_str2, 1000);
}

void
Parser::messageReceivedCallback(void *ev_data, void *object)
{
	Parser *psr = (Parser *)object;

	// psr->log("received");
	struct websocket_message *wm = (struct websocket_message *)ev_data;
	// printf("%.*s\n", (int)wm->size, wm->data);

	pugi::xml_document xml;
	xml.load_buffer(wm->data, wm->size);

	int command = QString(xml.child("command").child_value()).toInt();
	if (command) {
		psr->parseCommand(QString(xml.child("command").attribute("id").value()).toInt(), command, xml);
	} else {
		psr->log("Couldn't parse xml (shown below)");
		psr->log(QString((const char *)wm->data));
	}
}

void
Parser::update()
{  // Poll for messages

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
		QSettings traySettings("Rfbevanco", "lmos-tray");
		log("Parser::update : Trying to connect to server " + traySettings.value("wsServer").toString());
		this->wsConnect();

		// This is a static member function of QThread
		Sleep(1);

	} else {
		if (laserStatus < 1) {
			log("Parser::update : Connected.");
			QSettings traySettings("Rfbevanco", "lmos-tray");
			QString wsserver = "Connected to server " + traySettings.value("wsServer").toString();
			emit settray(wsserver);
			emit seticon("nolaser");
			laserStatus = 1;

			lmos.CreateControl();
			lmos.ShowWindow();
			lmos.ConnectSignals();
		}
	}


	pollMessages(blast);

	// This works
	// bp_sendMessage(blast, 0, "test message");
}

void
Parser::log(QString string)
{
	emit sendlog(string);
}

void
Parser::log(int level, const char *function, QString entry)
{
	QString time = QTime::currentTime().toString("hh:mm:ss.zzz");
	QString log = QString::number(level);
	QString logFunc = QString(function);
	emit sendlog(time + ": (" + log + ") [" + logFunc + "] " + entry);
}

void
Parser::ack(QString message)
{ /* Send a network acknowledgment */

	log("[ack]");
	bp_sendMessage(blast, 0, message.toStdString().c_str());
}

void
Parser::ackReturn(int id, int retval)
{ /* Send a network acknowledgment */

	QString message = QString::number(retval);
	log("[ackReturn] (" + QString::number(id) + ") " + QString(bpRetvalName(retval)));
	bp_sendMessage(blast, id, message.toStdString().c_str());
}

void
Parser::ackMessage(int id, QString message)
{
	log("[ackMessage] id: " + QString::number(id) + "  " + message);
	bp_sendMessage(blast, id, message.toStdString().c_str());
}

void
Parser::parseCommand(int id, int command, pugi::xml_document &xml)
{
	qDebug() << "Parser::parseCommand";
	if (command >= BPCOMMAND_MAX) {
		log("Bad command number");
		ackReturn(id, kBadCommand);
		return;
	}

	log("(" + QTime::currentTime().toString("hh:mm:ss.zzz") + ") #" + QString::number(id) + ": " + QString(bpCommandString[command]));

	std::stringstream out;
	QString time = QTime::currentTime().toString("hh:mm:ss.zzz");
	pugi::xml_node cmd = xml.child("command");
	QPixmap pixmap;
	QByteArray bArray;
	QBuffer buffer(&bArray);
	std::string stdString;

	/* Have to do this outside of switch because????? */

	switch (command) {
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
			/* xml.child("DRAWING").remove_child("COMMAND"); */
			xml.child("command").child("DRAWING").print(out, "", pugi::format_raw);
			/* xml.save(out); */
			/* qInfo() << "Drawing string: " */
			/* 	<< QString::fromStdString(out.str()); */
			ackReturn(id, lmos.LoadXML(QString::fromStdString(out.str())) ? kSuccess : kFailure);
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
			lmos.SetLaserable(cmd.attribute("object").value(), true);
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
			lmos.StartPosHelp(cmd.attribute("object").value());
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
			lmos.StartPosHelp(cmd.attribute("object").value());
			ackReturn(id, kSuccess);
			break;
		case kLayerSetHeight:
			lmos.LayerSetHeightZAxis(cmd.attribute("layer").value(), QString(cmd.attribute("height").value()).toFloat());
			ackReturn(id, kSuccess);
			break;
		case kLayerSetLaserable:
			lmos.LayerSetLaserable(cmd.attribute("layer").value(), QString(cmd.attribute("laserable").value()).toInt());
			ackReturn(id, kSuccess);
			break;
		case kLayerSetVisible:
			lmos.LayerSetVisible(cmd.attribute("layer").value(), QString(cmd.attribute("visible").value()).toInt());
			ackReturn(id, kSuccess);
			break;
		case kLayerSetExportable:
			lmos.LayerSetExportable(cmd.attribute("layer").value(), QString(cmd.attribute("exportable").value()).toInt());
			ackReturn(id, kSuccess);
			break;
		case kSetDimension:
			lmos.SetDimension(cmd.attribute("object").value(), QString(cmd.attribute("x").value()).toDouble(), QString(cmd.attribute("y").value()).toDouble());
			ackReturn(id, kSuccess);
			break;
		case kSetPosValues:
			lmos.SetPosValues(cmd.attribute("object").value(), QString(cmd.attribute("x").value()).toDouble(), QString(cmd.attribute("y").value()).toDouble(), QString(cmd.attribute("r").value()).toDouble());
			ackReturn(id, kSuccess);
			break;
		case kZoomWindow:
			lmos.ZoomWindow(QString(cmd.attribute("x1").value()).toInt(), QString(cmd.attribute("y1").value()).toInt(), QString(cmd.attribute("x2").value()).toInt(), QString(cmd.attribute("y2").value()).toInt());
			ackReturn(id, kSuccess);
			break;
		case kShowMarkingArea:
			lmos.ShowMarkingArea();
			ackReturn(id, kSuccess);
			break;
		case kAddQpSet:
			/* qInfo() << "Parsing qpsets"; */
			for (pugi::xml_node qpset = xml.child("command").child("qpset"); qpset; qpset = qpset.next_sibling("qpset")) {
				lmos.AddQPSet(qpset.attribute("name").value(), QString(qpset.attribute("current").value()).toDouble(), QString(qpset.attribute("speed").value()).toInt(), QString(qpset.attribute("frequency").value()).toInt());
			}
			/* qInfo() << "Finished parsing qpsets"; */
			lmos.SaveQPSets();
			ackReturn(id, kSuccess);
			break;
		case kWriteIOBit:
			lmos.WriteIOBit(cmd.attribute("bitfunction").value(), QString(cmd.attribute("value").value()).toInt());
			ackReturn(id, kSuccess);
			break;
		/* case kReadByte: */
		/* 	lmos.ReadByte(getXmlInt(xml, "port"), */
		/* 		      getXmlInt(xml, "mask")); */
		/* 	break; */
		/* case kReadIOBit: */
		/* 	lmos.ReadIOBit(getXml(xml, "bitfunction")); */
		/* 	break; */
		case kSaveVLM:
			if (lmos.SaveVLM(cmd.attribute("filename").value())) {
				emit settray(
					"Saved as " +
					QString(cmd.attribute("filename").value()));
				ackReturn(id, kSuccess);
			} else {
				ackReturn(id, kFailure);
			}
			break;
		case kLoadVLM:
			lmos.LoadVLM(QString(cmd.attribute("filename").value()));
			ackReturn(id, kSuccess);
			break;
		case kMoveZ:
			lmos.MoveZ(QString(cmd.attribute("height").value()).toFloat());
			ackReturn(id, kSuccess);
			break;
		case kMoveW:
			lmos.MoveW(QString(cmd.attribute("rotation").value()).toDouble());
			ackReturn(id, kSuccess);
			break;
		case kGetPng:
			pixmap = lmos.GrabWindow();
			buffer.open(QIODevice::WriteOnly);
			pixmap.save(&buffer, "PNG");
			bArray = bArray.toBase64();
			/* ackMessage(id, bArray); */
			// QByteArray => std::string
			stdString = std::string(bArray.constData(), bArray.length());
			ackReturn(id, kSuccess);
			bp_sendMessage(blast, id, stdString.c_str());
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
			lmos.SuppressRedraw(QString(cmd.attribute("redraw").value()).toInt());
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
		default:
			// We should not get here
			log("Warning: default case label reached in Parser::parseCommand");
			ackReturn(id, kFailure);
	}
}
