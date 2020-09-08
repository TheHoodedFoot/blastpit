#include <stdlib.h>
#include "unity_fixture.h" /* MUST be before <stdlib.h> */

#include "blastpit.h"
#include "xml.h"

TEST_GROUP(XmlGroup);

TEST_SETUP(XmlGroup) {}

TEST_TEAR_DOWN(XmlGroup) {}

TEST(XmlGroup, AddRemoveIdTest)
{
	TEST_ASSERT_EQUAL(kInvalid, GetMessageId("Hello, world!"));

	TEST_ASSERT_EQUAL(33, GetMessageId("<?xml?><message id=\"33\"/>"));

	sds messageRemoved = xml_removeId("<message id=\"33\"/>");
	/* fprintf(stderr, "messageRemoved: %p\n", messageRemoved); */
	TEST_ASSERT_NOT_NULL(messageRemoved);
	TEST_ASSERT_EQUAL(kInvalid, GetMessageId(messageRemoved));
	TEST_ASSERT_EQUAL_STRING("<?xml version=\"1.0\"?>\n<message />\n", messageRemoved);

	sds messageAdded = xml_setId(123, messageRemoved);
	TEST_ASSERT_NOT_NULL(messageAdded);
	TEST_ASSERT_EQUAL_STRING("<?xml version=\"1.0\"?>\n<message id=\"123\" />\n", messageAdded);

	sds messageReplaced = xml_setId(124, messageAdded);
	TEST_ASSERT_NOT_NULL(messageReplaced);
	TEST_ASSERT_EQUAL_STRING("<?xml version=\"1.0\"?>\n<message id=\"124\" />\n", messageReplaced);

	/* fprintf(stderr, "messageRemoved: %p\n", messageRemoved); */
	sdsfree(messageRemoved);
	sdsfree(messageAdded);
	sdsfree(messageReplaced);
}

TEST(XmlGroup, AddHeaderTest)
{
	sds addHeader = xml_addHeader("foo");
	TEST_ASSERT_EQUAL_STRING("<?xml version=\"1.0\"?>\n<message>foo</message>\n", addHeader);
	sdsfree(addHeader);
}

TEST(XmlGroup, CommandStringTest)
{
	sds message = xml_getCommandString("<message>Teststring</message>");
	TEST_ASSERT_EQUAL_STRING("Teststring", message);

	sds badmessage = xml_getCommandString("<feck>Teststring</feck>");
	TEST_ASSERT_NULL(badmessage);

	sdsfree(message);
	sdsfree(badmessage);
}

TEST(XmlGroup, OverflowTest)
{
	sds messageRemoved = xml_removeId("<message id=\"33\"/>");
	sdsfree(messageRemoved);
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
	const char *xml = "<?xml?><message id=\"73\">992</message>";
	IdAck reply = ParseXmlIdAndRetval(xml);
	TEST_ASSERT_EQUAL(73, reply.id);
	TEST_ASSERT_EQUAL(992, reply.retval);

	// Invalid or missing id should return kInvalid
	xml = "<?xml?><message>123</message>";
	reply = ParseXmlIdAndRetval(xml);
	TEST_ASSERT_EQUAL(kInvalid, reply.id);
	TEST_ASSERT_EQUAL(123, reply.retval);

	// Invalid retvals should return zero (kFailure)
	xml = "<?xml?><message id=\"39\"></message>";
	reply = ParseXmlIdAndRetval(xml);
	TEST_ASSERT_EQUAL(39, reply.id);
	TEST_ASSERT_EQUAL(kFailure, reply.retval);

	// Warn if no values found
}

TEST(XmlGroup, MultipleMessageTest)
{
	// What are the requirements to test 'x'?
	// 	What does the object do?
	// 	How does it interact with the data or hardware it controls?
	// 	How can we make it testable?

	// Must load XML and detect error
	TEST_ASSERT_EQUAL(0, HasMultipleMessages("Feck!"));

	// Must count number of <message> blocks
	const char *xml1 =
		"<?xml?><message id=\"1\" command=\"1\"></message>";
	TEST_ASSERT_EQUAL(1, HasMultipleMessages(xml1));
	const char *xml2 =
		"<?xml?><message id=\"1\" command=\"1\"></message>"
		"<message id=\"2\" command=\"2\"></message>";
	TEST_ASSERT_EQUAL(2, HasMultipleMessages(xml2));

	// Must be able to retrieve specific messages
	const char *xml3 =
		"<?xml?><message id=\"1\" command=\"1\"></message>"
		"<message id=\"2\" command=\"2\"></message>"
		"<message id=\"3\" command=\"3\"></message>";
	TEST_ASSERT_EQUAL_STRING("<?xml?><message id=\"1\" command=\"1\" />\n", GetMessageByIndex(xml3, 0));
	TEST_ASSERT_EQUAL_STRING("<?xml?><message id=\"2\" command=\"2\" />\n", GetMessageByIndex(xml3, 1));
	TEST_ASSERT_EQUAL_STRING("<?xml?><message id=\"3\" command=\"3\" />\n", GetMessageByIndex(xml3, 2));
	TEST_ASSERT_NULL(GetMessageByIndex(xml3, 3));
}

TEST(XmlGroup, AttributeTest)
{
	// What are the requirements to test 'x'?
	// 	What does the object do?
	// 	How does it interact with the data or hardware it controls?
	// 	How can we make it testable?

	// Get attribute value or NULL from node
	const char *xml =
		"<?xml?><message id=\"1\" command=\"1\" lorem=\"ipsum\"></message>";
	TEST_ASSERT_EQUAL_STRING("ipsum", GetMessageAttribute(xml, "lorem"));
	TEST_ASSERT_NULL(GetMessageAttribute(xml, "missing"));
}

TEST_GROUP_RUNNER(XmlGroup)
{ /* Add a line below for each unit test */

	RUN_TEST_CASE(XmlGroup, AttributeTest);
	RUN_TEST_CASE(XmlGroup, MultipleMessageTest);
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
