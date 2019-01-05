#ifndef RUNNER_H
#define RUNNER_H

#include <QObject>

class Runner : public QObject {
	Q_OBJECT
      public:
	Runner(QObject *parent, int argc, char **argv);

      public slots:
	void runTests();

      signals:
	void testsFinished();

      private:
	int argc;
	char **argv;
};

#endif  // RUNNER_H
