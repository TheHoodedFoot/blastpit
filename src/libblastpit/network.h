#ifndef BPNET_H
#define BPNET_H

#include <QDataStream>
#include <QObject>
#include <QString>
#include <QTcpServer>
#include <QTcpSocket>
#include <QTemporaryFile>
#include <QUdpSocket>
#include <memory>
#include <queue>

#include "common.h"

#define MAX_PACKET_LENGTH 10 * 1024 * 1024
#define NET_BUF_SIZE 4096
#define SOCK_MAX_WAIT_MS 1000

class Network : public QObject {
	Q_OBJECT
      public:
	explicit Network(QObject* parent = 0);
	~Network();

      public:
	struct ackPkt {
		uint32_t reply;
		uint32_t id;
	};

	bool bumpPacket(int index);
	bool dropPacket(int index);
	bool popCommandReply(struct ackPkt&);
	bool getPacket(QByteArray&);
	bool getPacket(int, QByteArray&);
	bool getPacketHeader(int, uint32_t&, uint32_t&, uint32_t&);
	int packetsWaiting();
	bool sendCommandOnly(uint32_t, uint32_t, uint32_t, QString, int);
	static QString getLocalIP();
	uint32_t getId(int index);
	uint32_t getParent(int index);
	void flushPacketQueue();
	QByteArray buildPacket(uint32_t id, uint32_t command, uint32_t parent,
			       QByteArray& payload);
	void injectPacket(QByteArray& packet);

      signals:
	void error(QString err);
	void log(const QString entry);
	void log(int, const char*, const QString);
	void receivedPacket();

      public slots:
	void getCommandReply();
	void listenTcp(int);
	bool listenUdp(int);
	void connectSocketUdp();
	void connectSocketTcp();
	// void receiveqpsets( QString qpsets );
	void newPacket();
	void monitorPackets();

      private:
	int putBytes(const char* buffer, int size);
	int joinPackets();
	void processPackets();
	int verifyChecksums(QByteArray& packet);

      private:
	QTcpServer* tcpServer;
	QTcpSocket* tcpSocket;
	QUdpSocket* udpSocket;
	QTcpSocket* cmdSock;
	QDataStream ds;
	QVector<QByteArray> packetsGood;
	QByteArray packetsPartial;
	std::queue<ackPkt> cmdReplies;
};

enum netChecksums {
	kBadChecksums,
	kHeaderChecksum,
	kDataChecksum,
	kGoodChecksums,
};

#endif  // BPNET_H
