#include "parser.h"
#include <QTime>
#include <QtCore>
#include "network.h"
#include "pugixml.hpp"

Parser::Parser(QObject *parent) : QObject(parent)
{
	QObject::connect(&network, SIGNAL(receivedPacket()), this,
			 SLOT(getPacket()));
	QObject::connect(&lmos, SIGNAL(log(QString)), this,
			 SLOT(log(QString)));
	QObject::connect(&lmos, SIGNAL(log(int, const char *, QString)), this,
			 SLOT(log(int, const char *, QString)));
	network.listenTcp(LISTEN_PORT);
}

Parser::Parser(QObject *parent, int port) : QObject(parent)
{
	QObject::connect(&network, SIGNAL(receivedPacket()), this,
			 SLOT(getPacket()));
	QObject::connect(&lmos, SIGNAL(log(QString)), this,
			 SLOT(log(QString)));
	QObject::connect(&lmos, SIGNAL(log(int, const char *, QString)), this,
			 SLOT(log(int, const char *, QString)));

	// TODO: Just a test for the signal/slot. Delete this
	// when we create proper signal replies.
	QObject::connect(&lmos, SIGNAL(sigMouseDown()), this, SLOT(test()));
	QObject::connect(&lmos, SIGNAL(sendEvent(QString)), this,
			 SLOT(receiveEvent(QString)));
	QObject::connect(&lmos, SIGNAL(sigImageEnd2(double, int)), this,
			 SLOT(imageEnd2(double, int)));

	network.listenTcp(port);
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
Parser::retval(const char *function, int value)
{
	QString logFunc = QString(function);
	emit sendlog("(" + logFunc +
		     ") : Return value: " + QString::number(value));
}

int
Parser::getPacket()
{  // Get a packet from the network buffer and parse it

	int validPackets = 0;

	/* qDebug() << "getPacket(): Network packets waiting = " + */
	/* 		    QString::number(network.packetsWaiting()); */

	// Skip any packets whose parent has not completed yet, and
	// fail any packets whose parent has failed.
	for (int i = 0; i < network.packetsWaiting(); i++) {
		uint32_t parentId = network.getParent(i);
		if (parentId) {
			switch (logGetResult(parentId)) {
				case kSuccess:
					// Parent completed successfully so
					// continue parsing /*
					break;
				case kNoEntry:
				case kRunning:
					// Parent not completed,
					// so skip
					continue;
				case kFail:
				default:
					// Parent failed, so fail too
					uint32_t id, command;
					network.getPacketHeader(
						i, id, command, parentId);
					logWrite(id, command, kFail);
					continue;
			}
		}

		QByteArray packet;
		network.getPacket(i, packet);

		const char *pkt = packet.constData();
		struct BpPacket *header = (struct BpPacket *)pkt;

		/* qDebug() << "getPacket(): Parsing packet: id: " + */
		/* 		    QString::number(header->id) + */
		/* 		    ", command: " + */
		/* 		    QString::number(header->command); */

		parseCommand(header->id, header->command,
			     packet.right(packet.size() -
					  sizeof(struct BpPacket)));

		validPackets++;
	}

	// Now we are safe to drop any packets in the buffer that have a
	// failed log entry. We do it in reverse order so the index is not
	// affected by previous removals.
	for (int j = network.packetsWaiting(); j > 0; j--) {
		uint32_t id, command, parentId;
		if (network.getPacketHeader(j, id, command, parentId)) {
			if (logGetResult(id) == kFail) network.dropPacket(j);
		}
	}

	return validPackets;
}

QString
Parser::getXml(QByteArray &data, const char *attr)
{
	pugi::xml_document xml;
	xml.load_buffer(data.constData(), data.size());
	return xml.child("Blastpit").attribute(attr).value();
}

int
Parser::getXmlInt(QByteArray &data, const char *attr)
{
	return QString(getXml(data, attr)).toInt();
}

float
Parser::getXmlFloat(QByteArray &data, const char *attr)
{
	return QString(getXml(data, attr)).toFloat();
}

double
Parser::getXmlDouble(QByteArray &data, const char *attr)
{
	return QString(getXml(data, attr)).toDouble();
}

bool
Parser::getXmlBool(QByteArray &data, const char *attr)
{
	return QString(getXml(data, attr)).toInt();
}

void
Parser::parseCommand(int id, int command, QByteArray data)
{
	emit log("ID: " + QString::number(id) +
		 ", Command: " + QString::number(command) +
		 ", Size: " + QString::number(data.size()));

	switch (command) {
		case kInitMachine:
			lmos.InitMachine();
			break;
		case kTermMachine:
			lmos.TermMachine();
			break;
		case kClearQpSets:
			lmos.ClearQPSets();
			break;
		case kImportXML:
			lmos.LoadXML(getXml(data, "drawing"));
			break;
		case kStopPosHelp:
			lmos.StopPosHelp();
			break;
		case kStartMarking:
			logWrite(id, command, kRunning);
			lmos.StartMarking();
			break;
		case kStopMarking:
			network.flushPacketQueue();
			lmos.StopMarking();
			break;
		case kSelfTest:
			lmos.Test();
			break;
		case kStartPosHelp:
			lmos.StartPosHelp(getXml(data, "object"));
			break;
		case kExit:
			emit finished();
			break;
		case kReference:
			lmos.Reference();
			break;
		case kSetMOLayer:
			lmos.SetMOLayer(getXml(data, "object"),
					getXml(data, "layer"));
			break;
		case kLayerSetHeightZAxis:
			lmos.SetMOLayer(getXml(data, "layer"),
					getXml(data, "height"));
			break;
		case kLayerSetLaserable:
			lmos.LayerSetLaserable(getXml(data, "layer"),
					       getXmlBool(data, "laserable"));
			break;
		case kLayerSetVisible:
			lmos.LayerSetLaserable(getXml(data, "layer"),
					       getXmlBool(data, "visible"));
			break;
		case kLayerSetExportable:
			lmos.LayerSetLaserable(
				getXml(data, "layer"),
				getXmlBool(data, "exportable"));
			break;
		case kCancelJob:
			network.flushPacketQueue();
			lmos.CancelJob();
			break;
		case kSetDimension:
			lmos.SetDimension(getXml(data, "object"),
					  getXmlDouble(data, "x"),
					  getXmlDouble(data, "y"));
			break;
		case kSetPosValues:
			lmos.SetPosValues(getXml(data, "object"),
					  getXmlDouble(data, "x"),
					  getXmlDouble(data, "y"),
					  getXmlDouble(data, "r"));
			break;
		case kZoomWindow:
			lmos.ZoomWindow(getXmlDouble(data, "x1"),
					getXmlDouble(data, "y1"),
					getXmlDouble(data, "x2"),
					getXmlDouble(data, "y2"));
			break;
		case kAddQpSet:
			lmos.AddQPSet(getXml(data, "name"),
				      getXmlDouble(data, "current"),
				      getXmlDouble(data, "speed"),
				      getXmlDouble(data, "frequency"));
			break;
		case kWriteIOBit:
			lmos.WriteIOBit(getXml(data, "bitfunction"),
					getXmlBool(data, "value"));
			break;
		case kReadByte:
			lmos.ReadByte(getXmlInt(data, "port"),
				      getXmlInt(data, "mask"));
			break;
		case kReadIOBit:
			lmos.ReadIOBit(getXml(data, "bitfunction"));
			break;
		case kSaveVLM:
			lmos.SaveVLM(getXml(data, "filename"));
			break;
		case kLoadVLM:
			lmos.LoadVLM(getXml(data, "filename"));
			break;
		case kMoveZ:
			lmos.MoveZ(getXmlFloat(data, "height"));
			break;
		case kMoveW:
			lmos.MoveW(getXmlFloat(data, "rotation"));
			break;
		case kNoCommand: /* Dummy command for unit tests */
			logWrite(id, command, kSuccess);
			break;

		default:
			// We should not get here
			qDebug() << "Unknown command " << command
				 << " in parseCommand()";
			logWrite(id, command, kSuccess);
	}
}

void
Parser::test()
{
	asm("nop");
}

void
Parser::imageEnd2(double time, int result)
{  // Marking has completed

	log(QString(__func__) + ": Time " + QString::number(time));

	// Since there is no connection between this signal and the id of the
	// packet that began the marking, we must look through the log and
	// determine it.
	uint32_t id = getLastCommandOfType(kStartMarking);
	if (id) {
		// result is of type ImageResultConstants, for which
		// zero means success
		if (result == 0) {
			logEdit(id, kSuccess);
		} else {
			logEdit(id, kFail);
		}

		getPacket();
	} else {
		// Something went wrong, because there should be a log entry
		// that started this.
		qDebug() << __func__ << " error";
		throw "Log entry error";
	}
}

void
Parser::receiveEvent(QString event)
{  // Signal has been received from Lmos

	qDebug() << "myEvent: " << event;
}

void
Parser::logWrite(uint32_t id, uint32_t command, uint32_t result)
{  // Add an entry to the log

	struct BpLogEntry entry;
	entry.id = id;
	entry.command = command;
	entry.result = result;
	logfile.push_back(entry);
}

uint32_t
Parser::logGetResult(uint32_t id)
{  // Get result from log, if it exists

	// We iterate backwards because it is more likely
	// that what we want is near the end
	for (auto it = logfile.rbegin(); it != logfile.rend(); it++) {
		if ((*it).id == id) return (*it).result;
	}

	return kNoEntry;
}

bool
Parser::logEdit(uint32_t id, uint32_t result)
{ /* Edit a log entry result */

	for (auto it = logfile.rbegin(); it != logfile.rend(); it++) {
		if ((*it).id == id) {
			(*it).result = result;
			return true;
		}
	}

	return false;
}

void
Parser::listLog()
{  // Dump the log for viewing

	for (auto it = logfile.begin(); it != logfile.end(); it++) {
		qDebug() << "Parser log: id: " + QString::number((*it).id) +
				    ", command: " +
				    QString::number((*it).command) +
				    ", result: " +
				    QString::number((*it).result);
	}
}

void
Parser::clearLog()
{  // Clear the log (useful when testing)

	logfile.clear();
}

uint32_t
Parser::getLastCommandOfType(uint32_t command)
{  // Search the log for the last command of a type

	for (auto it = logfile.rbegin(); it != logfile.rend(); it++) {
		/* qDebug() << "getLastCommandOfType: id: " +
		 * QString::number((*it).id) + ", command: " +
		 * QString::number((*it).command) + */
		/* 		    ", result: " + */
		/* 		    QString::number((*it).result); */
		if ((*it).command == command) return (*it).id;
	}

	return false;
}
