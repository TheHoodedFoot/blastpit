#include "lmos.hpp"
#include "blastpit.h"

#include <QDir>
#include <QMetaMethod>
#include <QTemporaryFile>

Lmos::Lmos(QObject* parent) : QObject(parent)
{
#if defined(Q_OS_WIN32)
	connect(&lmos_actx, SIGNAL(MessageMap(QString&)), this,
		SLOT(messagemap(QString&)),
		Qt::ConnectionType::DirectConnection);
	connect(&lmos_actx, SIGNAL(ALARM(int, QString, QString, int)), this,
		SLOT(alarm(int, QString, QString, int)));
	connect(&lmos_actx, SIGNAL(ImageBegin()), this, SLOT(imagebegin()));
	connect(&lmos_actx, SIGNAL(ImageEnd()), this, SLOT(imageend()));
	connect(&lmos_actx, SIGNAL(ImageEnd2(double, ImageResultConstants)),
		this, SLOT(imageEnd2(double, ImageResultConstants)));
	connect(&lmos_actx, SIGNAL(CurrentChanged(double)), this,
		SLOT(currentChanged(double)));
	connect(&lmos_actx, SIGNAL(FrequencyChanged(int)), this,
		SLOT(frequencyChanged(int)));
	connect(&lmos_actx, SIGNAL(JobBegin()), this, SLOT(jobBegin()));
	connect(&lmos_actx, SIGNAL(JobEnd()), this, SLOT(jobEnd()));
	connect(&lmos_actx, SIGNAL(MOBeginName(QString&)), this,
		SLOT(mOBeginName(QString&)));
	connect(&lmos_actx, SIGNAL(MOEndName(QString&)), this,
		SLOT(mOEndName(QString&)));
	connect(&lmos_actx, SIGNAL(PLCEvent(QString&, QString&, QString&)),
		this, SLOT(plcEvent(QString&, QString&, QString&))),
		connect(&lmos_actx,
			SIGNAL(exception(int, QString, QString, QString)),
			this,
			SLOT(exception(int, const QString, const QString,
				       const QString)));
	connect(&lmos_actx, SIGNAL(signal(const QString&, int, void*)), this,
		SLOT(logstring(const QString&, int, void*)));
	// connect(&lmos_actx, SIGNAL(MouseDown(int, int, double, double)),
	// this, SLOT(emitSig(QString("mouse"))));
	lmos_actx.show();
#endif
}

Lmos::~Lmos()
{
#if defined(Q_OS_WIN32)
	lmos_actx.TermMachine();
#endif
}

QPixmap
Lmos::GrabWindow()
{
#if defined(Q_OS_WIN32)
	return lmos_actx.grab();
#else
	QPixmap* ptr_pixmap = new QPixmap(256, 256);
	ptr_pixmap->fill("#777777");
	QPixmap pixmap = *ptr_pixmap;
	delete ptr_pixmap;
	return pixmap;
#endif
}

bool
Lmos::InitMachine()
{
#if defined(Q_OS_WIN32)
	try {
		return lmos_actx.InitMachine();
	} catch (...) {
		emit log(
			"bplInitMachine: An exception occurred initialising "
			"the laser.");
	}
#endif
	return false;
}

void
Lmos::SaveVLM(const QString& filename)
{
#ifdef QT_DEBUG
	emit log(kDebug, __func__, "Debug mode: not saving " + filename);
	return;
#endif
	emit log(kDebug, __func__, "Saving vlm as " + filename);
	if (filename.length() > 0) {
#if defined(Q_OS_WIN32)
		emit retval(__func__, lmos_actx.Save(filename));
#endif
	}
}

void
Lmos::LoadVLM(const QString& filename)
{
	QString vlm = QDir::toNativeSeparators(filename);
	emit log(kDebug, __func__, "Loading " + vlm);
#if defined(Q_OS_WIN32)
	lmos_actx.FileName2(vlm);
#endif
}

bool
Lmos::LoadXML(const QString& xml)
{
#if defined(Q_OS_WIN32)
	bool result = lmos_actx.LoadXML(xml);
	lmos_actx.ShowMarkingArea();
	return result;
#else
	(void)xml;
	return false;
#endif
}

void
Lmos::ClearQPSets()
{
#ifdef QT_DEBUG
	emit log(kDebug, __func__, "Debug mode: not Clearing QP Sets");
	return;
#endif
	emit log(kDebug, __func__, "Clearing QP Sets");
#if defined(Q_OS_WIN32)
	QVariant qpNames = lmos_actx.GetGlobalQPSetNames();
	QStringList qpNamesList = qpNames.toStringList();
	QString qpName;
	for (int i = 0; i < qpNamesList.count(); i++) {
		if (!QString::compare("bp_", qpNamesList[i].left(3),
				      Qt::CaseInsensitive)) {
			emit log(kDebug, __func__,
				 "Deleting " + qpNamesList[i]);
			lmos_actx.RemoveGlobalQPSet(qpNamesList[i]);
		}
		// lmos_actx.SaveGlobalQPSets();
	}
#endif
	emit log(kDebug, __func__, "Finished clearing QP Sets");
}
void
Lmos::ImportXML(QTemporaryFile* XMLFile)
{
	// Qt may be happy with path separators, but LMOS isn't
	QString winXML = QDir::toNativeSeparators(XMLFile->fileName());
	emit log(kInfo, __func__, "bplImportXML: Importing " + winXML);

	// LMOS fails to import the file if it is still owned by us,
	// so we have to close it, read it then delete it.
	XMLFile->setAutoRemove(false);
	delete XMLFile;
#if defined(Q_OS_WIN32)
	lmos_actx.ActivateZoomWindow(false);
	lmos_actx.ImportXMLFile2(winXML);
	lmos_actx.ShowMarkingArea();
	// lmos_actx.ShowWholeDrawing();
#endif
	QFile::remove(winXML);
}

void
Lmos::ShowMarkingArea()
{
#if defined(Q_OS_WIN32)
	lmos_actx.ActivateZoomWindow(false);
	lmos_actx.ShowMarkingArea();
#endif
}

void
Lmos::StopPosHelp()
{
	// stopPosHelp: Disable the positioning help
	emit log(kDebug, __func__, "Cancelling positioning help...");
#if defined(Q_OS_WIN32)
	lmos_actx.StopPosHelp();
#endif
}
void
Lmos::TermMachine()
{
	// termMachine: Shuts down the laser system
	emit log(kDebug, __func__, "Calling lmos.TermMachine()");
#if defined(Q_OS_WIN32)
	emit retval(__func__, lmos_actx.TermMachine());
#endif
}

bool
Lmos::StartMarking()
{
	// startMarking: Begin the marking operation
	emit log(kDebug, __func__, "Calling lmos.StartMarking()");
#if defined(Q_OS_WIN32)
	if (lmos_actx.LoadJob()) {
		return lmos_actx.StartMarking();
	} else {
		emit log(kDebug, __func__, "lmos.LoadJob() failed");
	}
#else
	// this->sigImageEnd2(
	// 999,
	// 0); /* When testing in linux, always return 0 (success) */
	return true;
#endif
	return false;
}

void
Lmos::StopMarking()
{
	// stopMarking: End the marking operation
	emit log(kDebug, __func__, "Calling lmos.StopMarking()");
#if defined(Q_OS_WIN32)
	bool stop = lmos_actx.StopMarking();
	bool cancel = lmos_actx.CancelJob();
	emit retval(__func__, stop && cancel);
#endif
}

void
Lmos::MoveZ(const float z)
{
	// move: Position the Z axis
	emit log(kDebug, __func__, "Moving Z axis...");
#if defined(Q_OS_WIN32)
	LMOSACTXLib::Axis myaxis(lmos_actx.Axis());
	myaxis.NewPos(LMOSACTXLib::Z_AXIS, static_cast<double>(z), true);
	myaxis.MoveAxes();
#else
	(void)z;
#endif
}

void
Lmos::MoveW(const double w)
{
	emit log(kDebug, __func__, "Moving W axis...");
#if defined(Q_OS_WIN32)
	LMOSACTXLib::Axis myaxis(lmos_actx.Axis());
	myaxis.NewPos(LMOSACTXLib::W_AXIS, w, true);
	myaxis.MoveAxes();
#else
	(void)w;
#endif
}
void
Lmos::Reference()
{
	emit log(kDebug, __func__, "Referencing Z and W axes...");
#if defined(Q_OS_WIN32)
	LMOSACTXLib::Axis myaxis(lmos_actx.Axis());
	myaxis.NewReference(LMOSACTXLib::Z_AXIS);
	myaxis.ReferenceAxes();
	myaxis.NewReference(LMOSACTXLib::W_AXIS);
	myaxis.ReferenceAxes();
#endif
}

void
Lmos::AddQPSet(const QString& name, const double current, const int speed,
	       const int frequency)
{
	emit log(kDebug, __func__,
		 "Name: " + name + ", Current: " + QString::number(current) +
			 ", Speed: " + QString::number(speed) +
			 ", Frequency: " + QString::number(frequency));
#if defined(Q_OS_WIN32)
	lmos_actx.AddGlobalQPSet(name, current, speed, frequency);
#else
	(void)name;
	(void)frequency;
	(void)current;
	(void)speed;
#endif
}

void
Lmos::SaveQPSets()
{
#if defined(Q_OS_WIN32)
	lmos_actx.SaveGlobalQPSets();
#endif
}

void
Lmos::LayerSetHeightZAxis(const QString& layer, const float height)
{
	/* emit log(kDebug, __func__, "Debug mode: layer set height z axis");
	 */
#if defined(Q_OS_WIN32)
	LMOSACTXLib::Layers moLayers(lmos_actx.Layers());
	try {
		// int a = moLayers.Count();
		// qDebug() << "Layer count: " +
		// QString::number(moLayers.Count());
		LMOSACTXLib::ILayer* mylayer = moLayers.Item(layer);
		if (mylayer) {
			/* qDebug() << __func__ */
			/* 	 << "Layer " + layer + " height is now " + */
			/* 		    QString::number( */
			/* 			    mylayer->HeightZAxis());
			 */
			mylayer->SetHeightZAxis(height);
			/* qDebug() << __func__ */
			/* 	 << "Layer " + layer + " height set to " + */
			/* 		    QString::number( */
			/* 			    mylayer->HeightZAxis());
			 */
		} else {
			qDebug() << __func__ << "Null layer pointer!";
			return;
		}
	} catch (...) {
		qDebug() << "EXCEPTION: Setting layer height of " +
				    QString::number(height) +
				    " for layer named " + layer + " failed.";
	}
#else
	(void)layer;
	(void)height;
#endif
}

void
Lmos::LayerSetLaserable(const QString& layer, const bool is_laserable)
{
#if defined(Q_OS_WIN32)
	if (layer.length()) {
		LMOSACTXLib::Layers moLayers(lmos_actx.Layers());
		if (moLayers.Count()) {
			if (!moLayers.Item(layer)->isNull()) {
				moLayers.Item(layer)->SetLaserable(
					is_laserable);
			} else {
				qDebug() << __func__ << "Layer is null";
			}
		} else {
			qDebug() << "Layer count: " +
					    QString::number(moLayers.Count());
		}
	}
#else
	(void)layer;
	(void)is_laserable;
	qInfo() << "Setting layer " << layer
		<< " laserable: " << is_laserable;
#endif
}

void
Lmos::LayerSetVisible(const QString& layer, const bool is_visible)
{
	emit log(kDebug, __func__,
		 "Layer: " + layer +
			 ", is_visible: " + QString::number(is_visible));
#if defined(Q_OS_WIN32)
	if (layer.length()) {
		LMOSACTXLib::Layers moLayers(lmos_actx.Layers());
		if (moLayers.Count()) {
			moLayers.Item(layer)->SetVisible(is_visible);
			moLayers.Item(layer)->SetExportable(is_visible);
			lmos_actx.RedrawLayout();
		} else {
			qDebug() << __func__ << "Error: layer count is zero";
		}
	}
#else
	(void)layer;
	(void)is_visible;
#endif
}

void
Lmos::LayerSetExportable(const QString& layer, const bool is_exportable)
{
	emit log(kDebug, __func__,
		 "Layer: " + layer + ", is_exportable: " +
			 QString::number(is_exportable));
#if defined(Q_OS_WIN32)
	if (layer.length()) {
		LMOSACTXLib::Layers moLayers(lmos_actx.Layers());
		if (moLayers.Count()) {
			moLayers.Item(layer)->SetExportable(is_exportable);
			lmos_actx.RedrawLayout();
		} else {
			qDebug() << __func__ << "Error: layer count is zero";
		}
	}
#else
	(void)layer;
	(void)is_exportable;
#endif
}

void
Lmos::ReadByte(const int port, const int mask)
{
#if defined(Q_OS_WIN32)
	int byte;
	emit log(kDebug, __func__, "port: " + QString::number(port));
	emit log(kDebug, __func__, "mask: " + QString::number(mask));
	emit retval(__func__,
		    lmos_actx.ReadByte(
			    static_cast<LMOSACTXLib::InPortConstants>(port),
			    mask, byte));
	emit retval(__func__, byte);
#else
	(void)port;
	(void)mask;
#endif
}

void
Lmos::ReadIOBit(const QString& bitfunction)
{
#if defined(Q_OS_WIN32)
	QVariant res;
	emit retval(__func__, lmos_actx.ReadIOBit(bitfunction, res));
	if (res.canConvert<bool>()) {
		bool result = res.Bool;
		if (result) {
			emit log(kDebug, __func__, "Result: True");
		} else {
			emit log(kDebug, __func__, "Result: False");
		}
	}
#else
	(void)bitfunction;
#endif
}

void
Lmos::SetDimension(const QString& object, const double width,
		   const double height)
{
	emit log(kDebug, __func__, "object: " + object);
	emit log(kDebug, __func__, "width: " + QString::number(width));
	emit log(kDebug, __func__, "height: " + QString::number(height));
#if defined(Q_OS_WIN32)
	emit retval(__func__, lmos_actx.SetDimension(object, width, height));
#endif
}

void
Lmos::SetMOLayer(const QString& object, const QString& layer)
{
	emit log(kDebug, __func__, "object: " + object);
	emit log(kDebug, __func__, "layer: " + layer);
#if defined(Q_OS_WIN32)
	emit retval(__func__, lmos_actx.SetMOLayer(object, layer));
#endif
}

void
Lmos::SetQualityParam(const QString& qpname, const int qptype,
		      const int partype, const QVariant& value,
		      const bool save)
{
#if defined(Q_OS_WIN32)
	emit retval(__func__,
		    lmos_actx.SetQualityParam(
			    qpname, static_cast<LMOSACTXLib::eQPType>(qptype),
			    static_cast<LMOSACTXLib::eParamType>(partype),
			    value, save));
#else
	(void)qpname;
	(void)qptype;
	(void)partype;
	(void)value;
	(void)save;
#endif
}

void
Lmos::WriteByte(const int outport, const char mask, const char data)
{
#if defined(Q_OS_WIN32)
	emit retval(
		__func__,
		lmos_actx.WriteByte(
			static_cast<LMOSACTXLib::OutPortConstants>(outport),
			mask, data));
#else
	(void)outport;
	(void)mask;
	(void)data;
#endif
}

void
Lmos::WriteIOBit(const QString& bitfunction, const bool value)
{
	emit log(kDebug, __func__, "bitfunction: " + bitfunction);
	emit log(kDebug, __func__, "value: " + QString::number(value));
#if defined(Q_OS_WIN32)
	emit retval(__func__, lmos_actx.WriteIOBit(bitfunction, value));
#endif
}

void
Lmos::SetPosValues(const QString& object, const double x, const double y,
		   const double angle)
{
	emit log(kDebug, __func__, "object: " + object);
	emit log(kDebug, __func__, "x: " + QString::number(x));
	emit log(kDebug, __func__, "y: " + QString::number(y));
	emit log(kDebug, __func__, "angle: " + QString::number(angle));
#if defined(Q_OS_WIN32)
	emit retval(__func__, lmos_actx.SetPosValues(object, x, y, angle));
#endif
}

void
Lmos::StartPosHelp(const QString& object)
{
	emit log(kDebug, __func__, "object: " + object);
#if defined(Q_OS_WIN32)
	lmos_actx.StartPosHelp(object);
#endif
}

void
Lmos::CancelJob()
{
	emit log(kDebug, __func__, "Cancelling job");
#if defined(Q_OS_WIN32)
	emit retval(__func__, lmos_actx.CancelJob());
	// emit retval(__func__, lmos_actx.ClearLayout());
#endif
}

void
Lmos::logstring(const QString& name, int argc, void* argv)
{
	(void)argc;
	(void)argv;

	emit log(QString("(Event) " + name));
}

void
Lmos::messagemap(QString& name)
{
	QString bob = name;
	emit log(QString("(MessageMap) " + bob));
}

void
Lmos::imagebegin()
{
	emit log(kDebug, __func__, "imagebegin signal issued");
}

void
Lmos::imageend()
{
	emit log(kDebug, __func__, "imageend2 signal issued");
}

void
Lmos::currentChanged(double)
{
	emit log(kDebug, __func__, "signal issued");
}

void
Lmos::jobBegin()
{
	emit log(kDebug, __func__, "signal issued");
}

void
Lmos::jobEnd()
{
	emit log(kDebug, __func__, "signal issued");
}

#if defined(Q_OS_WIN32)
void
Lmos::imageEnd2(double time, ImageResultConstants result)
{
	emit log(kDebug, __func__, "signal issued");
	emit sigImageEnd2(time, static_cast<int>(result));
}
#endif

void
Lmos::frequencyChanged(int a)
{
	emit log(kDebug, __func__, "signal issued");
	emit log(kDebug, __func__, "param1: " + QString::number(a));
}

void
Lmos::mOBeginName(QString& a)
{
	emit log(kDebug, __func__, "signal issued");
	emit log(kDebug, __func__, "param1: " + a);
}

void
Lmos::mOEndName(QString& a)
{
	emit log(kDebug, __func__, "signal issued");
	emit log(kDebug, __func__, "param1: " + a);
}

void
Lmos::plcEvent(QString&, QString&, QString&)
{
	emit log(kDebug, __func__, "signal issued");
}
void
Lmos::alarm(int alarmNum, QString description, QString moName, int moID)
{
	emit log(QString("(Alarm) " + QString::number(alarmNum) + ", " +
			 description + ", " + moName + ", " +
			 QString::number(moID)));
}

void
Lmos::exception(int alarmNum, const QString& description,
		const QString& moName, const QString& moID)
{
	emit log(QString("(exception) " + QString::number(alarmNum) + ", " +
			 description + ", " + moName + ", " + moID));
}

void
Lmos::ZoomWindow(int x1, int y1, int x2, int y2)
{
	emit log(QString(__func__) + QString("Begin: ") +
		 QString::number(x1) + QString(", ") + QString::number(y1) +
		 QString(", ") + QString::number(x2) + QString(", ") +
		 QString::number(y2));
#if defined(Q_OS_WIN32)
	// lmos_actx.ShowZoomWindow(50, 50, 70, 70);
	lmos_actx.ActivateZoomWindow(true);
	lmos_actx.ShowZoomWindow(x1, y1, x2, y2);
	// lmos_actx.ActivateZoomWindow(false);
	// lmos_actx.ShowMarkingAreaZoom();
	// lmos_actx.RedrawLayout();
#endif
}

void
Lmos::Test()
{
	emit log(kInfo, __func__, "Beginning LMOS::Test");

#if defined(Q_OS_WIN32)

	// List signals
	QMetaObject metaObject = lmos_actx.staticMetaObject;
	for (int i = 0; i < metaObject.methodCount(); i++) {
		QMetaMethod method = metaObject.method(i);
		if (method.methodType() == QMetaMethod::Signal) {
			log(kDebug, __func__,
			    "Signal: " + method.methodSignature());
		}
		if (method.methodType() == QMetaMethod::Method) {
			log(kDebug, __func__,
			    "Method: " + method.methodSignature());
		}
	}

#endif
}

void
Lmos::emitSig(QString sig)
{
	emit sendEvent(sig);
}

// vi: fdm=syntax foldlevel=0
