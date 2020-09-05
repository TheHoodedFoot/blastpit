#ifndef XML_H
#define XML_H

#ifdef __cplusplus
extern "C" {
#endif

int HasMultipleMessages(const char* xml);
int GetMessageId(const char* message);
char* GetMessageByIndex(const char* xml, int index);

#ifdef __cplusplus
}
#endif

#endif /* end of include guard: XML_H */
