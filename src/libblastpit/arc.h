#ifndef ARC_H
#define ARC_H

class Arc {
      public:
	Arc(double, double, double, double, double, double);
	virtual ~Arc();

	static Arc FromThreePoints(double start_x, double start_y,
				   double middle_x, double middle_y,
				   double end_x, double end_y);

	int GetSvgParameters(double &start_x, double &start_y, double &end_x,
			     double &end_y, double &radius, bool &large_arc);
	double GetRadius();

      private:
	double start_x;
	double start_y;
	double centre_x;
	double centre_y;
	double end_x;
	double end_y;
};

#endif /* end of include guard: ARC_H */
