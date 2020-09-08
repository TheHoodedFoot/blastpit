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
	if (!xml) return kInvalid;

	// Zero is considered an invalid id because atoi() returns 0
	// for invalid inputs.
	mxml_node_t *node = mxmlFindElement(xml, xml, "message", NULL, NULL, MXML_DESCEND);
	if (!node) {
		// fprintf(stderr, "Failed with message %s\n", message);
		mxmlDelete(xml);
		return kInvalid;
	}

	const char *value = mxmlElementGetAttr(node, "id");

	int id = 0;
	if (value)
		id = atoi(value);
	mxmlDelete(xml);
	return id ? id : kInvalid;
}

int
GetParentId(const char *message)
{  // Returns parent id, or kInvalid if nonexistent

	if (!message)
		return kInvalid;

	mxml_node_t *xml = mxmlLoadString(NULL, message, MXML_OPAQUE_CALLBACK);
	if (!xml) return kInvalid;

	// Zero is considered an invalid id because atoi() returns 0
	// for invalid inputs.
	mxml_node_t *node = mxmlFindElement(xml, xml, "message", NULL, NULL, MXML_DESCEND);
	if (!node) {
		// fprintf(stderr, "Failed with message %s\n", message);
		mxmlDelete(xml);
		return kInvalid;
	}

	const char *value = mxmlElementGetAttr(node, "parentid");

	int id = 0;
	if (value)
		id = atoi(value);
	mxmlDelete(xml);
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

	mxmlDelete(tree);
	return count;
}

sds
GetMessageByIndex(const char *xml, int index)
{  // Returns a string containing a single message element

	mxml_node_t *tree = mxmlLoadString(NULL, xml, MXML_OPAQUE_CALLBACK);

	mxml_node_t *node;
	int count = 0;

	// TODO: Memory leaks here
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
	mxmlDelete(tree);

	return retval;
}

sds
GetMessageAttribute(const char *message, const char *attribute)
{  // Retrieves an attribute value, or NULL

	if (!message || !attribute)
		return NULL;

	mxml_node_t *xml = mxmlLoadString(NULL, message, MXML_OPAQUE_CALLBACK);

	mxml_node_t *node = mxmlFindElement(xml, xml, "message", NULL, NULL, MXML_DESCEND);
	if (!node) {
		mxmlDelete(xml);
		return NULL;
	}

	const char *value = mxmlElementGetAttr(node, attribute);
	if (!value)
		return NULL;

	sds retval = sdsnew(value);

	mxmlDelete(xml);

	return retval;
}

int
xml_hasHeader(const char* message)
{ // Returns true if <command> header exists

	if (!message) return kInvalid;

	mxml_node_t *xml = mxmlLoadString(NULL, message, MXML_OPAQUE_CALLBACK);
	if (!xml) return false;

	mxml_node_t *node = mxmlFindElement(xml, xml, "message", NULL, NULL, MXML_DESCEND);
	if (node) {
		mxmlDelete(xml);
		return true;
	}

	mxmlDelete(xml);
	return false;
}

IdAck
ParseXmlIdAndRetval(const char* xml)
{  // Extract the id and return value into a struct

	IdAck reply = {kInvalid, kFailure, NULL};

	if (!xml_hasHeader(xml))
		return reply;

	reply.id = GetMessageId(xml);
	sds xmldata = GetCdata(xml);
	reply.retval = atoi(xmldata);
	sdsfree(xmldata);

	return reply;
}

sds
GetCdata(const char* string)
{ // Returns the pcdata contained by the command node

	if (!string)
		return NULL;
	if (!strlen(string))
		return NULL;

	mxml_node_t *xml = mxmlLoadString(NULL, string, MXML_OPAQUE_CALLBACK);
	if (!xml) return false;

	mxml_node_t *node = mxmlFindElement(xml, xml, "message", NULL, NULL, MXML_DESCEND);
	if (node) {
		sds cdata = sdsnew(mxmlGetCDATA(node));
		mxmlDelete(xml);
		return cdata;
	}

	return NULL;
}

sds
xml_removeId(sds message)
{ // Return a string with the id removed

	if (!message) return NULL;

	mxml_node_t *xml = mxmlLoadString(NULL, message, MXML_OPAQUE_CALLBACK);
	if (!xml) return false;

	mxml_node_t *node = mxmlFindElement(xml, xml, "message", NULL, NULL, MXML_DESCEND);
	if (!node) {
		mxmlDelete(xml);
		return NULL;
	}

	mxmlElementDeleteAttr(node, "id");

	char *node_data = mxmlSaveAllocString(xml, MXML_NO_CALLBACK);
	sds retval = sdscat(sdsempty(), node_data);
	free(node_data);
	mxmlDelete(xml);

	return retval;
}
