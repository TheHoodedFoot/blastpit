#include <stdlib.h>
#include <string.h>
#include "unity_fixture.h" /* MUST be before <stdlib.h> */

#include "xml.hpp"

TEST_GROUP(XmlGroup);

TEST_SETUP(XmlGroup) {}

TEST_TEAR_DOWN(XmlGroup) {}

/***********************
 *  Write a test list  *
 ***********************/

/* What is the set of tests which, when passes, demonstrates that the code
 * works correctly? */

/* When writing a test, imagine the perfect interface for that function */

/* What function do we wish existed? */

TEST(XmlGroup, AddRemoveIdTest)
{
	TEST_ASSERT_EQUAL(kInvalid, xml_getId("Hello, world!"));

	TEST_ASSERT_EQUAL(33, xml_getId("<command id=\"33\"/>"));

	char *messageRemoved = xml_removeId("<command id=\"33\"/>");
	/* fprintf(stderr, "messageRemoved: %p\n", messageRemoved); */
	TEST_ASSERT_NOT_NULL(messageRemoved);
	TEST_ASSERT_EQUAL(kInvalid, xml_getId(messageRemoved));
	TEST_ASSERT_EQUAL_STRING("<?xml version=\"1.0\"?>\n<command />\n", messageRemoved);

	char *messageAdded = xml_setId(123, messageRemoved);
	TEST_ASSERT_NOT_NULL(messageAdded);
	TEST_ASSERT_EQUAL_STRING("<?xml version=\"1.0\"?>\n<command id=\"123\" />\n", messageAdded);

	char *messageReplaced = xml_setId(124, messageAdded);
	TEST_ASSERT_NOT_NULL(messageReplaced);
	TEST_ASSERT_EQUAL_STRING("<?xml version=\"1.0\"?>\n<command id=\"124\" />\n", messageReplaced);

	/* fprintf(stderr, "messageRemoved: %p\n", messageRemoved); */
	free(messageRemoved);
	free(messageAdded);
	free(messageReplaced);
}

TEST(XmlGroup, AddHeaderTest)
{
	char *addHeader = xml_addHeader("foo");
	TEST_ASSERT_EQUAL_STRING("<?xml version=\"1.0\"?>\n<command>foo</command>\n", addHeader);
	free(addHeader);
}

TEST(XmlGroup, CommandStringTest)
{
	char *message = xml_getCommandString("<command>Teststring</command>");
	TEST_ASSERT_EQUAL_STRING("Teststring", message);

	char *badmessage = xml_getCommandString("<feck>Teststring</feck>");
	TEST_ASSERT_NULL(badmessage);

	free(message);
	free(badmessage);
}

TEST(XmlGroup, OverflowTest)
{
	char *messageRemoved = xml_removeId("<command id=\"33\"/>");
	free(messageRemoved);
}

TEST(XmlGroup, XmlRetvalTest)
{
	// What are the requirements to test 'x'?
	// 	What does the object do?
	// 	How does it interact with the data or hardware it controls?
	// 	How can we make it testable?

	// Replies are in the format:
	// <xml><command id="1">3</command>
	// or
	// <xml><command id="1">string</command>

	// Tests whether getValuesFromXml() can parse return values
	const char *xml = "<command id=\"73\">992</Command>";
	XmlReply reply = ParseXmlIdAndRetval(xml);
	TEST_ASSERT_EQUAL(73, reply.id);
	TEST_ASSERT_EQUAL(992, reply.retval);

	// Invalid or missing id should return kInvalid
	xml = "<command>123</Command>";
	reply = ParseXmlIdAndRetval(xml);
	TEST_ASSERT_EQUAL(kInvalid, reply.id);
	TEST_ASSERT_EQUAL(123, reply.retval);

	// Invalid retvals should return zero (kFailure)
	xml = "<command id=\"39\"></Command>";
	reply = ParseXmlIdAndRetval(xml);
	TEST_ASSERT_EQUAL(39, reply.id);
	TEST_ASSERT_EQUAL(kFailure, reply.retval);

	// Warn if no values found
}

TEST_GROUP_RUNNER(XmlGroup)
{ /* Add a line below for each unit test */

	RUN_TEST_CASE(XmlGroup, AddRemoveIdTest);
	RUN_TEST_CASE(XmlGroup, AddHeaderTest);
	RUN_TEST_CASE(XmlGroup, CommandStringTest);
	RUN_TEST_CASE(XmlGroup, OverflowTest);
	RUN_TEST_CASE(XmlGroup, XmlRetvalTest);
}

static void
runAllTests()
{
	RUN_TEST_GROUP(XmlGroup);
}

int
main(int argc, const char *argv[])
{
	return UnityMain(argc, argv, runAllTests);
}
