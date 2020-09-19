#include "lmos.hpp"
#include "../libblastpit/blastpit.h"

#include <QDir>
#include <QMetaMethod>
#include <QTemporaryFile>

Lmos::Lmos(QObject* parent) : QObject(parent)
{
#if defined(Q_OS_WIN32)
	lmos_actx = NULL;
#endif
}

Lmos::~Lmos() { DestroyControl(); }

void
Lmos::CreateControl()
{  // Dyamically creates the activex control

#if defined(Q_OS_WIN32)
	if (lmos_actx)
		return;
	lmos_actx = new (LMOSACTXLib::LMOSActX);
#endif
}

void
Lmos::DestroyControl()
{  // Destroy the activex control

#if defined(Q_OS_WIN32)
	if (lmos_actx) {
		delete lmos_actx;
		lmos_actx = NULL;
	}
#endif
}

void
Lmos::ConnectSignals()
{  // Connect up Qt signals and slots

#if defined(Q_OS_WIN32)
	if (!lmos_actx)
		return;
	connect(lmos_actx, SIGNAL(MessageMap(QString&)), this, SLOT(messagemap(QString&)),
		Qt::ConnectionType::DirectConnection);
	connect(lmos_actx, SIGNAL(ALARM(int, const QString, const QString, int)), this,
		SLOT(alarm(int, QString, QString, int)));
	connect(lmos_actx, SIGNAL(CurrentChanged(const double)), this, SLOT(currentChanged(double)));
	connect(lmos_actx, SIGNAL(FrequencyChanged(const int)), this, SLOT(frequencyChanged(int)));
	connect(lmos_actx, SIGNAL(JobBegin()), this, SLOT(jobBegin()));
	connect(lmos_actx, SIGNAL(JobEnd()), this, SLOT(jobEnd()));
	connect(lmos_actx, SIGNAL(PLCEvent(QString&, QString&, QString&)), this,
		SLOT(plcEvent(QString&, QString&, QString&)));
	connect(lmos_actx, SIGNAL(exception(const int, const QString, const QString, const QString)), this,
		SLOT(exception(int, const QString, const QString, const QString)));

	connect(lmos_actx, SIGNAL(signal(const QString&, int, void*)), this, SLOT(emitSig(const QString&)));

	/* Working */
	connect(lmos_actx, SIGNAL(ImageEnd()), this, SLOT(imageend()));
	connect(lmos_actx, SIGNAL(ImageBegin()), this, SLOT(imagebegin()));
	connect(lmos_actx, SIGNAL(MOBeginName(QString&)), this, SLOT(mOBeginName(QString&)));
	connect(lmos_actx, SIGNAL(MOEndName(QString&)), this, SLOT(mOEndName(QString&)));
	connect(lmos_actx, SIGNAL(ImageEnd2(const double, const ImageResultConstants)), this,
		SLOT(imageEnd2(double, ImageResultConstants)));
#endif
}

void
Lmos::DisconnectSignals()
{  // Disconnect Qt signals and slots

#if defined(Q_OS_WIN32)
	if (!lmos_actx)
		return;

	disconnect(lmos_actx, 0, 0, 0);
#endif
}

void
Lmos::ShowWindow()
{  // Makes the lmos window visible

#if defined(Q_OS_WIN32)
	if (!lmos_actx)
		return;
	lmos_actx->show();
#endif
}

void
Lmos::HideWindow()
{  // Hides the lmos window

#if defined(Q_OS_WIN32)
	if (!lmos_actx)
		return;
	lmos_actx->hide();
#endif
}

QPixmap
Lmos::GrabWindow()
{
#if defined(Q_OS_WIN32)
	return lmos_actx->grab();
#else
	QPixmap* ptr_pixmap = new QPixmap(256, 256);
	/* ptr_pixmap->fill("#777777"); */
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
		return lmos_actx->InitMachine();
	} catch (...) {
		emit log(
			"bplInitMachine: An exception occurred initialising "
			"the laser.");
	}
#endif
	return false;
}

bool
Lmos::SaveVLM(const QString& filename)
{
	emit log(kLvlDebug, __func__, "Saving vlm as " + filename);
	if (filename.length() > 0) {
#if defined(Q_OS_WIN32)
		return lmos_actx->Save(filename);
#else
		return false;
#endif
	}

	return false;
}

void
Lmos::LoadVLM(const QString& filename)
{
	QString vlm = QDir::toNativeSeparators(filename);
	emit log(kLvlDebug, __func__, "Loading " + vlm);
#if defined(Q_OS_WIN32)
	lmos_actx->FileName2(vlm);
#endif
}

bool
Lmos::LoadXML(const QString& xml)
{
#if defined(Q_OS_WIN32)
	/* We must force a clear here, since Lmos crashes if a previous layout
	 * exists */
	lmos_actx->CancelJob();
	lmos_actx->ClearLayout();

	bool result = lmos_actx->LoadXML(xml);
	lmos_actx->ShowMarkingArea();
	return result;
#else
	emit log(kLvlDebug, __func__, xml);
	return false;
#endif
}

void
Lmos::ClearQPSets()
{
	/*#ifdef QT_DEBUG
		emit log(kLvlDebug, __func__, "Debug mode: not Clearing QP
	Sets"); return; #endif
	*/
	emit log(kLvlDebug, __func__, "Clearing QP Sets");
#if defined(Q_OS_WIN32)
	QVariant qpNames = lmos_actx->GetGlobalQPSetNames();
	QStringList qpNamesList = qpNames.toStringList();
	QString qpName;
	for (int i = 0; i < qpNamesList.count(); i++) {
		if (!QString::compare("bp_", qpNamesList[i].left(3), Qt::CaseInsensitive)) {
			emit log(kLvlDebug, __func__, "Deleting " + qpNamesList[i]);
			lmos_actx->RemoveGlobalQPSet(qpNamesList[i]);
		}
		// lmos_actx->SaveGlobalQPSets();
	}
#endif
	emit log(kLvlDebug, __func__, "Finished clearing QP Sets");
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
	lmos_actx->ActivateZoomWindow(false);
	lmos_actx->ImportXMLFile2(winXML);
	lmos_actx->ShowMarkingArea();
	// lmos_actx->ShowWholeDrawing();
#endif
	QFile::remove(winXML);
}

void
Lmos::ShowMarkingArea()
{
#if defined(Q_OS_WIN32)
	lmos_actx->ActivateZoomWindow(false);
	lmos_actx->ShowMarkingArea();
#endif
}

void
Lmos::StopPosHelp()
{
	// stopPosHelp: Disable the positioning help
	emit log(kLvlDebug, __func__, "Stop positioning help");
#if defined(Q_OS_WIN32)
	lmos_actx->StopPosHelp();
#endif
}

void
Lmos::StartPosHelp(const QString& object)
{
	emit log(kLvlDebug, __func__, "object: " + object);
#if defined(Q_OS_WIN32)
	lmos_actx->StartPosHelp(object);
#endif
}

void
Lmos::TermMachine()
{
	// termMachine: Shuts down the laser system
	emit log(kLvlDebug, __func__, "Calling lmos.TermMachine()");
#if defined(Q_OS_WIN32)
	emit retval(__func__, lmos_actx->TermMachine());
#endif
}

bool
Lmos::StartMarking()
{
	// startMarking: Begin the marking operation
	emit log(kLvlDebug, __func__, "Calling lmos.StartMarking()");
#if defined(Q_OS_WIN32)
	return lmos_actx->StartMarking();
#endif
	return false;
}

bool
Lmos::LoadJob()
{
	emit log(kLvlDebug, __func__, "Calling loadjob");
#if defined(Q_OS_WIN32)
	bool loadjob = lmos_actx->LoadJob();
	emit retval(__func__, loadjob);
	return loadjob;
#endif
	return false;
}
void
Lmos::StopMarking()
{
	// stopMarking: End the marking operation
	emit log(kLvlDebug, __func__, "Calling lmos.StopMarking()");
#if defined(Q_OS_WIN32)
	bool stop = lmos_actx->StopMarking();
	bool cancel = lmos_actx->CancelJob();
	emit retval(__func__, stop && cancel);
#endif
}

void
Lmos::MoveZ(const float z)
{
	// move: Position the Z axis
	emit log(kLvlDebug, __func__, "Moving Z axis...");
#if defined(Q_OS_WIN32)
	LMOSACTXLib::Axis myaxis(lmos_actx->Axis());
	myaxis.NewPos(LMOSACTXLib::Z_AXIS, static_cast<double>(z), true);
	myaxis.MoveAxes();
#else
	(void)z;
#endif
}

void
Lmos::MoveW(const double w)
{
	emit log(kLvlDebug, __func__, "Moving W axis...");
#if defined(Q_OS_WIN32)
	LMOSACTXLib::Axis myaxis(lmos_actx->Axis());
	myaxis.NewPos(LMOSACTXLib::W_AXIS, w, true);
	myaxis.MoveAxes();
#else
	(void)w;
#endif
}
void
Lmos::Reference()
{
	emit log(kLvlDebug, __func__, "Referencing Z and W axes...");
#if defined(Q_OS_WIN32)
	LMOSACTXLib::Axis myaxis(lmos_actx->Axis());
	myaxis.NewReference(LMOSACTXLib::Z_AXIS);
	myaxis.ReferenceAxes();
	myaxis.NewReference(LMOSACTXLib::W_AXIS);
	myaxis.ReferenceAxes();
#endif
}

void
Lmos::AddQPSet(const QString& name, const double current, const int speed, const int frequency)
{
	emit log(kLvlDebug, __func__,
		 "Name: " + name + ", Current: " + QString::number(current) + ", Speed: " + QString::number(speed) +
			 ", Frequency: " + QString::number(frequency));
#if defined(Q_OS_WIN32)
	lmos_actx->AddGlobalQPSet(name, current, speed, frequency);
#endif
}

void
Lmos::SaveQPSets()
{
#if defined(Q_OS_WIN32)
	lmos_actx->SaveGlobalQPSets();
#endif
}

void
Lmos::LayerSetHeightZAxis(const QString& layer, const float height)
{
	/* emit log(kLvlDebug, __func__, "Debug mode: layer set height z axis");
	 */
#if defined(Q_OS_WIN32)
	LMOSACTXLib::Layers moLayers(lmos_actx->Layers());
	try {
		// int a = moLayers.Count();
		// qDebug() << "Layer count: " +
		// QString::number(moLayers.Count());
		LMOSACTXLib::ILayer* mylayer = moLayers.Item(layer);
		if (mylayer) {
			/* qDebug() << __func__ */
			/*	 << "Layer " + layer + " height is now " + */
			/*			QString::number( */
			/*				mylayer->HeightZAxis());
			 */
			mylayer->SetHeightZAxis(height);
			/* qDebug() << __func__ */
			/*	 << "Layer " + layer + " height set to " + */
			/*			QString::number( */
			/*				mylayer->HeightZAxis());
			 */
		} else {
			qDebug() << __func__ << "Null layer pointer!";
			return;
		}
	} catch (...) {
		qDebug() << "EXCEPTION: Setting layer height of " + QString::number(height) + " for layer named " +
				    layer + " failed.";
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
		LMOSACTXLib::Layers moLayers(lmos_actx->Layers());
		if (moLayers.Count()) {
			if (!moLayers.Item(layer)->isNull()) {
				moLayers.Item(layer)->SetLaserable(is_laserable);
			} else {
				qDebug() << __func__ << "Layer is null";
			}
		} else {
			qDebug() << "Layer count: " + QString::number(moLayers.Count());
		}
	}
#else
	(void)layer;
	(void)is_laserable;
	qInfo() << "Setting layer " << layer << " laserable: " << is_laserable;
#endif
}

void
Lmos::LayerSetVisible(const QString& layer, const bool is_visible)
{
	emit log(kLvlDebug, __func__, "Layer: " + layer + ", is_visible: " + QString::number(is_visible));
#if defined(Q_OS_WIN32)
	if (layer.length()) {
		LMOSACTXLib::Layers moLayers(lmos_actx->Layers());
		if (moLayers.Count()) {
			moLayers.Item(layer)->SetVisible(is_visible);
			moLayers.Item(layer)->SetExportable(is_visible);
			lmos_actx->RedrawLayout();
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
	emit log(kLvlDebug, __func__, "Layer: " + layer + ", is_exportable: " + QString::number(is_exportable));
#if defined(Q_OS_WIN32)
	if (layer.length()) {
		LMOSACTXLib::Layers moLayers(lmos_actx->Layers());
		if (moLayers.Count()) {
			moLayers.Item(layer)->SetExportable(is_exportable);
			lmos_actx->RedrawLayout();
		} else {
			qDebug() << __func__ << "Error: layer count is zero";
		}
	}
#else
	(void)layer;
	(void)is_exportable;
#endif
}

int
Lmos::ReadByte(const int port, const int mask)
{
#if defined(Q_OS_WIN32)
	int byte;
	emit log(kLvlDebug, __func__, "port: " + QString::number(port));
	emit log(kLvlDebug, __func__, "mask: " + QString::number(mask));
	return lmos_actx->ReadByte(static_cast<LMOSACTXLib::InPortConstants>(port), mask, byte);
#else
	(void)port;
	(void)mask;
	return kInvalid;
#endif
}

int
Lmos::ReadIOBit(const QString& bitfunction)
{
#if defined(Q_OS_WIN32)
	QVariant res;
	emit retval(__func__, lmos_actx->ReadIOBit(bitfunction, res));
	if (res.canConvert<bool>()) {
		bool result = res.Bool;
		if (result) {
			emit log(kLvlDebug, __func__, "Result: True");
			return true;
		} else {
			emit log(kLvlDebug, __func__, "Result: False");
			return false;
		}
	}
	return kInvalid;
#else
	(void)bitfunction;
	return kInvalid;
#endif
}

void
Lmos::SetDimension(const QString& object, const double width, const double height)
{
	emit log(kLvlDebug, __func__, "object: " + object);
	emit log(kLvlDebug, __func__, "width: " + QString::number(width));
	emit log(kLvlDebug, __func__, "height: " + QString::number(height));
#if defined(Q_OS_WIN32)
	emit retval(__func__, lmos_actx->SetDimension(object, width, height));
#endif
}

void
Lmos::SetMOLayer(const QString& object, const QString& layer)
{
	emit log(kLvlDebug, __func__, "object: " + object);
	emit log(kLvlDebug, __func__, "layer: " + layer);
#if defined(Q_OS_WIN32)
	emit retval(__func__, lmos_actx->SetMOLayer(object, layer));
#endif
}

void
Lmos::SetQualityParam(const QString& qpname, const int qptype, const int partype, const QVariant& value,
		      const bool save)
{
#if defined(Q_OS_WIN32)
	emit retval(__func__, lmos_actx->SetQualityParam(qpname, static_cast<LMOSACTXLib::eQPType>(qptype),
							 static_cast<LMOSACTXLib::eParamType>(partype), value, save));
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
	emit retval(__func__, lmos_actx->WriteByte(static_cast<LMOSACTXLib::OutPortConstants>(outport), mask, data));
#else
	(void)outport;
	(void)mask;
	(void)data;
#endif
}

void
Lmos::WriteIOBit(const QString& bitfunction, const bool value)
{
	emit log(kLvlDebug, __func__, "bitfunction: " + bitfunction);
	emit log(kLvlDebug, __func__, "value: " + QString::number(value));
#if defined(Q_OS_WIN32)
	QVariant bf = bitfunction;
	QVariant val = value;
	emit retval(__func__, lmos_actx->WriteIOBit(bf, value));
	// emit retval(__func__, lmos_actx->WriteIOBit(bitfunction, value));
#endif
}

void
Lmos::SetPosValues(const QString& object, const double x, const double y, const double angle)
{
	emit log(kLvlDebug, __func__, "object: " + object);
	emit log(kLvlDebug, __func__, "x: " + QString::number(x));
	emit log(kLvlDebug, __func__, "y: " + QString::number(y));
	emit log(kLvlDebug, __func__, "angle: " + QString::number(angle));
#if defined(Q_OS_WIN32)
	emit retval(__func__, lmos_actx->SetPosValues(object, x, y, angle));
#endif
}

int
Lmos::CancelJob()
{
#if defined(Q_OS_WIN32)
	emit log(kLvlDebug, __func__, "Cancelling job...");
	int cancelret = lmos_actx->CancelJob();
	emit retval(__func__, cancelret);
	emit log(kLvlDebug, __func__, "Clearing layout...");
	int clearret = lmos_actx->ClearLayout();
	emit retval(__func__, clearret);
	return cancelret || clearret;
#else
	return false;
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
Lmos::ZoomWindow(int x1, int y1, int x2, int y2)
{
	emit log(QString(__func__) + QString("Begin: ") + QString::number(x1) + QString(", ") + QString::number(y1) +
		 QString(", ") + QString::number(x2) + QString(", ") + QString::number(y2));
#if defined(Q_OS_WIN32)
	// lmos_actx->ShowZoomWindow(50, 50, 70, 70);
	lmos_actx->ActivateZoomWindow(true);
	lmos_actx->ShowZoomWindow(x1, y1, x2, y2);
	// lmos_actx->ActivateZoomWindow(false);
	// lmos_actx->ShowMarkingAreaZoom();
	// lmos_actx->RedrawLayout();
#endif
}

void
Lmos::Test()
{ /* Test function used during development */

	/* Lmos::Test() is also used as an indication
	 * of the server being alive, so we emit a signal here */
	/* sendIdEvent("Beginning LMOS::Test", kTest); */
	emit log(kInfo, __func__, "Beginning LMOS::Test");

#if defined(Q_OS_WIN32)

	// // List signals
	// QMetaObject metaObject = lmos_actx->staticMetaObject;
	// for (int i = 0; i < metaObject.methodCount(); i++) {
	// 	QMetaMethod method = metaObject.method(i);
	// 	if (method.methodType() == QMetaMethod::Signal) {
	// 		log(kLvlDebug, __func__, "Signal: " + method.methodSignature());
	// 	}
	// 	if (method.methodType() == QMetaMethod::Method) {
	// 		log(kLvlDebug, __func__, "Method: " + method.methodSignature());
	// 	}
	// }

	// QVariant moNames = lmos_actx->GetMONames();
	// QStringList moNamesList = moNames.toStringList();
	// QString moName;
	// for (int i = 0; i < moNamesList.count(); i++) {
	// 	log(kLvlDebug, __func__, "Moname: " + moNamesList[i]);
	// }

	QVariant bf = (QString) "Light";
	QVariant val = (bool)true;

	// QVariant result;
	// bool light = lmos_actx->ReadIOBit(bf, result);
	// emit retval(__func__, light);

	// Disable under wine
	// int res = lmos_actx->WriteIOBit(bf, val);
	// emit retval(__func__, res);
	sendIdEvent("Testing!", kImageEnd2);

	// lmos_actx->dynamicCall("WriteIOBit(const QVariant&,const QVariant&)", bf, true);
	// lmos_actx->dynamicCall("WriteIOBit(QVariant&,QVariant&)", bf, val);
	// lmos_actx->dynamicCall("WriteIOBit(QVariant,QVariant)", bf, val);

#else
	log(kLvlDebug, __func__, "Running linux Lmos::Test()");
	currentChanged(0);
	frequencyChanged(0);
	imagebegin();
	imageend();
	QString test = "test";
	mOBeginName(test);
	mOEndName(test);
	jobBegin();
	jobEnd();
#endif
}

void
Lmos::messagemap(QString& name)
{
	emit log(kLvlDebug, __func__, name);
	sendIdEvent(name, kMessageMap);
}

void
Lmos::imagebegin()
{
	emit log(kLvlDebug, __func__, "");
	sendIdEvent("0", kImageBegin);
}

void
Lmos::imageend()
{
	emit log(kLvlDebug, __func__, "");
	sendIdEvent("0", kImageEnd);
}

void
Lmos::currentChanged(double current)
{
	emit log(kLvlDebug, __func__, QString::number(current));
	sendIdEvent(QString::number(current), kCurrentChanged);
}

void
Lmos::jobBegin()
{
	emit log(kLvlDebug, __func__, "");
	sendIdEvent("0", kJobBegin);
}

void
Lmos::jobEnd()
{
	emit log(kLvlDebug, __func__, "");
	sendIdEvent("0", kJobEnd);
}

#if defined(Q_OS_WIN32)
void
Lmos::imageEnd2(double time, ImageResultConstants result)
{
	emit log(kLvlDebug, __func__, "signal issued");
	emit sigImageEnd2(time, static_cast<int>(result));
	QString message = "imageEnd2: " + QString::number(result);
	sendIdEvent(message, kImageEnd2);
}
#endif

void
Lmos::frequencyChanged(int a)
{
	emit log(kLvlDebug, __func__, QString::number(a));
	sendIdEvent(QString::number(a), kFreqChanged);
}

void
Lmos::mOBeginName(QString& moName)
{ /* Return name of object before marking */

	emit log(kLvlDebug, __func__, moName);
	sendIdEvent(moName, kMoBegin);
}

void
Lmos::mOEndName(QString& moName)
{ /* Return name of object after marking complete */

	emit log(kLvlDebug, __func__, moName);
	sendIdEvent(moName, kMoEnd);
}

void
Lmos::plcEvent(QString& a, QString& b, QString& c)
{
	emit log(kLvlDebug, __func__, "signal issued");
	emit log(kLvlDebug, __func__, "param1: " + a);
	emit log(kLvlDebug, __func__, "param2: " + b);
	emit log(kLvlDebug, __func__, "param3: " + c);
	QString message = "plcEvent: " + a + ", " + b + ", " + c;
	sendIdEvent(message, kPlcEvent);
}

void
Lmos::alarm(int alarmNum, QString description, QString moName, int moID)
{
	QString message = QString("(Alarm) " + QString::number(alarmNum) + ", " + description + ", " + moName + ", " +
				  QString::number(moID));
	emit log(message);
	sendIdEvent(message, kAlarm);
}

void
Lmos::exception(int alarmNum, const QString& description, const QString& moName, const QString& moID)
{
	QString message =
		QString("(exception) " + QString::number(alarmNum) + ", " + description + ", " + moName + ", " + moID);
	emit log(message);
	sendIdEvent(message, kException);
}

void
Lmos::emitSig(const QString& sig)
{
	emit log(kLvlDebug, __func__, sig);
	sendIdEvent(sig, kSignal);
}

void
Lmos::sendIdEvent(QString string, int event)
{  // Helper function for sending an event

	// To distinguish events from other return values,
	// they are all negatives of the enums they represent.
	QString message = string;  // + QChar::Null;
	emit log(kLvlDebug, __func__, QString("sendIdEvent: (") + QString::number(event) + QString(") ") + message);
	emit sendEvent(-event, message);
}

void
Lmos::SetLaserable(QString object, int laserable)
{ /* Sets object to laserable or non laserable */

#if defined(Q_OS_WIN32)
	lmos_actx->SetLaserable(object, laserable);
#else
	(void)object;
	(void)laserable;
#endif
}

void
Lmos::UnsetLaserableAllObjects()
{ /* Sets all marking objects to non-laserable */

#if defined(Q_OS_WIN32)
	QVariant moNames = lmos_actx->GetMONames();
	QStringList moNamesList = moNames.toStringList();
	for (int i = 0; i < moNamesList.count(); i++) {
		SetLaserable(moNamesList[i], false);
	}
#endif
}

void
Lmos::SuppressRedraw(int redraw)
{ /* Enables or disables auto redraw of activex control */

#if defined(Q_OS_WIN32)
	lmos_actx->SetSuppressAutoRedraw(redraw);
#else
	(void)redraw;
#endif
}

void
Lmos::ForceRedraw()
{ /* Redraws the activex control */

#if defined(Q_OS_WIN32)
	lmos_actx->RedrawLayout();
#endif
}

// vi: fdm=syntax foldlevel=0
