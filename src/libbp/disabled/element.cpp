#include "element.h"
#define _USE_MATH_DEFINES
#include "math.h"

Element::Element(double x1, double y1, double x2, double y2)
{
	sx = x1;
	sy = y1;
	ex = x2;
	ey = y2;
	isarc = AT_NONE;
}

Element::Element(double startX, double startY, double midX, double midY,
		 double endX, double endY)
{
	sx = startX;
	sy = startY;
	ex = endX;
	ey = endY;

	/*
	    c->center.w = 1.0;
	    vertex *v1 = (vertex *)c->c.p1;
	    vertex *v2 = (vertex *)c->c.p2;
	    vertex *v3 = (vertex *)c->c.p3;
	    float bx = v1->xw; float by = v1->yw;
	    float cx = v2->xw; float cy = v2->yw;
	    float dx = v3->xw; float dy = v3->yw;
	    float temp = cx*cx+cy*cy;
	    float bc = (bx*bx + by*by - temp)/2.0;
	    float cd = (temp - dx*dx - dy*dy)/2.0;
	    float det = (bx-cx)*(cy-dy)-(cx-dx)*(by-cy);
	    if (fabs(det) < 1.0e-6) {
		c->center.xw = c->center.yw = 1.0;
		c->center.w = 0.0;
		c->v1 = *v1;
		c->v2 = *v2;
		c->v3 = *v3;
		return;
		}
	    det = 1/det;
	    c->center.xw = (bc*(cy-dy)-cd*(by-cy))*det;
	    c->center.yw = ((bx-cx)*cd-(cx-dx)*bc)*det;
	    cx = c->center.xw; cy = c->center.yw;
	    c->radius = sqrt((cx-bx)*(cx-bx)+(cy-by)*(cy-by));
	 */

	// bool cw = arcCentreFrom3Points(&cx, &cy, startX, startY, midX,
	// midY, endX, endY); isarc = cw ? AT_CW : AT_CCW;

	(void)midX;
	(void)midY;
}

Element::Element(double x1, double y1, double x2, double y2, double centreX,
		 double centreY, bool cw)
{
	sx = x1;
	sy = y1;
	ex = x2;
	ey = y2;
	cx = centreX;
	cy = centreY;
	isarc = cw ? AT_CW : AT_CCW;
}
double
Element::CenX()
{
	return (sx + ex) * 0.5;
}
int
Element::isArc()
{
	return isarc;
}
double
Element::ceny()
{
	return (sy + ey) / (double)2;
}
double
Element::dx()
{
	return ex - sx;
}
double
Element::dy()
{
	return ey - sy;
}
double
Element::len()
{
	return sqrt((dx() * dx()) + (dy() * dy()));
}
double
Element::rot()
{
	return atan2(dy(), dx());
}
double
Element::rotdeg()
{
	return rot() * (180 / M_PI);
}
/// Rotate element about a pivot point. Takes an angle in degrees
/// and the centre point of the rotation.
void
Element::rotate(double pivotX, double pivotY, double angle)
{
	// Rotate the line about a pivot point
	double nx = pivotX + (sx - pivotX) * cos(angle) -
		    (sy - pivotY) * sin(angle);
	double ny = pivotY + (sx - pivotX) * sin(angle) +
		    (sy - pivotY) * cos(angle);

	sx = nx;
	sy = ny;
	nx = pivotX + (ex - pivotX) * cos(angle) - (ey - pivotY) * sin(angle);
	ny = pivotY + (ex - pivotX) * sin(angle) + (ey - pivotY) * cos(angle);
	ex = nx;
	ey = ny;
}
double
Element::hsx()
{
	return CenX() - len() / (double)2;
}
double
Element::hsy()
{
	return ceny();
}
double
Element::hex()
{
	return CenX() + len() / (double)2;
}
double
Element::hey()
{
	return ceny();
}
double
Element::getSx()
{
	return sx;
}
double
Element::getSy()
{
	return sy;
}
double
Element::getCx()
{
	return cx;
}
double
Element::getCy()
{
	return cy;
}
double
Element::getEx()
{
	return ex;
}
double
Element::getEy()
{
	return ey;
}
bool
Element::isCw()
{
	return cw;
}
