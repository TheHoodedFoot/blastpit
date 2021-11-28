#ifndef XML_H
#define XML_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <sds.h>

	int  XmlGetMessageCount( const char* xml );
	sds  XmlDeleteAttribute( sds message, const char* attribute );
	sds  XmlGetAttribute( const char* message, const char* attribute );
	sds  XmlGetChildNodeAsString( const char* message_str, const char* child );
	sds  XmlGetMessageByIndex( const char* xml, int index );
	sds  XmlSetAttribute( sds message, const char* attribute, const char* value );
	sds  XmlNewCdata( const char* message );
	void XmlAddXmlHeader( sds* message );
	sds  XmlDrawingToMessage( sds drawing );
	sds  XmlExtractMessagePayload( const char* xml );

#ifdef __cplusplus
}
#endif

#endif /* end of include guard: XML_H */
