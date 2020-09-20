#include "svgcmd.h"
#include <cppunit/CompilerOutputter.h>
#include <cppunit/TestAssert.h>
#include <cppunit/TestCaller.h>
#include <cppunit/TestSuite.h>
#include <cppunit/ui/text/TestRunner.h>
#include <getopt.h>
#include <stddef.h>
#include <stdlib.h>  // for exit()
#include <algorithm>
#include <cstring>
#include <iostream>
#include <vector>
/* #include "arc.h" */
#include "common.h"
/* #include "segment.h" */
#include "svg.h"

/* Flag set by ‘--verbose’. */
static int verbose_flag;

void
OutputXml()
{
	Svg	    test;
	std::string xml;
	test.ExportXml(xml);

	int len = xml.length();
	// Packet starts with 3 x 32bit words, little-endian
	printf("%c%c%c%c", 0, 0, 0, 0);	 // ID (not used)
	/* printf("%c%c%c%c", BP_LOAD_SVG, 0, 0, 0);  // Command */
	printf("%c%c%c%c", len, len >> 8, 0, 0);  // Subsequent data size
	std::cout << xml;
}

void
DTest(const char *filename)
{
	Svg test;
	test.LoadSvg(filename);
	test.Decompose(0);
}

void
NTest(const char *filename)
{
	Svg test;
	test.NanoTest(filename);
}

void
SvgTest::setUp()
{
}
void
SvgTest::tearDown()
{
}
/* void */
/* SvgTest::BezierToArcsTest() */
/* { */
/* 	Arc a = Arc::FromThreePoints(10, 0, 0, 10, -10, 0); */
/* 	double r = a.GetRadius(); */
/* 	std::cout << "Radius = " << r << std::endl; */
/* 	CPPUNIT_ASSERT(r == 10); */
/* } */

void
SvgTest::DecomposeTest()
{
	Svg test;
	test.LoadSvg("/home/thf/usr/src/blastpit/utils/test.svg");
	test.Decompose(0);
}

/* void */
/* SvgTest::NodesToStringTest() */
/* { */
/* } */
void
SvgTest::SvgToRofinTest()
{
	CPPUNIT_ASSERT(false);
}
/* void*/
/* SvgTest::StringToNodesTest()*/
/* {*/
/* 	 SVG test data*/
/* 	 **/
/* 	 * This data is all equivalent and should all generate the same*/
/* 	 * geometries,*/
/* 	 */
/* 	static std::string svg1("M 10 10 L 20 20 L 20 30 z");*/
/* 	static std::string svg2("m 20 20 L 20 30 L 10 10 z");*/

/* 	std::vector<Segment> seg1 = {Segment(10, 10, 15, 15, 20, 20),*/
/* 				     Segment(20, 20, 20, 25, 20, 30),*/
/* 				     Segment(20, 30, 15, 20, 10, 10)};*/
/* 	std::vector<Segment> seg2 = {Segment(20, 20, 20, 25, 20, 20),*/
/* 				     Segment(20, 20, 15, 15, 10, 10),*/
/* 				     Segment(10, 10, 15, 20, 20, 30)};*/

/* 	std::vector<Segment> test_seg_1, test_seg_2;*/
/* 	std::vector<BezierNode> test_bez_1, test_bez_2;*/
/* 	Svg::StringToNodes(svg1, test_bez_1);*/
/* 	Svg::StringToNodes(svg2, test_bez_2);*/
/* 	Svg::NodesToSegments(test_bez_1, test_seg_1);*/
/* 	Svg::NodesToSegments(test_bez_2, test_seg_2);*/
/* 	std::sort(seg1.begin(), seg1.end());*/
/* 	std::sort(seg2.begin(), seg2.end());*/
/* 	Segment s1 = seg1.front();*/
/* 	Segment s2 = seg2.front();*/
/* 	CPPUNIT_ASSERT(s1 == s2);*/
/* }*/
int
RunTests()
{
	CppUnit::TestSuite *suite = new CppUnit::TestSuite("SvgSuite");

	// Register all tests here
	suite->addTest(new CppUnit::TestCaller<SvgTest>("SVG to lines and arcs", &SvgTest::DecomposeTest));
	/* suite->addTest(new CppUnit::TestCaller<SvgTest>( */
	/* 	"Bezier to arcs", &SvgTest::BezierToArcsTest)); */
	suite->addTest(new CppUnit::TestCaller<SvgTest>("SVG to Rofin", &SvgTest::SvgToRofinTest));
	/* suite->addTest(new CppUnit::TestCaller<SvgTest>( */
	/* 	"Nodes to string", &SvgTest::NodesToStringTest)); */
	/* suite->addTest(new CppUnit::TestCaller<SvgTest>( */
	/* 	"String to nodes", &SvgTest::StringToNodesTest)); */

	// Execute tests
	CppUnit::TextUi::TestRunner runner;
	runner.addTest(suite);

	runner.setOutputter(new CppUnit::CompilerOutputter(&runner.result(), std::cerr));

	bool wasSucessful = runner.run();

	return wasSucessful ? 0 : 1;
}

int
main(int argc, char **argv)
{
	int c;

	while (1) {
		static struct option long_options[] = {/* These options set a flag. */
						       {"verbose", no_argument, &verbose_flag, 1},
						       {"brief", no_argument, &verbose_flag, 0},
						       /* These options don't set a flag.
							  We distinguish them by their indices. */
						       {"add", no_argument, 0, 'a'},
						       {"append", no_argument, 0, 'b'},
						       {"create", required_argument, 0, 'c'},
						       {"decompose", required_argument, 0, 'd'},
						       {"nanotest", required_argument, 0, 'n'},
						       {"file", required_argument, 0, 'f'},
						       {"output", no_argument, 0, 'o'},
						       {"server", required_argument, 0, 's'},
						       {"test", no_argument, 0, 't'},
						       {0, 0, 0, 0}};
		/* getopt_long stores the option index here. */
		int option_index = 0;

		c = getopt_long(argc, argv, "abc:d:f:os:tn:", long_options, &option_index);

		/* Detect the end of the options. */
		if (c == -1)
			break;

		switch (c) {
			case 0:
				/* If this option set a flag, do nothing else
				 * now. */
				if (long_options[option_index].flag != 0)
					break;
				std::cout << "option " << long_options[option_index].name;
				if (optarg)
					std::cout << " with arg " << optarg;
				std::cout << std::endl;
				break;

			case 'a':
				static_assert(std::is_arithmetic<int>::value, "Static Assert Test");
				break;

			case 'c':
				// printf("option -c with value `%s'\n",
				// optarg);
				break;

			case 'o':
				OutputXml();
				break;

			case 'd':
				DTest(optarg);
				break;

			case 'n':
				NTest(optarg);
				break;

			case 't':
				/* exit(RunTests()); */
				break;

			case '?':
				/* getopt_long already printed an error
				 * message.
				 */
				break;

			default:
				std::cout << "Unrecognised option" << std::endl;
		}
	}

	/* Instead of reporting ‘--verbose’
	   and ‘--brief’ as they are encountered,
	   we report the final status resulting from them. */
	if (verbose_flag)
		std::cout << "verbose flag is set";

	/* Print any remaining command line arguments (not options). */
	if (optind < argc) {
		std::cout << "non-option ARGV-elements: ";
		while (optind < argc)
			std::cout << argv[optind++] << " ";
		std::cout << std::endl;
	}

	exit(0);
}
