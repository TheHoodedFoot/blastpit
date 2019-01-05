#include "segment.h"

Segment::Segment(double startx, double starty, double endx, double endy)
{
	start_x = startx;
	start_y = starty;
	end_x = endx;
	end_y = endy;
	middle_x = (startx + endx) / 2;
	middle_y = (starty + endy) / 2;
}

Segment::Segment(double startx, double starty, double midx, double midy,
		 double endx, double endy)
{
	start_x = startx;
	start_y = starty;
	middle_x = midx;
	middle_y = midy;
	end_x = endx;
	end_y = endy;
}

Segment::~Segment() {}
bool
Segment::operator<(const Segment& comparison) const
{
	if (start_x < comparison.start_x) return true;
	if (start_x > comparison.start_x) return false;

	if (start_y < comparison.start_y) return true;
	if (start_y > comparison.start_y) return false;

	if (middle_x < comparison.middle_x) return true;
	if (middle_x > comparison.middle_x) return false;

	if (middle_y < comparison.middle_y) return true;
	if (middle_y > comparison.middle_y) return false;

	if (end_x < comparison.end_x) return true;
	if (end_x > comparison.end_x) return false;

	if (end_y < comparison.end_y) return true;
	return false;
}

bool
Segment::operator==(const Segment& comparison) const
{
	if (start_x != comparison.start_x) return false;
	if (start_y != comparison.start_y) return false;
	if (middle_x != comparison.middle_x) return false;
	if (middle_y != comparison.middle_y) return false;
	if (end_x != comparison.end_x) return false;
	if (end_y != comparison.end_y) return false;
	return true;
}
