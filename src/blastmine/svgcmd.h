#ifndef SVGCMD_H
#define SVGCMD_H

#include <cppunit/TestFixture.h>
#include "svg.h"

class SvgTest : public CppUnit::TestFixture {
      private:
	Svg svg;

      public:
	void OutputXml();
	int  RunTests();

	void setUp();
	void tearDown();

	void BezierToArcsTest();
	void DecomposeTest();
	void NodesToStringTest();
	void StringToNodesTest();
	void SvgToRofinTest();
};

#endif /* end of include guard: SVGCMD_H */
