#include <cstdlib>

#include "parser_test.h"

ParserTest::ParserTest() { srand(time(nullptr)); }

void
ParserTest::initTestCase()
{ /* Called before first test only */

	// Set the network listening
	qDebug() << "Activating listening server...";
	testParser.reset(new Parser(this, LISTEN_PORT));
	/* testParser->network.listenTcp(TEST_LISTEN_PORT); */
}

void
ParserTest::cleanupTestCase()
{ /* Called after last test only */

	qDebug() << "Destroying listening server...";
	testParser.reset(nullptr);
}

void
ParserTest::init()
{ /* Called before every test */

	testParser->network.flushPacketQueue();
	testParser->clearLog();
}

void
ParserTest::cleanup()
{ /* Called after every test */
}

void
ParserTest::parentTest()
{ /* Checks if commands are invalidated by failed parent */

	return;
	/* Setup packets (child, parent and orphan) */
	QByteArray li = "Lorem ipsum";
	QByteArray ba = testParser->network.buildPacket(2, kNoCommand, 1, li);
	testParser->network.injectPacket(ba);
	ba = testParser->network.buildPacket(3, kNoCommand, 2, li);
	testParser->network.injectPacket(ba);
	ba = testParser->network.buildPacket(4, kNoCommand, 0, li);
	testParser->network.injectPacket(ba);

	/* Fail grandparent */
	testParser->logWrite(1, kNoCommand, kFail);

	/* Attempt to retrieve packets for child and parent should fail, and
	 * new log entries failing them should have been written. The orphan
	 * packet should be unaffected and be logged successfully. */
	QCOMPARE(1,
		 testParser
			 ->getPacket()); /* Only one packet should be valid */
	QCOMPARE(static_cast<uint32_t>(kFail), testParser->logGetResult(2));
	QCOMPARE(static_cast<uint32_t>(kFail), testParser->logGetResult(3));
	QCOMPARE(static_cast<uint32_t>(kSuccess),
		 testParser->logGetResult(4));
}

void
ParserTest::queueTest()
{ /* Checks that marking success/failure activates or cancels child jobs and
     flushes buffers */

	return;
	/* Setup packets */
	QByteArray li = "Lorem ipsum";
	QByteArray ba = testParser->network.buildPacket(2, kNoCommand, 1, li);
	testParser->network.injectPacket(ba);
	QCOMPARE(0,
		 testParser->getPacket()); /* Packet is waiting on parent */
	ba = testParser->network.buildPacket(1, kNoCommand, 0, li);
	testParser->network.injectPacket(ba);

	QCOMPARE(testParser->getPacket(),
		 1); /* Only the parent packet should be parsed */
	QCOMPARE(testParser->network.packetsWaiting(),
		 1); /* Leaving the child packet */
	QCOMPARE(testParser->getPacket(),
		 1); /* Which should be parsed here */
	QCOMPARE(testParser->network.packetsWaiting(),
		 0); /* Leaving the queue empty */

	/* Put two packets with a dummy parent, and a cancel command, in queue
	 */
	ba = testParser->network.buildPacket(3, kNoCommand, 999, li);
	testParser->network.injectPacket(ba);
	ba = testParser->network.buildPacket(4, kNoCommand, 999, li);
	testParser->network.injectPacket(ba);
	ba = testParser->network.buildPacket(4, kCancelJob, 0, li);
	testParser->network.injectPacket(ba);
	QCOMPARE(testParser->getPacket(), 1); /* Only the cancel command
						 should be parsed */
	QCOMPARE(testParser->network.packetsWaiting(),
		 0); /* The queues should
		   have been flushed */
}

void
ParserTest::signalTest()
{ /* Checks that the imageEnd2 signal triggers the next packet */

	/* This test assumes that kStartMarking always emits imageEnd2 for
	 * this test, which it does on linux. */
	QByteArray li = "Lorem ipsum";
	QByteArray ba =
		testParser->network.buildPacket(1, kStartMarking, 0, li);
	testParser->network.injectPacket(ba);
	ba = testParser->network.buildPacket(2, kStartMarking, 1, li);
	testParser->network.injectPacket(ba);
	ba = testParser->network.buildPacket(3, kNoCommand, 2, li);
	testParser->network.injectPacket(ba);

	QCOMPARE(testParser->network.packetsWaiting(), 3);
	QCOMPARE(testParser->getPacket(),
		 1); /* Only the parent packet should be parsed... */
	QCOMPARE(testParser->network.packetsWaiting(),
		 0); /* ... but it should cause the child packets to finish,
			leaving the queue empty. */
}

QTEST_MAIN(ParserTest)

/* To skip a test: */
/* QSKIP("Test not written", QTest::SkipSingle); */
