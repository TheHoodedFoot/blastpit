#ifndef PARSER_TEST_H
#define PARSER_TEST_H

#include <QObject>
#include <QtTest/QtTest>
#include <memory>

#include "network.h"
#include "parser.h"


class ParserTest : public QObject {
	Q_OBJECT
      public:
	ParserTest();
      private slots:
	void initTestCase();
	void cleanupTestCase();
	void init();
	void cleanup();

	void parentTest();
	void queueTest();
	void signalTest();

      private:
	std::unique_ptr<Parser> testParser;
};

#endif  // PARSER_TEST_H
