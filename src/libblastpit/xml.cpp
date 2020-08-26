#include "xml.hpp"

#ifdef __cplusplus
#include "pugixml.hpp"
#endif

#include <cstring>
#include <iostream>
#include <sstream>

int
xml_getId(const char* message)
{ /* Returns message id, or -1 if nonexistent */

	if (!message)
		return kInvalid;

	pugi::xml_document xml;
	xml.load_string(message);

	// Zero is considered an invalid id because atoi() returns 0
	// for invalid inputs.
	int id = atoi(xml.child("command").attribute("id").value());
	return id ? id : kInvalid;
}

int
xml_hasHeader(const char* message)
{ /* Returns true if <command> header exists */

	pugi::xml_document xml;

	xml.load_string(message);
	return (xml.child("command") != nullptr);
}

/* The id modification functions below all produce dynamic copies of the input
 * string, which will need to be freed by the caller */

char*
xml_getCommandString(const char* string)
{ /* Returns the pcdata contained by the command node */

	pugi::xml_document xml;

	if (!string)
		return nullptr;
	if (!strlen(string))
		return nullptr;
	xml.load_string(string);
	pugi::xml_node command = xml.child("command");
	if (!command)
		return nullptr;

	return xml_mallocCopy(command.child_value());
}

char*
xml_mallocCopy(const char* string)
{ /* Creates a dynamic memory copy of a string */

	char* copiedString = (char*)calloc(1, strlen(string) + 1);
	if (!copiedString)
		return nullptr;

	strncpy(copiedString, string, strlen(string));
	return copiedString;
}

char*
xml_setId(const int id, const char* message_const)
{ /* Adds or replaces id and returns (realloc)ed string */

	if (!xml_hasHeader(message_const)) {
		char* msgWithHeader = xml_addHeader(message_const);
		char* msgWithId = xml_addId(id, msgWithHeader);
		free(msgWithHeader);
		return msgWithId;
	}

	if (xml_getId(message_const)) {
		char* msgWithoutId = xml_removeId(message_const);
		char* msgWithId = xml_addId(id, msgWithoutId);
		free(msgWithoutId);
		return msgWithId;
	}

	char* withId = xml_addId(id, message_const);
	return withId;
}

char*
xml_addHeader(const char* message)
{ /* Adds the <command> header to a string */

	(void)message;
	std::stringstream xmlstream;
	pugi::xml_document xml;

	pugi::xml_node command = xml.append_child("command");
	pugi::xml_node data = command.append_child(pugi::node_pcdata);
	data.set_value(message);
	xml.save(xmlstream);

	const std::string stdstring = xmlstream.str();
	const char* cstring = stdstring.c_str();
	return xml_mallocCopy(cstring);
}

char*
xml_addId(const int id, const char* message)
{ /* Helper function for xml_setId */

	std::stringstream xmlstream;
	pugi::xml_document xml;

	xml.load_string(message);
	pugi::xml_node command = xml.child("command");
	if (!command)
		return nullptr;
	command.append_attribute("id");
	command.attribute("id") = id;
	xml.save(xmlstream);

	const std::string stdstring = xmlstream.str();
	const char* cstring = stdstring.c_str();
	return xml_mallocCopy(cstring);
}

char*
xml_removeId(const char* message)
{ /* Return an allocated string with the id removed */

	std::stringstream xmlstream;
	pugi::xml_document xml;

	xml.load_string(message);
	xml.child("command").remove_attribute("id");
	xml.save(xmlstream);

	const std::string stdstring = xmlstream.str();
	const char* cstring = stdstring.c_str();
	return xml_mallocCopy(cstring);
}

XmlReply
ParseXmlIdAndRetval(const char* xml)
{  // Extract the id and return value into a struct

	XmlReply reply = {kInvalid, kInvalid};

	if (!xml_hasHeader(xml))
		return reply;

	reply.id = xml_getId(xml);
	char* xmldata = xml_getCommandString(xml);
	reply.retval = atoi(xmldata);
	free(xmldata);

	return reply;
}
