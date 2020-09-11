#include "xml.h"
#include <mxml.h>
#include "blastpit.h"
#include "sds.h"

// int
// GetMessageId(const char *message)
// {  // Returns message id, or kInvalid if nonexistent

// 	if (!message)
// 		return kInvalid;

// 	mxml_node_t *xml = mxmlLoadString(NULL, message, MXML_OPAQUE_CALLBACK);
// 	if (!xml) return kInvalid;

// 	// Zero is considered an invalid id because atoi() returns 0
// 	// for invalid inputs.
// 	mxml_node_t *node = mxmlFindElement(xml, xml, "message", NULL, NULL, MXML_DESCEND);
// 	if (!node) {
// 		// fprintf(stderr, "Failed with message %s\n", message);
// 		mxmlDelete(xml);
// 		return kInvalid;
// 	}

// 	const char *value = mxmlElementGetAttr(node, "id");

// 	int id = 0;
// 	if (value)
// 		id = atoi(value);
// 	mxmlDelete(xml);
// 	return id ? id : kInvalid;
// }

// int
// GetParentId(const char *message)
// {  // Returns parent id, or kInvalid if nonexistent

// 	if (!message)
// 		return kInvalid;

// 	mxml_node_t *xml = mxmlLoadString(NULL, message, MXML_OPAQUE_CALLBACK);
// 	if (!xml) return kInvalid;

// 	// Zero is considered an invalid id because atoi() returns 0
// 	// for invalid inputs.
// 	mxml_node_t *node = mxmlFindElement(xml, xml, "message", NULL, NULL, MXML_DESCEND);
// 	if (!node) {
// 		// fprintf(stderr, "Failed with message %s\n", message);
// 		mxmlDelete(xml);
// 		return kInvalid;
// 	}

// 	const char *value = mxmlElementGetAttr(node, "parentid");

// 	int id = 0;
// 	if (value)
// 		id = atoi(value);
// 	mxmlDelete(xml);
// 	return id ? id : kInvalid;
// }

// int
// xml_hasHeader(const char* message)
// { // Returns true if <command> header exists

// 	if (!message) return kInvalid;

// 	mxml_node_t *xml = mxmlLoadString(NULL, message, MXML_OPAQUE_CALLBACK);
// 	if (!xml) return false;

// 	mxml_node_t *node = mxmlFindElement(xml, xml, "message", NULL, NULL, MXML_DESCEND);
// 	if (node) {
// 		mxmlDelete(xml);
// 		return true;
// 	}

// 	mxmlDelete(xml);
// 	return false;
// }

// IdAck
// ParseXmlIdAndRetval(const char* xml)
// {  // Extract the id and return value into a struct

// 	IdAck reply = {kInvalid, kFailure, NULL};

// 	if (!xml_hasHeader(xml))
// 		return reply;

// 	reply.id = GetMessageId(xml);
// 	sds xmldata = GetCdata(xml);
// 	reply.retval = atoi(xmldata);
// 	sdsfree(xmldata);

// 	return reply;
// }

int
XmlGetMessageCount(const char *xml)
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
XmlGetAttribute(const char *message, const char *attribute)
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

sds
XmlGetCdata(const char *string)
{  // Returns the pcdata contained by the command node

	if (!string)
		return NULL;
	if (!strlen(string))
		return NULL;

	sds message = sdsnew(string);
	XmlAddXmlHeader(&message);
	printf("looing fior message in: %s\n", message);
	mxml_node_t *xml = mxmlLoadString(NULL, message, MXML_OPAQUE_CALLBACK);
	if (!xml) {
		sdsfree(message);
		return NULL;
	}

	mxml_node_t *node = mxmlFindElement(xml, xml, "message", NULL, NULL, MXML_DESCEND);
	if (node) {
		node = mxmlFindElement(node, node, NULL, NULL, NULL, MXML_DESCEND);
		printf("element name: %s\n", mxmlGetElement(node));
		printf("cdata from node: %s\n", mxmlGetCDATA(node));
		sds cdata = sdsnew(mxmlGetCDATA(node));
		mxmlDelete(xml);
		return cdata;
	}

	LOG(kLvlDebug, "%s: Couldn't find the message element\n", __func__);
	sdsfree(message);
	return NULL;
}

sds
XmlGetMessageByIndex(const char *xml, int index)
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
XmlSetAttribute(sds message, const char *attribute, const char *value)
{  // Sets an attribute value. Invalidates original message.

	if (!message || !attribute || !value)
		return NULL;

	XmlAddXmlHeader(&message);
	mxml_node_t *xml = mxmlLoadString(NULL, message, MXML_OPAQUE_CALLBACK);

	mxml_node_t *node = mxmlFindElement(xml, xml, "message", NULL, NULL, MXML_DESCEND);
	if (!node) {
		mxmlDelete(xml);
		return NULL;
	}

	mxmlElementSetAttr(node, attribute, value);

	char *node_data = mxmlSaveAllocString(node, MXML_NO_CALLBACK);
	sds retval = sdsnew(node_data);
	free(node_data);
	mxmlDelete(xml);
	sdsfree(message);  // Caller must treat existing message as deleted

	return retval;
}

void
XmlAddXmlHeader(sds *message)
{  // Helper to add the <?xml?> header to allow mxmlLoadString to work

	sds header_added = sdsnew("<?xml?>");
	header_added = sdscatsds(header_added, *message);
	sdsfree(*message);
	printf("message should now be %s\n", header_added);
	*message = header_added;
}

sds
XmlDeleteAttribute(sds message, const char *attribute)
{  // Deletes an attribute value. Invalidates original message.

	if (!message || !attribute) {
		printf("neight mess");
		return NULL;
	}

	XmlAddXmlHeader(&message);
	mxml_node_t *xml = mxmlLoadString(NULL, message, MXML_OPAQUE_CALLBACK);

	mxml_node_t *node = mxmlFindElement(xml, xml, "message", NULL, NULL, MXML_DESCEND);
	if (!node) {
		mxmlDelete(xml);
		return NULL;
	}

	mxmlElementDeleteAttr(node, attribute);

	char *node_data = mxmlSaveAllocString(node, MXML_NO_CALLBACK);
	sds retval = sdsnew(node_data);
	free(node_data);
	mxmlDelete(xml);
	sdsfree(message);  // Caller must treat existing message as deleted

	return retval;
}

sds
XmlNewCdata(const char *message)
{  // Adds a CDATA string to a <message>

	if (!message)
		return NULL;

	mxml_node_t *xml = mxmlNewXML("1.0");
	mxml_node_t *message_node = mxmlNewElement(xml, "message");
	mxmlNewCDATA(message_node, message);

	// We don't want the xml header, just the message node
	char *xml_string = mxmlSaveAllocString(message_node, MXML_NO_CALLBACK);

	sds retval = sdsnew(xml_string);
	free(xml_string);
	mxmlDelete(xml);

	return retval;
}

// sds
// xml_setId(sds message, const int id)
// { // Add or change the id on a message
//   // Returns null on error, or modified message
//   // Deletes original message upon success

// 	if (!message) return NULL;

// 	mxml_node_t *xml = mxmlLoadString(NULL, message, MXML_OPAQUE_CALLBACK);
// 	if (!xml) return false;

// 	mxml_node_t *node = mxmlFindElement(xml, xml, "message", NULL, NULL, MXML_DESCEND);
// 	if (!node) {
// 		mxmlDelete(xml);
// 		return NULL;
// 	}

// 	sds id_str = sdsfromlonglong(id);
// 	mxmlElementSetAttr(node, "id", id_str);

// 	char *node_data = mxmlSaveAllocString(node, MXML_NO_CALLBACK);
// 	sds retval = sdsnew(node_data);
// 	free(node_data);
// 	mxmlDelete(xml);
// 	sdsfree(message); // Caller must treat existing message as deleted

// 	return retval;
// }
