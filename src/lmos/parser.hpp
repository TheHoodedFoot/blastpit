#ifndef PARSER_H
#define PARSER_H

#include <QObject>
#include "../libblastpit/blastpit.h"
#include "lmos.hpp"

#define MIN_SERVER_STRING_LEN 5
#define WS_RETRY_SECONDS      5

class Parser : public QObject {
	Q_OBJECT
      public:
	Parser(QObject *parent = nullptr);
	~Parser();
	static void messageReceivedCallback(void *ev_data, void *object);

      signals:
	void finished();
	void sendlog(QString);
	void settray(QString);
	void seticon(QString);
	void ClearLog();

      public slots:
	void log(QString);
	void ack(QString);
	void ackReturn(int id, int retval);
	void SendSignal(int signal, QString message);
	void log(int, const char *, QString);
	void ReplyWithPayload(int id, int retval, const char *payload);
	void wsConnect();

      private:
	void parseCommand(const char *message);
	void ProcessMessageBlock(const char *msg_data_string);

      private slots:
	void update();

      private:
	Lmos	    lmos;
	t_Blastpit *blast;
	QTimer *    timer;
	int	    mutex;
};

#endif	// PARSER_H
