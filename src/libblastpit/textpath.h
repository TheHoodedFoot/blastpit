#ifndef BPTEXTPATH_H
#define BPTEXTPATH_H

#include "bppath.h"
#include "bptext.h"

class bpTextPath {
      public:
	bpTextPath();
	virtual ~bpTextPath();

	void setText(bpText text);
	void setPath(bpPath path);
	bpText* getText();
	bpPath* getPath();
	void flipPath();
	void reversePath();

      private:
	bpText text;
	bpPath path;
	double posAlong;  //!< Distance along path from start point
	bool flip;	//!< Put text above or below path
};
#endif /* end of include guard: BPTEXTPATH_H */
