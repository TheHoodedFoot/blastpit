#include <QApplication>
#include "blastpitgui.h"

int
main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	BlastpitGui w;

	return a.exec();
}
