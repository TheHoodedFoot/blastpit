#include "xml.h"
#include <mxml.h>
#include "blastpit.h"
#include "sds.h"

int
GetMessageId(const char *message)
{  // Returns message id, or kInvalid if nonexistent

	if (!message)
		return kInvalid;

	mxml_node_t *xml = mxmlLoadString(NULL, message, MXML_OPAQUE_CALLBACK);

	// Zero is considered an invalid id because atoi() returns 0
	// for invalid inputs.
	mxml_node_t *node = mxmlFindElement(xml, xml, "message", NULL, NULL, MXML_DESCEND);
	if (!node) {
		// fprintf(stderr, "Failed with message %s\n", message);
		return kInvalid;
	}

	const char *value = mxmlElementGetAttr(node, "id");

	int id = 0;
	if (value)
		id = atoi(value);
	return id ? id : kInvalid;
}

int
HasMultipleMessages(const char *xml)
{  // Returns count of <message> elements or kInvalid on bad xml

	mxml_node_t *tree = mxmlLoadString(NULL, xml, MXML_OPAQUE_CALLBACK);

	mxml_node_t *node;
	int count = 0;

	for (node = mxmlFindElement(tree, tree, "message", NULL,
				    NULL, MXML_DESCEND);
	     node != NULL;
	     node = mxmlFindElement(node, tree, "message", NULL,
				    NULL, MXML_DESCEND)) {
		count++;
	}

	return count;
}

sds
GetMessageByIndex(const char *xml, int index)
{  // Returns a string containing a single message element

	mxml_node_t *tree = mxmlLoadString(NULL, xml, MXML_OPAQUE_CALLBACK);

	mxml_node_t *node;
	int count = 0;

	for (node = mxmlFindElement(tree, tree, "message", NULL,
				    NULL, MXML_DESCEND);
	     node != NULL && count < index;
	     node = mxmlFindElement(node, tree, "message", NULL,
				    NULL, MXML_DESCEND)) {
		count++;
	}

	if (!node)
		return NULL;
	char *node_data = mxmlSaveAllocString(node, MXML_NO_CALLBACK);
	sds retval = sdsnew("<?xml?>");
	retval = sdscat(retval, node_data);
	free(node_data);

	return retval;
}

// int
// xml_hasHeader(const char* message)
// { /* Returns true if <command> header exists */

// 	pugi::xml_document xml;

// 	xml.load_string(message);
// 	return (xml.child("command") != nullptr);
// }

// /* The id modification functions below all produce dynamic copies of the input
//  * string, which will need to be freed by the caller */

// char*
// xml_getCommandString(const char* string)
// { /* Returns the pcdata contained by the command node */

// 	pugi::xml_document xml;

// 	if (!string)
// 		return nullptr;
// 	if (!strlen(string))
// 		return nullptr;
// 	xml.load_string(string);
// 	pugi::xml_node command = xml.child("command");
// 	if (!command)
// 		return nullptr;

// 	return xml_mallocCopy(command.child_value());
// }

// char*
// xml_mallocCopy(const char* string)
// { /* Creates a dynamic memory copy of a string */

// 	char* copiedString = (char*)calloc(1, strlen(string) + 1);
// 	if (!copiedString)
// 		return nullptr;

// 	strncpy(copiedString, string, strlen(string));
// 	return copiedString;
// }

// char*
// xml_setId(const int id, const char* message_const)
// { /* Adds or replaces id and returns (realloc)ed string */

// 	if (!xml_hasHeader(message_const)) {
// 		char* msgWithHeader = xml_addHeader(message_const);
// 		char* msgWithId = xml_addId(id, msgWithHeader);
// 		free(msgWithHeader);
// 		return msgWithId;
// 	}

// 	if (xml_getId(message_const)) {
// 		char* msgWithoutId = xml_removeId(message_const);
// 		char* msgWithId = xml_addId(id, msgWithoutId);
// 		free(msgWithoutId);
// 		return msgWithId;
// 	}

// 	char* withId = xml_addId(id, message_const);
// 	return withId;
// }

// char*
// xml_addHeader(const char* message)
// { /* Adds the <command> header to a string */

// 	(void)message;
// 	std::stringstream xmlstream;
// 	pugi::xml_document xml;

// 	pugi::xml_node command = xml.append_child("command");
// 	pugi::xml_node data = command.append_child(pugi::node_pcdata);
// 	data.set_value(message);
// 	xml.save(xmlstream);

// 	const std::string stdstring = xmlstream.str();
// 	const char* cstring = stdstring.c_str();
// 	return xml_mallocCopy(cstring);
// }

// char*
// xml_addId(const int id, const char* message)
// { /* Helper function for xml_setId */

// 	std::stringstream xmlstream;
// 	pugi::xml_document xml;

// 	xml.load_string(message);
// 	pugi::xml_node command = xml.child("command");
// 	if (!command)
// 		return nullptr;
// 	command.append_attribute("id");
// 	command.attribute("id") = id;
// 	xml.save(xmlstream);

// 	const std::string stdstring = xmlstream.str();
// 	const char* cstring = stdstring.c_str();
// 	return xml_mallocCopy(cstring);
// }

// char*
// xml_removeId(const char* message)
// { /* Return an allocated string with the id removed */

// 	std::stringstream xmlstream;
// 	pugi::xml_document xml;

// 	xml.load_string(message);
// 	xml.child("command").remove_attribute("id");
// 	xml.save(xmlstream);

// 	const std::string stdstring = xmlstream.str();
// 	const char* cstring = stdstring.c_str();
// 	return xml_mallocCopy(cstring);
// }

// XmlReply
// ParseXmlIdAndRetval(const char* xml)
// {  // Extract the id and return value into a struct

// 	XmlReply reply = {kInvalid, kInvalid};

// 	if (!xml_hasHeader(xml))
// 		return reply;

// 	reply.id = xml_getId(xml);
// 	char* xmldata = xml_getCommandString(xml);
// 	reply.retval = atoi(xmldata);
// 	free(xmldata);

// 	return reply;
// }
