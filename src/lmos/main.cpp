#include <QApplication>
#include <QMessageBox>
#include <QtGui>

#include "lmosactxlib.h"
using namespace LMOSACTXLib;

int
main( int argc, char* argv[] )
{
	Q_INIT_RESOURCE( lmos_tray );

	QApplication a( argc, argv );

	QApplication::setQuitOnLastWindowClosed( true );

	LMOSACTXLib::LMOSActX lmos_actx;
	lmos_actx.CancelJob();
	lmos_actx.ClearLayout();

	// const QString file = "C:\\inkscape_xml.xml";
	// lmos_actx.LoadXML( file );

	lmos_actx.ShowMarkingArea();

	return a.exec();
}
