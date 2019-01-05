#-------------------------------------------------
#
# Project created by QtCreator 2014-12-26T12:27:23
#
#-------------------------------------------------

QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = lmos-tray
TEMPLATE = app
CONFIG += widgets

#TYPELIBS = $$system(dumpcpp -getfile {18213698-A9C9-11D1-A220-0060973058F6})

QMAKE_CXXFLAGS_DEBUG += -O0
QMAKE_CXXFLAGS_RELEASE += -O2

INCLUDEPATH += $$PWD/../
INCLUDEPATH += $$PWD/../../sub/pugixml/src
INCLUDEPATH += ../libblastpit

SOURCES +=	lmos-tray.cpp \
		main.cpp

SOURCES +=	lmos.cpp \
		parser.cpp 

HEADERS  += 	../libblastpit/common.h \
		../libblastpit/element.h \
		../libblastpit/network.h \
        	lmos-tray.h \ 
		parser.h

HEADERS += lmos.h

FORMS    += \
    lmos-tray.ui

RC_FILE = lmos-tray.rc

RESOURCES = \
    lmos_tray.qrc

LIBS += -L$$OUT_PWD/../libblastpit/ -L$$OUT_PWD/../libblastpit/debug/ -L$$OUT_PWD/../libblastpit/release/ -lblastpit
QMAKE_LFLAGS    += -Wl,-rpath=$$OUT_PWD/../libblastpit

CONFIG( release ) {
        CONFIG += static
}

win32 {
        #CONFIG += static

	QT += axcontainer # Qt5
	CONFIG += axcontainer

        SOURCES += lmosactxlib.cpp
        HEADERS += lmosactxlib.h

        GITVERSION = $$system( \"C:\\Qt\\Git\\bin\\git.exe\" describe --long --dirty --always )
        GITDESC = $$system( \"C:\\Qt\\Git\\bin\\git.exe\" log -1 --pretty=format:%s )
        VERSTR = '\\"$${GITVERSION} ($${GITDESC})\"'
        DEFINES += VER=\"$${VERSTR}\"
}


QMAKE_CXXFLAGS += -Wno-narrowing -Wno-missing-field-initializers -fexceptions

#define CLSID_LMOS "{18213698-A9C9-11D1-A220-0060973058F6}"

