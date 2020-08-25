#ifndef XML_HPP_ECPGQ3I1
#define XML_HPP_ECPGQ3I1

#include "blastpit.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	int id;
	int retval;
} XmlReply;

char* xml_addHeader(const char* message);
char* xml_addId(const int id, const char* message);
char* xml_removeId(const char* message);
char* xml_mallocCopy(const char* string);
char* xml_getCommandString(const char* string);
char* xml_setId(const int id, const char* message);
int xml_getId(const char* message);
int xml_hasHeader(const char* message);
XmlReply ParseXmlIdAndRetval(const char* xml);

#ifdef __cplusplus
}
#endif

#endif /* end of include guard: XML_HPP_ECPGQ3I1 */
