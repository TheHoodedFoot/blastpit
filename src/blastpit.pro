TEMPLATE = subdirs
SUBDIRS = libblastpit lmos-tray
CONFIG += ordered

!win32 {
	SUBDIRS += testchamber blastpitgui
	blastpitgui.depends = libblastpit
	testchamber.depends = libblastpit
}
