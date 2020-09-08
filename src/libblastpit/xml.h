#ifndef XML_H
#define XML_H

#ifdef __cplusplus
extern "C" {
#endif

#include <sds.h>

int HasMultipleMessages(const char* xml);
int GetMessageId(const char* message);
int GetParentId(const char* message);
char* GetMessageByIndex(const char* xml, int index);
char* GetMessageAttribute(const char* message, const char* attribute);
int xml_hasHeader(const char* message);
sds GetCdata(const char* string);

#ifdef __cplusplus
}
#endif

#endif /* end of include guard: XML_H */
