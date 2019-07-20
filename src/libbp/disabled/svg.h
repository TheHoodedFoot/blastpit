#ifndef SVG_H
#define SVG_H

#include <memory>
#include <vector>
#include "pugixml.hpp"
/* #include "segment.h" */

typedef struct {
	double x1;  /*!< start pt. x coordinate */
	double y1;  /*!< start pt. y coordinate */
	double x2;  /*!< end pt. x coordinate */
	double y2;  /*!< end pt. x coordinate */
	double cx1; /*!< control handle 1 x coordinate */
	double cy1; /*!< control handle 1 y coordinate */
	double cx2; /*!< control handle 2 x coordinate */
	double cy2; /*!< control handle 2 y coordinate */
} BezierNode;       /*!< Holds a single cubic bezier section */

class Svg {
      private:
      public:
	Svg();
	virtual ~Svg();

	int ImportSvg(const char *);
	int LoadSvg(const std::string filename);
	int ExportSvg(char *[]);
	int ExportXml(std::string &);
	int Decompose(float);
	int BezierToArcs(float, const std::string, std::string &);
	/* static int StringToNodes(const std::string, */
	/* 			 std::vector<BezierNode> &); */
	/* static int NodesToString(const std::vector<BezierNode>, */
	/* 			 std::string &); */
	/* static int SegmentsToNodes(const std::vector<Segment>, */
	/* 			   std::vector<BezierNode> &); */
	/* static int NodesToSegments(const std::vector<BezierNode>, */
	/* 			   std::vector<Segment> &); */
	/* static int SegmentToArc(const Segment, std::string &); */
	/* static int ParsePath(const pugi::xml_attribute *, */
	/* 		     std::vector<Segment> &); */

	static int NanoTest(const std::string);

      private:
	pugi::xml_document svg;
};

#endif /* SVG_H */
