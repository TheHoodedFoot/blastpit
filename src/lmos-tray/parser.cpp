#include "parser.hpp"
#include <QTime>
#include <QtCore>
/* #include "network.hpp" */
#include "pugixml.hpp"
#include "t_common.h"

#include <unistd.h>
#include <sstream>

Parser::Parser(QObject *parent) : QObject(parent)
{
	QObject::connect(&lmos, SIGNAL(log(QString)), this,
			 SLOT(log(QString)));
	QObject::connect(&lmos, SIGNAL(log(int, const char *, QString)), this,
			 SLOT(log(int, const char *, QString)));
	QObject::connect(&lmos, SIGNAL(ack(QString)), this,
			 SLOT(ack(QString)));

	blast = bp_new();
	bp_connectToServer(blast, SERVER, "lmos", 1000);
	usleep(200000);
	bp_subscribe(blast, "lmos", 1000);

	timer = new QTimer(this);
	connect(timer, SIGNAL(timeout()), this, SLOT(update()));
	timer->start(200);
}

Parser::~Parser()
{
	timer->stop();
	bp_unsubscribe(blast, "lmos", 1000);
	bp_disconnectFromServer(blast);
	bp_delete(blast);
}

void
Parser::update()
{ /* Examine the mqtt message stack */

	if (bp_getMessageCount(blast)) {
		t_bp_message msg = bp_getNewestMessage(blast);
		pugi::xml_document xml;
		xml.load_buffer(msg.data, msg.length);

		int command =
			QString(xml.child("command").child_value()).toInt();
		// qInfo() << "Parser::update() Command string: " << command;
		/* emit sendlog(command); */
		if (command) {
			parseCommand(QString(xml.child("command")
						     .attribute("id")
						     .value())
					     .toInt(),
				     command, xml);
		} else {
			// qInfo() << "Couldn't parse xml (shown below).";
			// qInfo() << QString(msg.data);
		}
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
	QString time = QTime::currentTime().toString("hh:mm:ss.zzz");
	QString log = QString::number(level);
	QString logFunc = QString(function);
	emit sendlog(time + ": (" + log + ") [" + logFunc + "] " + entry);
}

void
Parser::ack(QString message)
{ /* Send a network acknowledgment */

	bp_sendMessage(blast, "broadcast", message.toStdString().c_str());
}

void
Parser::ackReturn(int id, int retval)
{ /* Send a network acknowledgment */

	QString message = QString::number(id) + "," + QString::number(retval);
	bp_sendMessage(blast, "broadcast", message.toStdString().c_str());
}

void
Parser::parseCommand(int id, int command, pugi::xml_document &xml)
{
	log("(" + QTime::currentTime().toString("hh:mm:ss.zzz") + ") " +
	    "ID: " + QString::number(id) +
	    ", Command: " + QString::number(command));

	std::stringstream out;
	QString time = QTime::currentTime().toString("hh:mm:ss.zzz");
	pugi::xml_node cmd = xml.child("command");
	QPixmap pixmap;
	QByteArray bArray;
	QBuffer buffer(&bArray);

	switch (command) {
		case kInitMachine:
			lmos.InitMachine();
			break;
		case kTermMachine:
			lmos.TermMachine();
			break;
		case kClearQpSets:
			lmos.ClearQPSets();
			lmos.SaveQPSets();
			break;
		case kImportXML:
			/* xml.child("DRAWING").remove_child("COMMAND"); */
			xml.child("command").child("DRAWING").print(
				out, "", pugi::format_raw);
			/* xml.save(out); */
			/* qInfo() << "Drawing string: " */
			/* 	<< QString::fromStdString(out.str()); */
			ackReturn(id, lmos.LoadXML(QString::fromStdString(
					      out.str())));
			break;
		case kStopPosHelp:
			lmos.StopPosHelp();
			ackReturn(id, 0);
			break;
		case kStartMarking:
			lmos.StartMarking();
			ackReturn(id, 0);
			break;
		case kStopMarking:
			lmos.StopMarking();
			ackReturn(id, 0);
			break;
		case kSelfTest:
			lmos.Test();
			break;
		case kStartPosHelp:
			lmos.StartPosHelp(cmd.attribute("object").value());
			ackReturn(id, 0);
			break;
		case kExit:
			emit finished();
			ackReturn(id, 0);
			break;
		case kReference:
			lmos.Reference();
			ackReturn(id, 0);
			break;
		case kSetMOLayer:
			lmos.StartPosHelp(cmd.attribute("object").value());
			ackReturn(id, 0);
			break;
		case kLayerSetHeight:
			lmos.LayerSetHeightZAxis(
				cmd.attribute("layer").value(),
				QString(cmd.attribute("height").value())
					.toFloat());
			ackReturn(id, 0);
			break;
		case kLayerSetLaserable:
			lmos.LayerSetLaserable(
				cmd.attribute("layer").value(),
				QString(cmd.attribute("laserable").value())
					.toInt());
			ackReturn(id, 0);
			break;
		case kLayerSetVisible:
			lmos.LayerSetVisible(
				cmd.attribute("layer").value(),
				QString(cmd.attribute("visible").value())
					.toInt());
			ackReturn(id, 0);
			break;
		case kLayerSetExportable:
			lmos.LayerSetExportable(
				cmd.attribute("layer").value(),
				QString(cmd.attribute("exportable").value())
					.toInt());
			ackReturn(id, 0);
			break;
		case kCancelJob:
			lmos.CancelJob();
			ackReturn(id, 0);
			break;
		case kSetDimension:
			lmos.SetDimension(cmd.attribute("object").value(),
					  QString(cmd.attribute("x").value())
						  .toDouble(),
					  QString(cmd.attribute("y").value())
						  .toDouble());
			ackReturn(id, 0);
			break;
		case kSetPosValues:
			lmos.SetPosValues(cmd.attribute("object").value(),
					  QString(cmd.attribute("x").value())
						  .toDouble(),
					  QString(cmd.attribute("y").value())
						  .toDouble(),
					  QString(cmd.attribute("r").value())
						  .toDouble());
			ackReturn(id, 0);
			break;
		case kZoomWindow:
			lmos.ZoomWindow(
				QString(cmd.attribute("x1").value()).toInt(),
				QString(cmd.attribute("y1").value()).toInt(),
				QString(cmd.attribute("x2").value()).toInt(),
				QString(cmd.attribute("y2").value()).toInt());
			break;
		case kShowMarkingArea:
			lmos.ShowMarkingArea();
			break;
		case kAddQpSet:
			/* qInfo() << "Parsing qpsets"; */
			for (pugi::xml_node qpset =
				     xml.child("command").child("qpset");
			     qpset; qpset = qpset.next_sibling("qpset")) {
				lmos.AddQPSet(
					qpset.attribute("name").value(),
					QString(qpset.attribute("current")
							.value())
						.toDouble(),
					QString(qpset.attribute("speed")
							.value())
						.toInt(),
					QString(qpset.attribute("frequency")
							.value())
						.toInt());
			}
			/* qInfo() << "Finished parsing qpsets"; */
			lmos.SaveQPSets();
			ackReturn(id, 0);
			break;
		/* case kWriteIOBit: */
		/* 	lmos.WriteIOBit(getXml(xml, "bitfunction"), */
		/* 			getXmlBool(xml, "value")); */
		/* 	break; */
		/* case kReadByte: */
		/* 	lmos.ReadByte(getXmlInt(xml, "port"), */
		/* 		      getXmlInt(xml, "mask")); */
		/* 	break; */
		/* case kReadIOBit: */
		/* 	lmos.ReadIOBit(getXml(xml, "bitfunction")); */
		/* 	break; */
		case kSaveVLM:
			lmos.SaveVLM(cmd.attribute("filename").value());
			ackReturn(id, 0);
			emit settray(
				"Saving as " +
				QString(cmd.attribute("filename").value()));
			break;
		case kLoadVLM:
			lmos.LoadVLM(
				QString(cmd.attribute("filename").value()));
			break;
		case kMoveZ:
			lmos.MoveZ(QString(cmd.attribute("height").value())
					   .toFloat());
			ackReturn(id, 0);
			break;
		case kMoveW:
			lmos.MoveW(QString(cmd.attribute("rotation").value())
					   .toDouble());
			break;
		case kGetPng:
			pixmap = lmos.GrabWindow();
			buffer.open(QIODevice::WriteOnly);
			pixmap.save(&buffer, "PNG");
			bArray = bArray.toBase64();
			ack(QString::number(id) + "," + bArray);
			break;
		case kNoCommand: /* Dummy command for unit tests */
		case 99:	 /* Dummy command for unit tests */
			ackReturn(id, 9);
			break;

		default:
			// We should not get here
			qDebug() << "Unknown command " << command
				 << " in parseCommand()";
	}
}

void
Parser::receiveEvent(QString event)
{  // Signal has been received from Lmos

	qDebug() << "myEvent: " << event;
}
