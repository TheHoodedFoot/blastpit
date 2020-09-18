#ifndef LIBLMOS_H
#define LIBLMOS_H

#include <QObject>
#include <QPixmap>
#include <QtCore>

#if defined(Q_OS_WIN32)
#include "lmosactxlib.h"
using namespace LMOSACTXLib;
#endif


class Lmos : public QObject {
	Q_OBJECT
      public:
	explicit Lmos(QObject* parent = nullptr);
	~Lmos();

      signals:
	void log(QString);
	void ack(QString);
	void log(int, const char*, QString);
	void sigMouseDown();
	void sendEvent(int, QString);
	void sigImageEnd2(double, int);
#if defined(Q_OS_WIN32)
	void retval(const char*, QVariant);
#endif
	void retval(const char*, bool);
	void retval(const char*, int);

      public slots:
	/* void Command(); // Issue a command to lmos */
	/* void Status(); // Get the current lmos status */

	void logstring(const QString& name, int argc, void* argv);
	void messagemap(QString& name);
	void alarm(int alarmNum, QString description, QString moName, int moID);
	void exception(int, const QString&, const QString&, const QString&);
	void plcEvent(QString&, QString&, QString&);
	void imagebegin();
	void imageend();
#if defined(Q_OS_WIN32)
	void imageEnd2(double, ImageResultConstants);
#endif
	void currentChanged(double);
	void frequencyChanged(int);
	void jobBegin();
	void jobEnd();
	void mOBeginName(QString&);
	void mOEndName(QString&);
	/* void mouseDown(int, int, double, double); */
	void emitSig(const QString&);

      public:
	QPixmap GrabWindow();
	bool InitMachine();
	bool LoadJob();
	bool LoadXML(const QString&);
	bool StartMarking();
	int CancelJob();
	void AddQPSet(const QString&, double, int, int);
	void ClearQPSets();
	void ImportXML();
	void ImportXML(QTemporaryFile*);
	void LayerSetExportable(const QString&, const bool);
	void LayerSetHeightZAxis(const QString&, float);
	void LayerSetLaserable(const QString&, const bool);
	void LayerSetVisible(const QString&, const bool);
	void LoadVLM(const QString&);
	void MoveW(double);
	void MoveZ(float);
	int ReadByte(const int, const int);
	int ReadIOBit(const QString&);
	void Reference();
	void SaveQPSets();
	bool SaveVLM(const QString&);
	void SetDimension(const QString&, const double, const double);
	void SetMOLayer(const QString&, const QString&);
	void SetPosValues(const QString&, const double, const double, const double);
	void SetQualityParam(const QString&, const int, const int, const QVariant&, const bool);
	void ShowMarkingArea();
	void StartPosHelp(const QString&);
	void StopMarking();
	void StopPosHelp();
	void TermMachine();
	void Test();
	void WriteByte(const int, const char, const char);
	void WriteIOBit(const QString&, const bool);
	void ZoomWindow(int, int, int, int);
	void SetLaserable(QString object, int laserable);
	void UnsetLaserableAllObjects();
	void SuppressRedraw(int redraw);
	void ForceRedraw();
	void CreateControl();
	void DestroyControl();
	void ConnectSignals();
	void DisconnectSignals();
	void ShowWindow();
	void HideWindow();


      private:
	bool Call(char const*) const;
	void sendIdEvent(QString string, int id);

#if defined(Q_OS_WIN32)
	LMOSACTXLib::LMOSActX* lmos_actx;
#endif
};
#endif	// LIBLMOS_H
