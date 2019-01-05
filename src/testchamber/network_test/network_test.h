#ifndef NETWORK_TEST_H
#define NETWORK_TEST_H

#include <QObject>
#include <QtTest/QtTest>
#include <memory>

#include "network.h"


#define LISTEN_PORT 1030


class NetworkTest : public QObject {
	Q_OBJECT
      public:
	NetworkTest();
      private slots:
	void initTestCase();
	void cleanupTestCase();
	void init();
	void cleanup();

	void sendSingleCommandGetAck();
	void randomAccessPackets();

      private:
	std::unique_ptr<Network> testNetwork;
};

#endif  // NETWORK_TEST_H
