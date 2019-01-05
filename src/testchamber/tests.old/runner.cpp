#include "runner.h"

#include <unistd.h>
#include <memory>
#include "common.h"
#include "network.h"
#include "parser.h"

// Place these after main includes to prevent compile error
#include "CppUTest/CommandLineTestRunner.h"
#include "CppUTest/TestHarness.h"

#define TEST_PORT 1030


/* Globals */

std::unique_ptr<Parser> server(nullptr);
std::unique_ptr<Network> client(nullptr);

/* QThread workerThread; */

Runner::Runner(QObject *parent, int argc, char **argv) : QObject(parent)
{
	this->argc = argc;
	this->argv = argv;
}

/* Runner::~Runner() { */
/*         workerThread.quit(); */
/*         workerThread.wait(); */
/*     } */

// clang-format off
TEST_GROUP(FirstTestGroup){
	void
	setup()
	{
		client.reset(new Network);

		/* server.reset(new Parser(nullptr, TEST_PORT)); */
		/* server->moveToThread(&workerThread); */

		/* QObject::connect(&workerThread, &QThread::finished, server, &QObject::deleteLater); */
        /* connect(this, &Controller::operate, worker, &Worker::doWork); */
        /* connect(worker, &Worker::resultReady, this, &Controller::handleResults); */

		/* workerThread.start(); */

	}

	void
	teardown()
	{
		client.reset(nullptr);
		/* server.reset(nullptr); */
		/* workerThread.quit(); */
	}
};
// clang-format on

/* TEST(FirstTestGroup, FirstTest) { FAIL("Fail me!"); } */
/* TEST(FirstTestGroup, SecondTest) { STRCMP_EQUAL("hello", "world"); } */

TEST(FirstTestGroup, SingleCommand)
{  // Send a single command
	client->sendCommandOnly(999, kSelfTest, "127.0.0.1", TEST_PORT);
	struct Network::ackPkt reply;
	reply.id = 0;
	reply.reply = 0;
	bool found = false;
	int i;
	for (i = 0; i < 100; i++) {
		found = client->popCommandReply(reply);
		if (found) break;
		printf(".");
		usleep(100000);
	}
	CHECK(client->popCommandReply(reply));
	CHECK_EQUAL(999, reply.id);
	CHECK_EQUAL(999, reply.reply);
}

void
Runner::runTests()
{
	MemoryLeakWarningPlugin::turnOffNewDeleteOverloads();
	CommandLineTestRunner::RunAllTests(argc, argv);
	emit(testsFinished());
}

// clang-format off
/*
    CHECK(boolean condition) - checks any boolean result.
    CHECK_TEXT(boolean condition, text) - checks any boolean result and prints text on failure.
    CHECK_FALSE(condition) - checks any boolean result
    CHECK_EQUAL(expected, actual) - checks for equality between entities using ==. So if you have a class that supports operator==() you can use this macro to compare two instances. You will also need to add a StringFrom() function like those found in SimpleString. This is for printing the objects when the check failed.
    CHECK_COMPARE(first, relop, second) - checks thats a relational operator holds between two entities. On failure, prints what both operands evaluate to.
    CHECK_THROWS(expected_exception, expression) - checks if expression throws expected_exception (e.g. std::exception). CHECK_THROWS is only available if CppUTest is built with the Standard C++ Library (default).
    STRCMP_EQUAL(expected, actual) - checks const char* strings for equality using strcmp().
    STRNCMP_EQUAL(expected, actual, length) - checks const char* strings for equality using strncmp().
    STRCMP_NOCASE_EQUAL(expected, actual) - checks const char* strings for equality, not considering case.
    STRCMP_CONTAINS(expected, actual) - checks whether const char* actual contains const char* expected.
    LONGS_EQUAL(expected, actual) - compares two numbers.
    UNSIGNED_LONGS_EQUAL(expected, actual) - compares two positive numbers.
    BYTES_EQUAL(expected, actual) - compares two numbers, eight bits wide.
    POINTERS_EQUAL(expected, actual) - compares two pointers.
    DOUBLES_EQUAL(expected, actual, tolerance) - compares two floating point numbers within some tolerance
    FUNCTIONPOINTERS_EQUAL(expected, actual) - compares two void (*)() function pointers
    MEMCMP_EQUAL(expected, actual, size) - compares two areas of memory
    BITS_EQUAL(expected, actual, mask) - compares expected to actual bit by bit, applying mask
    FAIL(text) - always fails

NOTE Most macros have _TEXT() equivalents like CHECK_TEXT(), which are not explicitly listed here.

*/
