#ifndef PARSER_H
#define PARSER_H

#include <QObject>
#include "lmos.h"
#include "network.h"

class Parser : public QObject {
	Q_OBJECT
      public:
	explicit Parser(QObject *parent = nullptr);
	Parser(QObject *parent = nullptr, int = LISTEN_PORT);
	void parseCommand(int);
	void listLog();

      signals:
	void finished();
	void sendlog(QString);

      public slots:
	int getPacket();
	void log(QString);
	void log(int, const char *, QString);
	void retval(const char *, int);
	void test();
	void imageEnd2(double, int);
	void receiveEvent(QString);

	void logWrite(uint32_t id, uint32_t command, uint32_t result);
	uint32_t logGetResult(uint32_t id);
	void clearLog();
	bool logEdit(uint32_t id, uint32_t result);

      private:
	void parseCommand(int, int, QByteArray);
	uint32_t getLastCommandOfType(uint32_t command);
	QString getXml(QByteArray &data, const char *attr);
	int getXmlInt(QByteArray &data, const char *attr);
	bool getXmlBool(QByteArray &data, const char *attr);
	float getXmlFloat(QByteArray &data, const char *attr);
	double getXmlDouble(QByteArray &data, const char *attr);

      public:
	Network network;

      private:
	Lmos lmos;

	std::vector<BpLogEntry> logfile;
};

#endif  // PARSER_H
