#include "nodes.h"

// Base nodes
t_node_cl node_classes[] = { { NODE_FILENAME, "Filename", node_filename }, { NODE_PREVIEW, "Preview", node_preview } };

// Array of nodes
t_node nodes[] = { { 1, &node_classes[NODE_FILENAME], 0, 0 } };

// Array of all attributes and their associated nodes
t_attr_cl attr_classes[] = {};

// Array of attributes
t_attr attrs[] = {};

// Array of links. Links join output nodes to input nodes.
t_link links[] = {};


// ░▀█▀░█░█░█▀█░█░█░█▀▀░█░█░▀█▀░█▀▀
// ░░█░░█▀█░█░█░█░█░█░█░█▀█░░█░░▀▀█
// ░░▀░░▀░▀░▀▀▀░▀▀▀░▀▀▀░▀░▀░░▀░░▀▀▀

// Below is an instruction that describes a task, paired with an input that provides further context. Write a response
// that appropriately completes the request.
//
// ### Instruction:
// I wish to create a C program that can take a directed acyclic graph of nodes and generate a sequence of operations to
// be performed based on the graph.
//
// Each node of the graph has the following properties:
// * Each node has a type that determines its function.
// * Each node type has an associated struct that maintains the state of the node.
// * Each node type has a specific C function that is called to process the node.
// * Each node type has attributes.
// * The node attributes have different types.
// * A node type may be either input or output.
// * The output attributes of a node can be linked to the input attributes of
//   another node, provided that the attributes are the same type.
//
// Create a C program that takes as input an array of nodes
//
// ### Input:
// Input
//
// ### Response:
//
//
// Static arrays:
//   An array of node types
//   An array of attribute types
//   An array showing which attributes a node has
//
// Dynamic arrays:
//   An array of node instances (our graph)
//   An array of non-calculated attribute values
//     (Input attributes that are not generated by a previous output, such as
//     sliders and combo boxes)
//   An array of links between attributes (input to output pairs)
