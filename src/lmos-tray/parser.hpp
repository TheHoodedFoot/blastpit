#ifndef PARSER_H
#define PARSER_H

#include <QObject>
#include "blastpit.h"
#include "lmos.hpp"
#include "pugixml.hpp"

class Parser : public QObject {
	Q_OBJECT
      public:
	Parser(QObject *parent = nullptr);
	~Parser();
	void parseCommand(int);
	void listLog();

      signals:
	void finished();
	void sendlog(QString);
	void settray(QString);

      public slots:
	void log(QString);
	void ack(QString);
	void ackReturn(int id, int retval);
	void log(int, const char *, QString);
    void receiveSignal(QString message);

      private:
	void parseCommand(int id, int command, pugi::xml_document &xml);

      private slots:
	void update();

      private:
	Lmos lmos;
	t_Blastpit *blast;
	QTimer *timer;
};

#endif  // PARSER_H
