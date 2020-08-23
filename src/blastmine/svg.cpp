#include "svg.h"
#include <iostream>
#include <memory>
#include <regex>
#include <sstream>
#include "common.h"
#include "pugixml.hpp"

// Nanosvg dependencies
#include <math.h>
#include <stdio.h>
#include <string.h>
#define NANOSVG_IMPLEMENTATION	// Expands implementation
#include "nanosvg.h"

Svg::Svg() {}
Svg::~Svg() {}
/*! Imports an SVG file from memory.
    Reads a memory location and stores the resulting XML object in the global
    pointer. If the pointer is not already null then the existing document is
    first destroyed.
    \param [in] byte_array byte array of SVG to be imported
    \return Error code on failure, otherwise zero.  */
int
Svg::ImportSvg(const char *byte_array)
{
	return sizeof(byte_array);  // Squash warning
}

int
Svg::LoadSvg(const std::string filename)
{
	svg.load_file(filename.c_str());
	return false;
	/* return(svg->load_file(filename.c_str())); */
}

/*! Exports an SVG file to memory.
    Allocates a byte array and returns the current SVG data.
    Note: The array must be destroyed by the user.
    \param [out] byte_array The returned byte array
    \return Error code on failure, otherwise zero.  */
int
Svg::ExportSvg(char *byte_array[])
{
	return sizeof(*byte_array);  // Squash warning
}

/*! Exports SVG data as Rofin XML.
    Converts the SVG data into a format compatible with the Rofin XML importer
    and returns it as a string.
    \param [out] svgxml String reference to the xml result
    \return Error code on failure, otherwise zero.  */
int
Svg::ExportXml(std::string &svgxml)
{
	// Preamble
	pugi::xml_document xml;
	pugi::xml_node drawing = xml.append_child("DRAWING");
	drawing.append_attribute("UNIT").set_value("MM");
	pugi::xml_node root = drawing.append_child("ROOT");
	root.append_attribute("ID").set_value("Blastpit_Export");
	root.append_attribute("WIDTH").set_value("120.0");
	root.append_attribute("HEIGHT").set_value("120.0");

	// Layers
	//
	// Iterate through <g> elements with an attribute
	// inkscape:groupmode=layer
	// (ignore hidden layers)
	//
	// Extract the height from the layer name
	// Get the colour
	//
	pugi::xml_node layer = root.append_child("LAYER");
	layer.append_attribute("NAME").set_value("Layer [60.0]");
	layer.append_attribute("HEIGHT_Z_AXIS").set_value("60.0");
	layer.append_attribute("COLOR").set_value("227,127,27");

	// Marking objects
	//
	// For each path:
	//
	// Polyline header
	//
	// ID
	pugi::xml_node polyline = root.append_child("POLYLINE");
	polyline.append_attribute("ID").set_value("POLYLINE0001");
	polyline.append_attribute("HATCH").set_value("Y");
	polyline.append_attribute("HP").set_value("Blastpit");
	polyline.append_attribute("DESCRIPTION").set_value("Layer [60.0]");
	polyline.append_attribute("LP").set_value("Blastpit");

	pugi::xml_node polypoint = polyline.append_child("POLYPOINT");
	polypoint.append_attribute("TYPE").set_value("LINE");
	polypoint.text().set("25.0 25.0");
	polypoint = polyline.append_child("POLYPOINT");
	polypoint.append_attribute("TYPE").set_value("LINE");
	polypoint.text().set("25.0 35.0");
	polypoint = polyline.append_child("POLYPOINT");
	polypoint.append_attribute("TYPE").set_value("ARC");
	polypoint.text().set("35.0 35.0");
	polypoint = polyline.append_child("POLYPOINT");
	polypoint.append_attribute("TYPE").set_value("LINE");
	polypoint.text().set("35.0 25.0");
	polypoint = polyline.append_child("POLYPOINT");
	polypoint.append_attribute("TYPE").set_value("LINE");
	polypoint.text().set("25.0 25.0");

	// Convert to text and export
	// Remember: colour attribute of layer must be after the name
	// attribute
	//

	std::ostringstream buf;
	xml.save(buf);
	svgxml = buf.str();
	return kOK;  // Squash warning
}

/*! Convert a bezier curve to circular arcs.
    Converts a bezier curve to circular arcs using de Casteljau
    \param [in] tolerance The maximum allowed deviation from the original
   curve \param [in] original_path segment in SVG notation \param [out]
   new_path converted path in SVG notation \return Returns zero on success,
   otherwise returns a BpError code.
*/
int
Svg::BezierToArcs(float tolerance, const std::string original_path, std::string &new_path)
{
	(void)original_path;
	(void)new_path;
	// Iterate through bezier nodes
	//
	//
	return (int)tolerance;
}

/*! Converts SVG bezier string to a node array.
    Takes a string in SVG cubic bezier notation and returns an array of nodes.
    \param [in] path SVG path string
    \param [out] nodes Bezier node array
    \return Returns zero on success, otherwise returns a BpError code.
*/
/* int */
/* Svg::StringToNodes(const std::string path, std::vector<BezierNode> &nodes) */
/* { */
/* 	(void)path; */
/* 	(void)nodes; */
/* 	return false; */
/* } */

/*! Converts a node array to an SVG compatible path.
    Takes an array of nodes and returns a string in SVG cubic bezier notation
    \return Returns zero on success, otherwise returns a BpError code.
    \param [in] nodes Bezier node array
    \param [out] path SVG path string
*/
/* int */
/* Svg::NodesToString(const std::vector<BezierNode> nodes, std::string &path) */
/* { */
/* 	(void)nodes; */
/* 	(void)path; */
/* 	// Ensure that there are at least two nodes */
/* 	// */
/* 	// Extract start point from first node */
/* 	// */
/* 	// Add start point to text */
/* 	// */
/* 	// For each node: */
/* 	// */
/* 	//      calculate control handles */
/* 	// */
/* 	//      add control handles to text */
/* 	// */
/* 	// Add end point to text */

/* 	return false; */
/* } */

/* int */
/* Svg::SegmentsToNodes(const std::vector<Segment> segments, */
/* 		     std::vector<BezierNode> &nodes) */
/* { */
/* 	(void)segments; */
/* 	(void)nodes; */
/* 	return false; */
/* } */

/* int */
/* Svg::NodesToSegments(const std::vector<BezierNode> nodes, */
/* 		     std::vector<Segment> &segments) */
/* { */
/* 	(void)nodes; */
/* 	(void)segments; */
/* 	return false; */
/* } */

/*! Convert the SVG to lines and arcs.
    Converts all paths to use only lines and arc segments. This operation is
    destructive.
    \param [in] tolerance The maximum allowed deviation from the original
   curve \return Returns zero on success, otherwise returns a BpError code. */
int
Svg::Decompose(float tolerance)
{
	// Get a list of all paths
	//      (all nodes with 'd' attribute)

	/* struct simple_walker : pugi::xml_tree_walker { */
	/* 	virtual bool */
	/* 	for_each(pugi::xml_node &node) */
	/* 	{ */
	/* 		const char *attr; */
	/* 		attr = node.attribute("d").value(); */
	/* 		if (*attr) { */
	/* 			std::cout << attr << std::endl; */
	/* 		} */

	/* 		return true;  // continue traversal */
	/* 	} */
	/* }; */

	/* simple_walker walker; */
	/* svg.traverse(walker); */

	//
	// Iterate through all paths
	//
	// For each path, find any section that is cubic bezier or
	// non-circular elliptical arc. Split the path at that section and
	// convert to circular arcs.
	//
	// Join the path back together if necessary.
	//
	//

	return tolerance;
}

/*! Converts an SVG path string into a segment array.
   Takes a single svg path string and returns a vector array of line or
   circular arc segments. \param [in] path an XML path attribute containing
   the SVG path string \param [out] segments The returned array of path
   elements \return A BpError code
 */
/* int */
/* Svg::ParsePath(const pugi::xml_attribute *path, */
/* 	       std::vector<Segment> &segments) */
/* { */
/* 	(void)segments; */
/* 	// Get the first move (or bail) */
/* 	const std::string value = path->value(); */
/* 	int pos = value.find_first_of("Mm"); */
/* 	if (!pos) return false; */

/* 	// Ignore whitespace and get a coordinate pair */
/* 	std::regex( */
/* 		"((\\+|-)?[[:digit:]]+)(\\.(([[:digit:]]+)?))?[[:space:]]*,?["
 */
/* 		"[" */
/* 		":" */
/* 		"space:]]*((\\+|-)?[[:digit:]]+)(\\.(([[:digit:]]+)?))?", */
/* 		std::regex_constants::ECMAScript); */
/* 	std::smatch match; */


/* 	return false; */
/* } */

int
Svg::NanoTest(const std::string filename)
{
	// Load
	struct NSVGimage *image;
	image = nsvgParseFromFile(filename.c_str(), "px", 96);
	printf("size: %f x %f\n", image->width, image->height);
	// Use...
	NSVGshape *shape;
	NSVGpath *path;
	int i;
	for (shape = image->shapes; shape != NULL; shape = shape->next) {
		for (path = shape->paths; path != NULL; path = path->next) {
			for (i = 0; i < path->npts - 1; i += 3) {
				float *p = &path->pts[i * 2];
				std::cout << p[0] << p[1] << p[2] << p[3] << p[4] << p[5] << p[6] << p[7] << std::endl;
			}
		}
	}
	// Delete
	nsvgDelete(image);
	return false;
}
