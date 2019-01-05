#ifndef BLASTPIT_TEST_H
#define BLASTPIT_TEST_H

#include <QObject>
#include <QtTest/QtTest>
#include <memory>

#include "blastpit.h"

#define TEST_LISTEN_PORT 9999


class BlastpitTest : public QObject {
	Q_OBJECT
      public:
	BlastpitTest();
      private slots:
	void initTestCase();
	void cleanupTestCase();
	void init();
	void cleanup();

	void listen();

      private:
};

#endif  // BLASTPIT_TEST_H
