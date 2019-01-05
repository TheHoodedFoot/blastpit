#include <stdint.h>
#include <QByteArray>
#include <QDebug>
#include <QHostInfo>
#include <QNetworkInterface>
#include <QTcpSocket>
#include <QThread>
#include <cstring>
#include <iostream>
#include <sstream>
#include <string>

#include "../../sub/CRCpp/inc/CRC.h"
#include "common.h"
/* #include "mosquitto.h" */
#include "network.h"

Network::Network(QObject *parent) : QObject(parent)
{
	tcpServer = nullptr;
	udpSocket = nullptr;
	cmdSock = nullptr;
}

Network::~Network()
{
	if (tcpServer != nullptr) delete tcpServer;
	if (udpSocket != nullptr) delete udpSocket;
}

void
Network::listenTcp(int port)
{
	if (tcpServer == nullptr) {
		tcpServer = new QTcpServer(this);
	}
	if (!tcpServer->listen(QHostAddress::Any, port)) {
		qDebug() << "TCP server listen failure";
		return;
	}
	connect(tcpServer, SIGNAL(newConnection()), this,
		SLOT(connectSocketTcp()));
}

bool
Network::listenUdp(int port)
{
	if (udpSocket == nullptr) {
		udpSocket = new QUdpSocket(this);
	}
	if (!udpSocket->bind(QHostAddress::Any, port)) {
		return false;
	}
	connect(udpSocket, SIGNAL(readyRead()), this,
		SLOT(connectSocketUdp()));
	return true;
}

void
Network::connectSocketTcp()
{
	// We want to automatically delete the socket
	// once it has been disconnected.
	tcpSocket = tcpServer->nextPendingConnection();
	connect(tcpSocket, SIGNAL(disconnected()), tcpSocket,
		SLOT(deleteLater()));

	// We have received a client connection,
	connect(tcpSocket, SIGNAL(readyRead()), this, SLOT(newPacket()));
	/* connect(tcpSocket, SIGNAL(readyRead()), this,
	 * SLOT(monitorPackets())); */
}

void
Network::monitorPackets()
{  // Dummy monitor used to examine received data
	if (tcpSocket == nullptr) {
		qDebug() << "tcpSocket is null!";
		return;
	}

	qInfo() << "*** Packet Monitor ***";
	qInfo() << "Bytes available to read: " << tcpSocket->bytesAvailable();

	QByteArray newPacket = tcpSocket->readAll();

	qInfo() << "Data received: " << newPacket;
}

void
Network::newPacket()
{
	/* qInfo() << "Bytes to read: " << tcpSocket->bytesAvailable(); */
	if (tcpSocket == nullptr) {
		qDebug() << "tcpSocket is null!";
		return;
	}
	QByteArray newPacket = tcpSocket->readAll();
	packetsPartial.append(newPacket);

	processPackets();

	if (packetsGood.size() > 0) {
		emit receivedPacket();
	}
}

QByteArray
Network::buildPacket(uint32_t id, uint32_t command, uint32_t parent,
		     QByteArray &payload)
{ /* Construct a network packet with checksums */

	union psHeader header;

	header.header.id = id;
	header.header.command = command;
	header.header.parentid = parent;
	header.header.size = payload.size();

	const char *data = payload.constData();
	header.header.dataChecksum =
		CRC::Calculate(data + sizeof(struct BpPacket), payload.size(),
			       CRC::CRC_32());
	header.header.packetChecksum = CRC::Calculate(
		header.bytes, BP_PACKET_CHECKSUM_SIZE, CRC::CRC_32());
	QByteArray packet =
		QByteArray(reinterpret_cast<const char *>(&header.bytes),
			   sizeof(struct BpPacket));
	packet.append(payload);

	return packet;
}

void
Network::injectPacket(QByteArray &packet)
{ /* Directly add a packet to the good packet queue, bypassing the network.
     This is mostly used for testing. */

	packetsGood.append(packet);
}

int
Network::verifyChecksums(QByteArray &packet)
{ /* Report the status of the checksums of a packet */

	int checksums = 0;

	// We can't access the QByteArray data directly
	const char *data = packet.constData();
	psHeader *header = (psHeader *)data;

	if (header->header.packetChecksum ==
	    CRC::Calculate(data, BP_PACKET_CHECKSUM_SIZE, CRC::CRC_32())) {
		checksums |= kHeaderChecksum;
	}

	if (header->header.dataChecksum ==
	    CRC::Calculate(data + sizeof(struct BpPacket),
			   packet.size() - sizeof(struct BpPacket),
			   CRC::CRC_32())) {
		checksums |= kDataChecksum;
	}

	return checksums;
}

void
Network::processPackets()
{
	for (;;) {
		if (packetsPartial.size() < (int)sizeof(BpPacket)) {
			return;
		}

		// We can't access the QByteArray data directly
		const char *data = packetsPartial.constData();
		psHeader *header = (psHeader *)data;

		// If the CRC fails, remove the first byte and hope to pick it
		// back up further down the line.
		if (header->header.packetChecksum !=
		    CRC::Calculate(data, BP_PACKET_CHECKSUM_SIZE,
				   CRC::CRC_32())) {
			packetsPartial.remove(1, 1);
			continue;
		}

		// Good so far, but have we received enough data yet?
		if (header->header.size <= (uint32_t)packetsPartial.size() -
						   sizeof(struct BpPacket)) {
			uint32_t crc = CRC::Calculate(
				data + sizeof(struct BpPacket),
				packetsPartial.size() -
					sizeof(struct BpPacket),
				CRC::CRC_32());
			if (header->header.dataChecksum == crc) {
				packetsGood.append(packetsPartial.left(
					header->header.size +
					sizeof(BpPacket)));
				struct ackPkt reply;
				reply.reply = kAck;
				reply.id = header->header.id;
				/* const char *ack = "ACK"; */
				putBytes(reinterpret_cast<const char *>(
						 &reply),
					 sizeof(struct ackPkt));
			} else {
				qInfo() << "Bad data checksum";
				// const char *ack = "NAK";
				struct ackPkt reply;
				reply.reply = kNack;
				reply.id = header->header.id;
				putBytes(reinterpret_cast<const char *>(
						 &reply),
					 sizeof(struct ackPkt));
				// putBytes(ack, sizeof("NAK"));
			}

			// Good or bad CRC, this data is now exhausted
			packetsPartial = packetsPartial.right(
				packetsPartial.size() -
				(header->header.size + sizeof(BpPacket)));
		} else {
			// Wait for more data to arrive
			return;
		}
	}
}

bool
Network::getPacket(QByteArray &packet)
{
	if (!packetsGood.empty()) {
		packet = packetsGood.takeFirst();
		return true;
	} else {
		return false;
	}
}

bool
Network::getPacket(int index, QByteArray &packet)
{
	if (packetsGood.size() >= index + 1) {
		packet = packetsGood[index];
		packetsGood.remove(index);
		return true;
	} else {
		return false;
	}
}

bool
Network::getPacketHeader(int index, uint32_t &id, uint32_t &command,
			 uint32_t &parentid)
{
	if (packetsGood.size() >= index + 1) {
		QByteArray packet = packetsGood[index];
		const char *pkt = packet.constData();
		struct BpPacket *header = (struct BpPacket *)pkt;
		id = header->id;
		command = header->command;
		parentid = header->parentid;
		return true;
	} else {
		return false;
	}
}

bool
Network::dropPacket(int index)
{  // Remove a packet from the queue

	if (packetsGood.size() >= index + 1) {
		packetsGood.remove(index);
		return true;
	}
	return false;
}

uint32_t
Network::getId(int index)
{  // Get the id of a packet

	uint32_t id, command, parentid;

	int result = getPacketHeader(index, id, command, parentid);
	if (result && id) return id;
	return false;
}

uint32_t
Network::getParent(int index)
{  // Get the id of a packet's parent command, if it has one

	uint32_t id, command, parentid;

	int result = getPacketHeader(index, id, command, parentid);
	if (result) return parentid;
	return false;
}

bool
Network::bumpPacket(int index)
{  // Pushes a packet further down the queue

	if (!packetsGood.empty() && packetsGood.size() >= index + 2) {
		packetsGood[index].swap(packetsGood[index + 1]);
		return true;
	}
	return false;
}


void
Network::flushPacketQueue()
{
	packetsPartial.clear();
	packetsGood.clear();
}

int
Network::packetsWaiting()
{
	return packetsGood.size();
}

int
Network::putBytes(const char *buffer, int numBytes)
{
	return tcpSocket->write(buffer, numBytes);
}

void
Network::connectSocketUdp()
{
	// Test with nc6: echo -n “\x7f\x00” | nc6 -4u -w1 -q1 localhost 1031
	while (udpSocket->hasPendingDatagrams()) {
		QByteArray datagram;
		datagram.resize(udpSocket->pendingDatagramSize());
		QHostAddress sender;
		quint16 senderPort;

		udpSocket->readDatagram(datagram.data(), datagram.size(),
					&sender, &senderPort);

		// Decode
		struct BpPacket pkt =
			*reinterpret_cast<const struct BpPacket *>(
				datagram.data());
		if (pkt.packetChecksum ==
		    CRC::Calculate(reinterpret_cast<const char *>(&pkt),
				   BP_PACKET_CHECKSUM_SIZE, CRC::CRC_32())) {
			/* QByteArray udppkt = reinterpret_cast<
			 * uint32_t[]>(datagram.data()); */
		}
	}
}

QString
Network::getLocalIP()
{
	foreach (const QHostAddress &address,
		 QNetworkInterface::allAddresses()) {
		if (address.protocol() == QAbstractSocket::IPv4Protocol &&
		    address.isInSubnet(
			    QHostAddress::parseSubnet("10.0.0.0/8"))) {
			return address.toString();
		}
	}
	return "127.0.0.1";
}

bool
Network::sendCommandOnly(uint32_t id, uint32_t command, uint32_t parent,
			 QString ip, int port)
{ /* Send a single command with no payload */

	(void)ip;

	union psHeader packet;
	packet.header.id = id;
	packet.header.command = command;
	packet.header.parentid = parent;
	packet.header.size = 0;
	packet.header.dataChecksum =
		CRC::Calculate(&packet.bytes, 0, CRC::CRC_32());
	packet.header.packetChecksum = CRC::Calculate(
		&packet.bytes, BP_PACKET_CHECKSUM_SIZE, CRC::CRC_32());

	// Open or reuse a connection to the server
	if (!cmdSock) cmdSock = new QTcpSocket();
	QHostAddress addr = QHostAddress(ip);
	cmdSock->connectToHost(addr, port, QIODevice::ReadWrite);
	if (cmdSock->waitForConnected(SOCK_MAX_WAIT_MS)) {
		cmdSock->write((const char *)packet.bytes,
			       sizeof(packet.bytes));
		connect(cmdSock, SIGNAL(readyRead()), this,
			SLOT(getCommandReply()));
		return true;
	}
	return false;
}

void
Network::getCommandReply()
{  // Receive the command return code
	if (cmdSock->bytesAvailable() != sizeof(struct ackPkt)) return;

	QByteArray reply = cmdSock->readAll();
	const char *data = reply.constData();
	cmdSock->close();
	cmdSock = nullptr;

	struct ackPkt rp;
	rp.reply = *reinterpret_cast<const uint32_t *>(data);
	rp.id = *reinterpret_cast<const uint32_t *>(data + sizeof(uint32_t));
	cmdReplies.push(rp);
}

bool
Network::popCommandReply(struct ackPkt &reply)
{ /* Returns the first queued reply
   */

	if (cmdReplies.empty()) return false;

	reply.id = cmdReplies.front().id;
	reply.reply = cmdReplies.front().reply;
	cmdReplies.pop();

	return true;
}


/* MqttSubscribe(topic, callback) */
/* MqttPublish(topic, payload) */
