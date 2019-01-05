#ifndef NULOOQ_H
#define NULOOQ_H

#include <QObject>

enum nlqEx {
	NULOOQ_NO_X,
	NULOOQ_NO_DAEMON,
};

enum nlqButton {
	NUBUTTON_TOP,
	NUBUTTON_BOTTOM,
	NUBUTTON_LEFT,
	NUBUTTON_RIGHT,
	NUBUTTON_INNER,
};

class Nulooq : public QObject {
	Q_OBJECT
      public:
	explicit Nulooq(QObject *parent = 0);

      public slots:
	void Loop();

      signals:
	void motionEvent(int x, int y, int z, int rx, int ry, int rz);
	void buttonEvent(int button, bool state);
	void finished();
	void shutdown();

      public slots:
};
#endif  // NULOOQ_H
