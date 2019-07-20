#include "arc.h"
#include "math.h"

Arc::Arc(double start_x, double start_y, double end_x, double end_y,
	 double centre_x, double centre_y)
{
	this->start_x = start_x;
	this->start_y = start_y;
	this->centre_x = centre_x;
	this->centre_y = centre_y;
	this->end_x = end_x;
	this->end_y = end_y;
}

Arc::~Arc() {}
/*! Create and arc from three points.
   Creates an arc from start, middle and end points.
   \param [in] start_x The start x co-ordinate
   \param [in] start_y The start y co-ordinate
   \param [in] middle_x The middle x co-ordinate
   \param [in] middle_y The middle y co-ordinate
   \param [in] end_x The end x co-ordinate
   \param [in] end_y The end y co-ordinate
   \return An Arc object or null if invalid.
 */
Arc
Arc::FromThreePoints(double start_x, double start_y, double middle_x,
		     double middle_y, double end_x, double end_y)
{
	double centre_x =
		0.5 *
		((start_x * start_x + start_y * start_y) *
			 (end_x - middle_x) +
		 (middle_x * middle_x + middle_y * middle_y) *
			 (start_x - end_x) +
		 (end_x * end_x + end_y * end_y) * (middle_x - start_x)) /
		(start_y * (end_x - middle_x) + middle_y * (start_x - end_x) +
		 end_y * (middle_x - start_x));
	double centre_y =
		0.5 *
		((start_x * start_x + start_y * start_y) *
			 (end_y - middle_y) +
		 (middle_x * middle_x + middle_y * middle_y) *
			 (start_y - end_y) +
		 (end_x * end_x + end_y * end_y) * (middle_y - start_y)) /
		(start_x * (end_y - middle_y) + middle_x * (start_y - end_y) +
		 end_x * (middle_y - start_y));

	return Arc(start_x, start_y, end_x, end_y, centre_x, centre_y);
}

/*! Get SVG elliptical arc parameters.
   Returns the arc data in a format compatible with the SVG elliptical arc
   curve. The arc is considered to be circular so only one radius is required,
   and both the rotation and sweep flag are zero.
   \param [out] start_x The x co-ordinate of the start of the arc
   \param [out] start_y The y co-ordinate of the start of the arc
   \param [out] end_x The x co-ordinate of the end of the arc
   \param [out] end_y The y co-ordinate of the end of the arc
   \param [out] radius The arc radius
   \param [out] large_arc The SVG large_arc flag
   \return zero on success, otherwise a BpError code
 */
int
Arc::GetSvgParameters(double &start_x, double &start_y, double &end_x,
		      double &end_y, double &radius, bool &large_arc)
{
	start_x = this->start_x;
	start_y = this->start_y;
	end_x = this->end_x;
	end_y = this->end_y;
	radius = this->GetRadius();

	large_arc = 0;

	return false;
}

double
Arc::GetRadius()
{
	return sqrt((start_x - centre_x) * (start_x - centre_x) +
		    (start_y - centre_y) * (start_y - centre_y));
}
