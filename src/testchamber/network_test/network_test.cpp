#include <cstdlib>

#include "network_test.h"

NetworkTest::NetworkTest() { srand(time(nullptr)); }

void
NetworkTest::initTestCase()
{ /* Called before first test only */

	// Set the network listening
	qDebug() << "Activating listening server...";
	testNetwork.reset(new Network);
	testNetwork->listenTcp(LISTEN_PORT);
}

void
NetworkTest::cleanupTestCase()
{ /* Called after last test only */

	qDebug() << "Destroying listening server...";
	testNetwork.reset(nullptr);
}

void
NetworkTest::init()
{ /* Called before every test */

	testNetwork->flushPacketQueue();
}

void
NetworkTest::cleanup()
{ /* Called after every test */
}

void
NetworkTest::sendSingleCommandGetAck()
{ /* Tests Network class.
   * Tests Tcp listening server, packet processing and checksum tests,
   * sending and receiving small packets
   */

	bool found = false;
	int i;
	int id = rand();
	struct Network::ackPkt reply;
	reply.id = false;
	reply.reply = false;

	testNetwork->sendCommandOnly(id, kSelfTest, 0, "127.0.0.1",
				     LISTEN_PORT);

	// Replace this with QNetworkTest::qWaitFor([&](){return
	// testNetwork->popCommandReply(reply);}, 1000); after Qt 5.10
	for (i = 0; i < 4; i++) {
		found = testNetwork->popCommandReply(reply);
		if (found) break;
		QTest::qWait(50);
	}

	QVERIFY(found);
	QCOMPARE(reply.id, static_cast<uint32_t>(id));
	QVERIFY(reply.reply);
}

void
NetworkTest::randomAccessPackets()
{ /* Tests random access to packet queue.
   */

	uint32_t id = 999, command = 999, parent = 999;
	QByteArray packet;

	testNetwork->sendCommandOnly(1, kSelfTest, 0, "127.0.0.1",
				     LISTEN_PORT);
	QTest::qWait(50);
	testNetwork->sendCommandOnly(2, kSelfTest, 0, "127.0.0.1",
				     LISTEN_PORT);
	QTest::qWait(50);
	QVERIFY(testNetwork->packetsWaiting() == 2);

	QVERIFY(testNetwork->getPacketHeader(1, id, command, parent));
	QCOMPARE(id, static_cast<uint32_t>(2));
	QCOMPARE(command, static_cast<uint32_t>(kSelfTest));

	QVERIFY(testNetwork->getPacket(1, packet));
	QVERIFY(testNetwork->packetsWaiting() == 1);
	QVERIFY(testNetwork->getPacketHeader(0, id, command, parent));
	QCOMPARE(id, static_cast<uint32_t>(1));
}


QTEST_MAIN(NetworkTest)


/*! \class Network network.h "network.h"
 *  \brief Main network class for blastpit
 *  \details Contains UDP and TCP listening server code, client code, packet
 * queueing and checksum verification
 */

/*! \fn int Network::sendCommandOnly( uint32_t id, uint32_t command, QString
 * ip, int port ) \brief Send a single command packet. \param [in] id The
 * identification number of the packet \param [in] command The command code
 *  \param [in] ip The ip address of the server
 *  \param [in] port The server port number
 *  \return True if socket does not timeout
 *  \details Sends a single packet containing a command and id.
 * Acknowledgement and reply can be obtained later with popCommandReply().
 */

/*! \fn void Network::popCommandReply( struct cmdReply& reply)
 *  \brief Retrive first queued command reply.
 *  \param [out] reply Reference to storage for reply packet
 *  \return True if a valid packet is available, false if queue is empty
 *  \details Retrieves the earliest reply packet and removes it from the
 * queue.
 */

/*! \fn void Network::processPackets( void )
 *  \brief Verify packet checksum and send acknowledgement.
 *  \details Examines the queued network packets and attempts to verify the
 * CRC32 checksum. Good packets are pushed on to the queue for parsing. Bad
 * packets are discarded. A reply packet is sent containing the packet id and
 * a status reply.
 */
