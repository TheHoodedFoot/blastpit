#include <QApplication>
#include "bpgui.h"

int
main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	BPGui w;

	return a.exec();
}
