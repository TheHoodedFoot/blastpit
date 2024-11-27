#include "unity_fixture.h" /* MUST be before <stdlib.h> */
#include <stdlib.h>

#include "blastpit.h"
#include "xml.h"

TEST_GROUP( XmlGroup );

TEST_SETUP( XmlGroup ) {}

TEST_TEAR_DOWN( XmlGroup ) {}

TEST( XmlGroup, AddRemoveIdTest )
{
	sds attr = XmlGetAttribute( "Hello, world!", "id" );
	TEST_ASSERT_NULL( attr );
	sdsfree( attr );

	attr = XmlGetAttribute( "<?xml?><message id=\"33\"/>", "id" );
	TEST_ASSERT_EQUAL_STRING( "33", attr );
	sdsfree( attr );

	sds message_removed = sdsnew( "<message id=\"33\"/>" );
	message_removed	    = XmlDeleteAttribute( message_removed, "id" );
	TEST_ASSERT_NOT_NULL( message_removed );
	TEST_ASSERT_EQUAL( NULL, XmlGetAttribute( message_removed, "id" ) );
	TEST_ASSERT_EQUAL_STRING( "<message />\n", message_removed );

	sds message_added = XmlSetAttribute( message_removed, "id", "123" );
	TEST_ASSERT_NOT_NULL( message_added );
	TEST_ASSERT_EQUAL_STRING( "<message id=\"123\" />\n", message_added );

	sds message_replaced = XmlSetAttribute( message_added, "id", "124" );
	TEST_ASSERT_NOT_NULL( message_replaced );
	TEST_ASSERT_EQUAL_STRING( "<message id=\"124\" />\n", message_replaced );

	/* fprintf(stderr, "message_removed: %p\n", message_removed); */
	sdsfree( message_replaced );
}

// TEST(XmlGroup, AddHeaderTest)
// {
// 	sds addHeader = xml_addHeader("foo");
// 	TEST_ASSERT_EQUAL_STRING("<?xml version=\"1.0\"?>\n<message>foo</message>\n", addHeader);
// 	sdsfree(addHeader);
// }

TEST( XmlGroup, CommandStringTest )
{
	sds message = XmlGetChildNodeAsString( "<message>Teststring</message>", "message" );
	TEST_ASSERT_EQUAL_STRING( "<message>Teststring</message>\n", message );

	// sds badmessage = XmlGetCdata("<feck>Teststring</feck>");
	// TEST_ASSERT_NULL(badmessage);

	sdsfree( message );
	// sdsfree(badmessage);
}

TEST( XmlGroup, DeleteAttributeTest )
{
	sds messageRemoved = XmlDeleteAttribute( "<message id=\"33\"/>", "id" );
	TEST_ASSERT_EQUAL_STRING( "<message />", messageRemoved );
	sdsfree( messageRemoved );
}

// TEST(XmlGroup, XmlRetvalTest)
// {
// 	// Replies are in the format:
// 	// <xml><command id="1">3</command>
// 	// or
// 	// <xml><command id="1">string</command>

// 	// Tests whether getValuesFromXml() can parse return values
// 	const char *xml = "<?xml?><message id=\"73\" retval=\"6\" />";
// 	IdAck reply = ParseXmlIdAndRetval(xml);
// 	TEST_ASSERT_EQUAL(73, reply.id);
// 	TEST_ASSERT_EQUAL(992, reply.retval);

// 	// Invalid or missing id should return kInvalid
// 	xml = "<?xml?><message>123</message>";
// 	reply = ParseXmlIdAndRetval(xml);
// 	TEST_ASSERT_EQUAL(kInvalid, reply.id);
// 	TEST_ASSERT_EQUAL(123, reply.retval);

// 	// Invalid retvals should return zero (kFailure)
// 	xml = "<?xml?><message id=\"39\"></message>";
// 	reply = ParseXmlIdAndRetval(xml);
// 	TEST_ASSERT_EQUAL(39, reply.id);
// 	TEST_ASSERT_EQUAL(kFailure, reply.retval);

// 	// Warn if no values found
// }

TEST( XmlGroup, MultipleMessageTest )
{
	// What are the requirements to test 'x'?
	// 	What does the object do?
	// 	How does it interact with the data or hardware it controls?
	// 	How can we make it testable?

	// Must load XML and detect error
	TEST_ASSERT_EQUAL( 0, XmlGetMessageCount( "Feck!" ) );

	// Must count number of <message> blocks
	const char* xml1 = "<?xml?><message id=\"1\" command=\"1\"></message>";
	TEST_ASSERT_EQUAL( 1, XmlGetMessageCount( xml1 ) );
	const char* xml2 = "<?xml?><message id=\"1\" command=\"1\"></message>"
			   "<message id=\"2\" command=\"2\"></message>";
	TEST_ASSERT_EQUAL( 2, XmlGetMessageCount( xml2 ) );

	// Must be able to retrieve specific messages
	const char* xml3 = "<?xml?><message id=\"1\" command=\"1\"></message>"
			   "<message id=\"2\" command=\"2\"></message>"
			   "<message id=\"3\" command=\"3\"></message>";
	TEST_ASSERT_EQUAL_STRING( "<?xml?><message id=\"1\" command=\"1\" />\n", XmlGetMessageByIndex( xml3, 0 ) );
	TEST_ASSERT_EQUAL_STRING( "<?xml?><message id=\"2\" command=\"2\" />\n", XmlGetMessageByIndex( xml3, 1 ) );
	TEST_ASSERT_EQUAL_STRING( "<?xml?><message id=\"3\" command=\"3\" />\n", XmlGetMessageByIndex( xml3, 2 ) );
	TEST_ASSERT_NULL( XmlGetMessageByIndex( xml3, 3 ) );
}

TEST( XmlGroup, AttributeTest )
{
	// What are the requirements to test 'x'?
	// 	What does the object do?
	// 	How does it interact with the data or hardware it controls?
	// 	How can we make it testable?

	// Get attribute value or NULL from node
	const char* xml = "<?xml?><message id=\"1\" command=\"1\" lorem=\"ipsum\"></message>";
	// sds attr1 = XmlGetAttribute((char *)xml, "lorem");
	// TEST_ASSERT_EQUAL_STRING("ipsum", attr1);
	sds attr2 = XmlGetAttribute( (char*)xml, "missing" );
	TEST_ASSERT_NULL( attr2 );
	sdsfree( attr2 );
	// sdsfree(attr1);
}

TEST_GROUP_RUNNER( XmlGroup )
{ /* Add a line below for each unit test */

	RUN_TEST_CASE( XmlGroup, AttributeTest );
	// RUN_TEST_CASE(XmlGroup, MultipleMessageTest);
	// RUN_TEST_CASE(XmlGroup, AddRemoveIdTest);
	// RUN_TEST_CASE(XmlGroup, CommandStringTest);

	// RUN_TEST_CASE(XmlGroup, AddHeaderTest);
	// RUN_TEST_CASE(XmlGroup, OverflowTest);
	// RUN_TEST_CASE(XmlGroup, XmlRetvalTest);
}

static void runAllTests( void )
{
	RUN_TEST_GROUP( XmlGroup );
}

int main( int argc, const char* argv[] )
{
	return UnityMain( argc, argv, runAllTests );
}
