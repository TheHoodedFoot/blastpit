/// \file nodes.h
/// \brief Structs and functions for CImNodes node editor

#ifndef NODES_H
#define NODES_H

#ifdef __cplusplus
extern "C"
{
#endif

	/// Unique identifiers for each node type
	enum NODE_TYPE
	{
		NODE_FILENAME,	///< File chooser
		NODE_PREVIEW,	///< Displays preview on screen
	};

	/// Unique identifiers for each attribute type
	enum ATTR_TYPE
	{
		ATTR_SVG,   ///< Complete SVG stream
		ATTR_GEOS,  ///< Geometry data only
	};

	/// Error codes for node processing
	enum NODE_ERROR
	{
		ERROR_UNKNOWN = 1,  ///< Undocumented error
	};

	/// Data common to each instance of the same node type
	typedef struct
	{
		enum NODE_TYPE type;			 ///< Unique identifier
		const char*    name;			 ///< The node name as displayed in the titlebar
		int	       ( *node_func )( void* );	 ///< Pointer to function that processes this node type
	} t_node_cl;

	/// Data common to each instance of the same attribute type
	typedef struct
	{
		enum ATTR_TYPE type;	///< Unique identifier
		const char*    name;	///< Attribute label
		int	       colour;	///< RGB value
	} t_attr_cl;

	/// Data specific to each single node instance
	typedef struct
	{
		int	   node_id;	   ///<  Unique identifier
		t_node_cl* type;	   ///< Class type of this node
		int	   x, y;	   ///< Position
		int	   width, height;  ///< Size
	} t_node;

	/// Link class
	/// Stores the source and destination attributes and their nodes
	typedef struct
	{
		int link_id;	  ///< Unique identifier
		int source_node;  ///< ImGui ID of the source node
		int source_attr;  ///< ImGui ID of the source attribute
		int dest_node;	  ///< ImGui ID of the destination node
		int dest_attr;	  ///< ImGui ID of the destination attribute
	} t_link;

	// Functions for processing nodes.
	// All of these functions take a void pointer that is cast to a struct specific to this
	// node type.
	/// \param self Pointer to node-specific struct
	/// \return Zero for success, else NODE_ERROR
	int node_filename( void* );
	int node_preview( void* );

#ifdef __cplusplus
}
#endif

#endif	// end of include guard: NODES_H
