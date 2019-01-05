#-------------------------------------------------
#
# Project created by QtCreator 2015-05-24T16:24:26
#
#-------------------------------------------------

QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = blastpitgui
TEMPLATE = app
#CONFIG += debug

#INCLUDEPATH += "$$PWD/../libblastpit/"
INCLUDEPATH += "../libblastpit"
INCLUDEPATH += ../../sub/pugixml/src

QMAKE_CXXFLAGS_DEBUG += -O0
QMAKE_CXXFLAGS_RELEASE += -O2

# This is needed to search for local libblastpit at runtime
QMAKE_RPATHDIR += "../libblastpit"

SOURCES += \
	lpdialog.cpp \
	main.cpp \
	nulooq.cpp \
	paramdialog.cpp \
        blastpitgui.cpp

HEADERS  += \
    lpdialog.h \
    nulooq.h \
    paramdialog.h \
    blastpitgui.h

FORMS    += \
	mainwindow.ui \
	lpdialog.ui \
	paramdialog.ui

LIBS    += -L$$OUT_PWD/../libblastpit/ -L$$OUT_PWD/../blastpit/release/ -L$$OUT_PWD/../blastpit/debug/ -lblastpit


RESOURCES += \
    blastpitgui.qrc

unix {
	DEFINES += SPACENAV
	LIBS    += -lX11 -lspnav 

}
