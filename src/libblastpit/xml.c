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
GetParentId(const char *message)
{  // Returns parent id, or kInvalid if nonexistent

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

	const char *value = mxmlElementGetAttr(node, "parentid");

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

sds
GetMessageAttribute(const char *message, const char *attribute)
{  // Retrieves an attribute value, or NULL

	if (!message || !attribute)
		return NULL;

	mxml_node_t *xml = mxmlLoadString(NULL, message, MXML_OPAQUE_CALLBACK);

	mxml_node_t *node = mxmlFindElement(xml, xml, "message", NULL, NULL, MXML_DESCEND);
	if (!node) {
		return NULL;
	}

	const char *value = mxmlElementGetAttr(node, attribute);
	if (!value)
		return NULL;

	sds retval = sdsnew(value);

	free((char *)value);
	return retval;
}
