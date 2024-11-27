#include "xml.h"
#include "blastpit.h"
#include "sds.h"
#include <mxml.h>

int XmlGetMessageCount( const char* xml )
{  // Returns count of <message> elements or kInvalid on bad xml

	if ( !xml ) {
		return 0;
	}

	mxml_options_t* mxmlopts = mxmlOptionsNew();
	mxml_node_t*	tree	 = mxmlLoadString( NULL, mxmlopts, xml );
	mxmlOptionsDelete( mxmlopts );

	mxml_node_t* node;
	int	     count = 0;

	for ( node = mxmlFindElement( tree, tree, "message", NULL, NULL, MXML_DESCEND_ALL ); node != NULL;
	      node = mxmlFindElement( node, tree, "message", NULL, NULL, MXML_DESCEND_ALL ) ) {
		count++;
	}

	mxmlDelete( tree );
	return count;
}

sds XmlGetAttribute( const char* message, const char* attribute )
{  // Retrieves an attribute value, or NULL

	if ( !message || !attribute ) {
		return NULL;
	}

	mxml_options_t* mxmlopts = mxmlOptionsNew();
	mxml_node_t*	xml	 = mxmlLoadString( NULL, mxmlopts, message );
	mxmlOptionsDelete( mxmlopts );

	mxml_node_t* node = mxmlFindElement( xml, xml, "message", NULL, NULL, MXML_DESCEND_ALL );
	if ( !node ) {
		mxmlDelete( xml );
		return NULL;
	}

	const char* value = mxmlElementGetAttr( node, attribute );
	if ( !value ) {
		mxmlDelete( xml );
		return NULL;
	}

	sds retval = sdsnew( value );

	mxmlDelete( xml );

	return retval;
}

sds XmlGetChildNodeAsString( const char* message_str, const char* child )
{  // Returns the child node tree as a string

	if ( !message_str ) {
		BPLOG( kLvlDebug, "%s: Invalid message string supplied\n", __func__ );
		return NULL;
	}
	if ( !strlen( message_str ) ) {
		BPLOG( kLvlDebug, "%s: Invalid message string length\n", __func__ );
		return NULL;
	}

	sds message = sdsnew( message_str );
	XmlAddXmlHeader( &message );
	mxml_options_t* mxmlopts = mxmlOptionsNew();
	mxml_node_t*	xml	 = mxmlLoadString( NULL, mxmlopts, message );
	mxmlOptionsDelete( mxmlopts );
	if ( !xml ) {
		BPLOG( kLvlDebug, "%s: Couldn't add xml header\n", __func__ );
		sdsfree( message );
		return NULL;
	}

	mxml_node_t* node = mxmlFindElement( xml, xml, child, NULL, NULL, MXML_DESCEND_ALL );
	if ( node ) {
		BPLOG( kLvlDebug, "%s: Converting tree to string\n", __func__ );
		mxml_options_t* mxmlopts   = mxmlOptionsNew();
		char*		mxmlString = mxmlSaveAllocString( node, mxmlopts );
		mxmlOptionsDelete( mxmlopts );
		sds node_data = sdsnew( mxmlString );
		free( mxmlString );
		mxmlDelete( xml );
		sdsfree( message );
		return node_data;
	}

	BPLOG( kLvlDebug, "%s: Couldn't find the message element\n", __func__ );
	sdsfree( message );
	return NULL;
}

sds XmlGetMessageByIndex( const char* xml, int index )
{  // Returns a string containing a single message element

	mxml_options_t* mxmlopts = mxmlOptionsNew();
	mxml_node_t*	tree	 = mxmlLoadString( NULL, mxmlopts, xml );

	mxml_node_t* node;
	int	     count = 0;

	// TODO: Memory leaks here
	for ( node = mxmlFindElement( tree, tree, "message", NULL, NULL, MXML_DESCEND_ALL );
	      node != NULL && count < index;
	      node = mxmlFindElement( node, tree, "message", NULL, NULL, MXML_DESCEND_ALL ) ) {
		count++;
	}

	if ( !node ) {
		mxmlOptionsDelete( mxmlopts );
		return NULL;
	}
	char* node_data = mxmlSaveAllocString( node, mxmlopts );
	mxmlOptionsDelete( mxmlopts );
	sds retval = sdsnew( "<?xml?>" );
	retval	   = sdscat( retval, node_data );
	free( node_data );
	mxmlDelete( tree );

	return retval;
}

sds XmlGetPayloadByIndex( const char* xml, int index )
{  // Returns a message payload

	mxml_options_t* mxmlopts = mxmlOptionsNew();
	mxml_node_t*	tree	 = mxmlLoadString( NULL, mxmlopts, xml );

	mxml_node_t* node;
	int	     count = 0;

	// TODO: Memory leaks here
	for ( node = mxmlFindElement( tree, tree, "message", NULL, NULL, MXML_DESCEND_ALL );
	      node != NULL && count < index;
	      node = mxmlFindElement( node, tree, "message", NULL, NULL, MXML_DESCEND_ALL ) ) {
		count++;
	}

	if ( !node ) {
		return NULL;
		mxmlOptionsDelete( mxmlopts );
	}
	char* node_data = mxmlSaveAllocString( node, mxmlopts );
	mxmlOptionsDelete( mxmlopts );
	sds retval = sdsnew( "<?xml?>" );
	retval	   = sdscat( retval, node_data );
	free( node_data );
	mxmlDelete( tree );

	return retval;
}

sds XmlSetAttribute( sds message, const char* attribute, const char* value )
{  // Sets an attribute value. Invalidates original message.

	if ( !message || !attribute || !value ) {
		return NULL;
	}

	XmlAddXmlHeader( &message );
	mxml_options_t* mxmlopts = mxmlOptionsNew();
	mxml_node_t*	xml	 = mxmlLoadString( NULL, mxmlopts, message );

	mxml_node_t* node = mxmlFindElement( xml, xml, "message", NULL, NULL, MXML_DESCEND_ALL );
	if ( !node ) {
		mxmlDelete( xml );
		return NULL;
		mxmlOptionsDelete( mxmlopts );
	}

	mxmlElementSetAttr( node, attribute, value );

	char* node_data = mxmlSaveAllocString( node, mxmlopts );
	mxmlOptionsDelete( mxmlopts );
	sds retval = sdsnew( node_data );
	free( node_data );
	mxmlDelete( xml );
	sdsfree( message );  // Caller must treat existing message as deleted

	return retval;
}

void XmlAddXmlHeader( sds* message )
{  // Helper to add the <?xml?> header to allow mxmlLoadString to work

	sds header_added = sdsnew( "<?xml?>" );
	header_added	 = sdscatsds( header_added, *message );
	sdsfree( *message );
	*message = header_added;
}

sds XmlDeleteAttribute( sds message, const char* attribute )
{  // Deletes an attribute value. Invalidates original message.

	if ( !message || !attribute ) {
		return NULL;
	}

	XmlAddXmlHeader( &message );
	mxml_options_t* mxmlopts = mxmlOptionsNew();
	mxml_node_t*	xml	 = mxmlLoadString( NULL, mxmlopts, message );

	mxml_node_t* node = mxmlFindElement( xml, xml, "message", NULL, NULL, MXML_DESCEND_ALL );
	if ( !node ) {
		mxmlDelete( xml );
		mxmlOptionsDelete( mxmlopts );
		return NULL;
	}

	mxmlElementClearAttr( node, attribute );

	char* node_data = mxmlSaveAllocString( node, mxmlopts );
	mxmlOptionsDelete( mxmlopts );
	sds retval = sdsnew( node_data );
	free( node_data );
	mxmlDelete( xml );
	sdsfree( message );  // Caller must treat existing message as deleted

	return retval;
}

sds XmlNewCdata( const char* message )
{  // Adds a CDATA string to a <message>

	if ( !message ) {
		return NULL;
	}

	mxml_node_t* xml	  = mxmlNewXML( "1.0" );
	mxml_node_t* message_node = mxmlNewElement( xml, "message" );
	mxmlNewCDATA( message_node, message );

	// We don't want the xml header, just the message node
	mxml_options_t* mxmlopts   = mxmlOptionsNew();
	char*		xml_string = mxmlSaveAllocString( message_node, mxmlopts );
	mxmlOptionsDelete( mxmlopts );

	sds retval = sdsnew( xml_string );
	free( xml_string );
	mxmlDelete( xml );

	return retval;
}

sds XmlDrawingToMessage( sds drawing )
{  // Rewrites Xml.

	if ( !drawing ) {
		return NULL;
	}

	// XmlAddXmlHeader(&message);
	mxml_options_t* mxmlopts = mxmlOptionsNew();
	mxml_node_t*	xml	 = mxmlLoadString( NULL, mxmlopts, drawing );
	mxml_node_t*	tidy	 = mxmlNewXML( "1.0" );
	mxml_node_t*	message	 = mxmlNewElement( tidy, "message" );
	mxmlAdd( message, MXML_ADD_AFTER, NULL, xml );
	char* tree = mxmlSaveAllocString( tidy, mxmlopts );
	mxmlOptionsDelete( mxmlopts );
	sds retval = sdsnew( tree );
	free( tree );
	mxmlDelete( xml );
	mxmlDelete( tidy );

	return retval;
}

sds XmlExtractMessagePayload( const char* xml )
{  // Returns the string payload of a message, or NULL

	if ( !xml ) {
		return NULL;
	}

	mxml_options_t* mxmlopts = mxmlOptionsNew();
	mxml_node_t*	message	 = mxmlLoadString( NULL, mxmlopts, xml );
	mxmlOptionsDelete( mxmlopts );
	mxml_node_t* node = mxmlFindElement( message, message, "message", NULL, NULL, MXML_DESCEND_ALL );
	if ( !node ) {
		mxmlDelete( message );
		return NULL;
	}

	const char* node_data = mxmlGetOpaque( node );
	sds	    retval    = sdsnew( node_data );
	// free(node_data);
	mxmlDelete( message );

	return retval;
}
