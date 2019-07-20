#ifndef SEGMENT_H
#define SEGMENT_H

class Segment {
      public:
	Segment(double startx, double starty, double endx, double endy);
	Segment(double startx, double starty, double midx, double midy,
		double endx, double endy);
	virtual ~Segment();

	bool operator<(const Segment&) const;
	bool operator==(const Segment&) const;

	bool IsLine();
	bool IsArc();

	double GetRadius();
	double GetLength();

	void SetStartX(double);
	void SetStartY(double);
	void SetMiddleX(double);
	void SetMiddleY(double);
	void SetEndX(double);
	void SetEndY(double);
	void SetCentreX(double);
	void SetCentreY(double);

	double GetStartX();
	double GetStartY();
	double GetMiddleX();
	double GetMiddleY();
	double GetEndX();
	double GetEndY();
	double GetCentreX();
	double GetCentreY();

      private:
	double start_x, start_y;
	double middle_x, middle_y;
	double end_x, end_y;
};

#endif /* end of include guard: SEGMENT_H */
