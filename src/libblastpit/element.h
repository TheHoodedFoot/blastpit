#ifndef BPELEMENT_H
#define BPELEMENT_H

enum ArcType {
	AT_NONE,
	AT_CW,
	AT_CCW,
};

class Element {
      public:
	Element(double startX, double startY, double endX, double endY);
	Element(double startX, double startY, double midX, double midY,
		double endX, double endY);
	Element(double startX, double startY, double endX, double endY,
		double centreX, double centreY, bool cw);

	double getSx();
	double getSy();
	double getEx();
	double getEy();
	double getCx();
	double getCy();
	bool isCw();
	void setSx(double startX);
	void setSy(double startY);
	void setEx(double endX);
	void setEy(double endY);
	void setCx(double centreX);
	void setCy(double centreY);
	void setCw(bool cw);

	int isArc();

	// Helper functions for LMOS
	double CenX();
	double ceny();
	double dx();
	double dy();
	double len();
	double rot();
	double rotdeg();
	void rotate(double pivotX, double pivotY, double angle);
	double hsx();  // Start point if rotated about centre to make
	// horizontal
	double hsy();
	double hex();
	double hey();

      protected:
	double sx, sy, ex, ey, cx, cy;
	bool cw;
	int isarc;
};
#endif

/*! \brief Calculate the centre point of an arc or circle
 *
 * Calculates the centre point and direction of an arc or circle given 3
 * points.
 *
 * \param[out] cx The calculated centre point
 * \param[out] cy The calculated centre point
 *
 * \return True if clockwise, otherwise false
 */
