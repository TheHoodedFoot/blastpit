#include <QThread>
#include <QUdpSocket>
#include <QtGui>

#include "blastpitgui.h"
#include "common.h"
#include "lpdialog.h"
#include "network.h"
#include "nulooq.h"
#include "pugixml.hpp"
#include "ui_mainwindow.h"

BlastpitGui::BlastpitGui(QWidget *parent)
	: QMainWindow(parent), ui(new Ui::MainWindow)
{
	ui->setupUi(this);
	setWindowFlags(Qt::Dialog | Qt::WindowStaysOnTopHint);
	CreateActions();
	InitTray();
	connect(trayIcon,
		SIGNAL(activated(QSystemTrayIcon::ActivationReason)), this,
		SLOT(IconActivated(QSystemTrayIcon::ActivationReason)));
	SetIcon();
	trayIcon->show();
	setWindowTitle(tr("Tentacle Engine"));

	IconActivated(QSystemTrayIcon::Trigger);

	Listen();
	NuLOOQ();

	return;
}

BlastpitGui::~BlastpitGui() { delete ui; }
void
BlastpitGui::CreateActions()
{
	minimizeAction = new QAction(tr("Mi&nimize"), this);
	connect(minimizeAction, SIGNAL(triggered()), this, SLOT(hide()));

	restoreAction = new QAction(tr("&Restore"), this);
	connect(restoreAction, SIGNAL(triggered()), this, SLOT(showNormal()));

	testAction = new QAction(tr("&Test"), this);
	connect(testAction, SIGNAL(triggered()), this, SLOT(Test()));

	quitAction = new QAction(tr("&Quit"), this);
	connect(quitAction, SIGNAL(triggered()), qApp, SLOT(quit()));
}
void
BlastpitGui::InitTray()
{
	trayIconMenu = new QMenu(this);
	trayIconMenu->addAction(minimizeAction);
	trayIconMenu->addAction(restoreAction);
	trayIconMenu->addAction(testAction);
	trayIconMenu->addSeparator();
	trayIconMenu->addAction(quitAction);

	trayIcon = new QSystemTrayIcon(this);
	trayIcon->setContextMenu(trayIconMenu);
}
void
BlastpitGui::SetIcon()
{
	QIcon icon(":///blastpitgui.png");

	trayIcon->setIcon(icon);
	setWindowIcon(icon);
}
void
BlastpitGui::closeEvent(QCloseEvent *event)
{
	if (trayIcon->isVisible()) {
		hide();
		event->ignore();
	}
}
void
BlastpitGui::SetTrayBalloon(QString title, QString text)
{
	trayIcon->showMessage(title, text);
}
void
BlastpitGui::IconActivated(QSystemTrayIcon::ActivationReason reason)
{
	switch (reason) {
		case QSystemTrayIcon::Trigger:
			break;
		case QSystemTrayIcon::DoubleClick:
			break;
		case QSystemTrayIcon::MiddleClick:
			break;
		default:;
	}
}

void
BlastpitGui::UpdateButton(int button, bool state)
{
	// * These two are the least useful *
	// b0 = touch anywhere on outer ring
	// b4 = touch inner ring

	// * If these three happen simultaneously then the main
	// ring has been pushed down (hard to do accidentally) *
	// b2 = I (left side pushed down)
	// b3 = III (right side pushed down)
	// b1 = O (bottom pushed down)

	if (button == NUBUTTON_BOTTOM) {
		qApp->quit();
	}
	if (button == NUBUTTON_LEFT && state == true) {
		LpDialog *testDialog = new LpDialog;
		Qt::WindowFlags flags = testDialog->windowFlags();
		testDialog->setWindowFlags(flags | Qt::WindowStaysOnTopHint);
		testDialog->show();
	}
	if (button == NUBUTTON_INNER && state == true) {
		struct BpHID nulooq;
		nulooq.event = HID_BUTTON;
		nulooq.value.iVal = 4;
		QByteArray ba(reinterpret_cast<const char *>(&nulooq),
			      sizeof(nulooq));
		QUdpSocket sock(this);
		// QHostAddress dest( "10.47.47.139" );
		QHostAddress dest("10.47.1.231");
		sock.writeDatagram(ba, dest, LISTEN_PORT);
	}
	if (button == NUBUTTON_TOP) {
	}
}

void
BlastpitGui::UpdateDial(int x, int y, int z, int rx, int ry, int rz)
{
	rz++;  // to squash warnings
	rx++;

	// ry = NuLOOQ grey ring rotation (-256 - 256)
	// x = grey ring x (-512 - 512)
	// z = grey ring y (-512 - 512)
	// y = jog dial rotation (0-4095)

	// Ignore dead zone
	/*x = ( x > -30 && x < 30 ) ? 0 : x;
	 * z = ( z > -30 && z < 30 ) ? 0 : z;
	 * y = ( y > -30 && y < 30 ) ? 0 : y;
	 * ry = ( ry > -30 && ry < 30 ) ? 0 : ry;
	 * if( x + y + z + ry == 0 ) {
	 *      return;
	 * }*/

	struct BpHID nulooq;

	// Prefer rotation over movement
	if (ry != 0) {
		// Rotate
		nulooq.event = HID_AXIS_ROT;
		nulooq.value.fVal = (float)ry / (float)256;
		nulooq.value.fVal /= 10;
		QByteArray ba(reinterpret_cast<const char *>(&nulooq),
			      sizeof(nulooq));
		QUdpSocket sock(this);
		// QHostAddress dest( "10.47.47.139" );
		QHostAddress dest("10.47.1.231");
		sock.writeDatagram(ba, dest, LISTEN_PORT);
	}
	if (x != 0) {
		// Move x
		nulooq.event = HID_AXIS_X;
		nulooq.value.fVal = (float)x / (float)512;
		nulooq.value.fVal /= 10;
		QByteArray ba(reinterpret_cast<const char *>(&nulooq),
			      sizeof(nulooq));
		QUdpSocket sock(this);
		// QHostAddress dest( "10.47.47.139" );
		QHostAddress dest("10.47.1.231");
		sock.writeDatagram(ba, dest, LISTEN_PORT);
	}
	if (z != 0) {
		// Move y
		nulooq.event = HID_AXIS_Y;
		nulooq.value.fVal = (float)z / (float)512;
		nulooq.value.fVal /= 10;
		QByteArray ba(reinterpret_cast<const char *>(&nulooq),
			      sizeof(nulooq));
		QUdpSocket sock(this);
		// QHostAddress dest( "10.47.47.139" );
		QHostAddress dest("10.47.1.231");
		sock.writeDatagram(ba, dest, LISTEN_PORT);
	}
	if (y != 0) {
		// Jog wheel
		nulooq.event = HID_AXIS_JOG;
		nulooq.value.iVal = y;
		nulooq.value.fVal /= 10;
		QByteArray ba(reinterpret_cast<const char *>(&nulooq),
			      sizeof(nulooq));
		QUdpSocket sock(this);
		// QHostAddress dest( "10.47.47.139" );
		QHostAddress dest("10.47.1.231");
		sock.writeDatagram(ba, dest, LISTEN_PORT);
	}

	// nulooq.value.fVal /= 10;
	// Blast them off into space
	qDebug() << "x: " << x << "y: " << y << "z: " << z << "rx: " << rx
		 << "ry: " << ry << "rz: " << rz;


	float val = (float)(360.0 / 1024.0);
	float rye = (z + 512);

	val = val * rye;
	/* int res = (int)val; */
	ui->dial->setValue((int)val);
}

void
BlastpitGui::ToggleLight()
{
	// Get light status

	// Send new light status
}
void
BlastpitGui::ToggleDoor()
{
	// Get door status

	// Send new door status
}
void
BlastpitGui::ToggleRun()
{
	// Get running status

	// Start or cancel job
}
void
BlastpitGui::Listen()
{
	// listen:      Create a thread to listen for network connections.
	// Setup
	//              signals and slots

	// TODO: Fix this so that only one listen thread can exist.

	QThread *thread = new QThread;
	Network *worker = new Network();

	worker->moveToThread(thread);

	connect(thread, SIGNAL(started()), worker, SLOT(listenTcp()));
	connect(thread, SIGNAL(started()), worker, SLOT(listenUdp()));
	// connect(worker, SIGNAL(finished()), thread, SLOT(quit()));
	// connect(worker, SIGNAL(finished()), worker, SLOT(deleteLater()));
	connect(thread, SIGNAL(finished()), thread, SLOT(deleteLater()));
	// connect(worker, SIGNAL(shutdown()), qApp, SLOT(quit()));

	// Balloon callback
	// connect(worker, SIGNAL(balloon(QString, QString)), this,
	// SLOT(SetTrayBalloon(QString, QString)));
	// connect(worker, SIGNAL(Parse(int, struct BpPacket *)), this,
	// SLOT(Parse(int, struct BpPacket *)));

	thread->start();

	SetTrayBalloon("Tentacle",
		       "Server listening on " + Network::getLocalIP());
	return;
}

void
BlastpitGui::Parse(int command, struct BpPacket *netbuf)
{
	typedef struct {
		int command;
		int (BlastpitGui::*function)(struct BpPacket *);
	} netfunc;

	// Can't remember exactly what this does.
	// I think that it's an array of functions that are called
	// according to the contents of the network packet.
	const netfunc functions[] = {{999, &BlastpitGui::LoadSVG}};

	for (unsigned int i = 0; i < sizeof(functions) / sizeof(*functions);
	     i++) {
		if (functions[i].command == command) {
			int result = (this->*functions[i].function)(netbuf);
			if (result) {
				qDebug() << "Command " << command
					 << " failed with return code "
					 << result;
			}
		}
	}
}
int
BlastpitGui::LoadSVG(struct BpPacket *netbuf)
{
	(void)netbuf;
	/* pugi::xml_document xml; */
	/* pugi::xml_parse_result result = */
	/* 	xml.load_buffer(netbuf->data, netbuf->length); */
	/* delete[](char *) netbuf; */
	/* if (result) { */
	/* 	for (pugi::xml_node tool = xml.root().first_child(); tool; */
	/* 	     tool = xml.next_sibling()) { */
	/* 		qDebug() << "Tool:" << tool.name(); */

	/* 		for (pugi::xml_attribute attr =
	 * tool.first_attribute();
	 */
	/* 		     attr; attr = attr.next_attribute()) { */
	/* 			qDebug() << " " << attr.name() << "=" */
	/* 				 << attr.value(); */
	/* 		} */
	/* 	} */
	/* } else { */
	/* 	qDebug("XML load failed"); */
	/* 	return kErrXmlLoadFailed; */
	/* } */
	return kOK;
}
void
BlastpitGui::NuLOOQ()
{
	// NuLOOQ controller thread
	QThread *thread = new QThread;
	Nulooq *worker = new Nulooq();
	worker->moveToThread(thread);

	connect(thread, SIGNAL(started()), worker, SLOT(Loop()));
	connect(worker, SIGNAL(finished()), thread, SLOT(quit()));
	connect(worker, SIGNAL(finished()), worker, SLOT(deleteLater()));
	connect(thread, SIGNAL(finished()), thread, SLOT(deleteLater()));
	connect(worker, SIGNAL(shutdown()), qApp, SLOT(quit()));

	connect(worker, SIGNAL(motionEvent(int, int, int, int, int, int)),
		this, SLOT(UpdateDial(int, int, int, int, int, int)));
	connect(worker, SIGNAL(buttonEvent(int, bool)), this,
		SLOT(UpdateButton(int, bool)));

	thread->start();
}
void
BlastpitGui::GeneratePosHelp()
{
}
int
BlastpitGui::SendXML(QHostAddress dest, pugi::xml_document *svg)
{
	QHostAddress dummy = dest;
	pugi::xml_document *dummy2 = svg;
	dest = dummy;
	svg = dummy2;

	// Convert the SVG to Rofin XML and send to the laser host
	return false;
}
void
BlastpitGui::Test()
{
	const char source[] =
		"<mesh name='sphere'><bounds>0 0 1 1</bounds></mesh>";
	pugi::xml_document xml;
	pugi::xml_parse_result result =
		xml.load_buffer(source, sizeof(source));
	if (result) {
		/* pugi::xml_node node = xml.root(); */
		/* qDebug(node.name()); */
		/* qDebug(node.value()); */
	}
}
// vim:foldmethod=syntax
