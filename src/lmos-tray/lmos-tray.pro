#-------------------------------------------------
#
# Project created by QtCreator 2014-12-26T12:27:23
#
#-------------------------------------------------

QT       += core gui network widgets

TARGET = lmos-tray
TEMPLATE = app

#TYPELIBS = $$system(dumpcpp -getfile {18213698-A9C9-11D1-A220-0060973058F6})

QMAKE_CXXFLAGS_DEBUG += -O0
QMAKE_CXXFLAGS_RELEASE += -O2

INCLUDEPATH += $$PWD/../
INCLUDEPATH += $$PWD/../../sub/pugixml/src
INCLUDEPATH += ../libbp

SOURCES +=	lmos-tray.cpp \
		main.cpp \
    traysettings.cpp

SOURCES +=	lmos.cpp \
		parser.cpp 

HEADERS  += 	../libbp/blastpit.h \
        	lmos-tray.hpp \ 
    ../libbp/network.hpp \
    parser.hpp \
    traysettings.h

HEADERS += lmos.hpp

FORMS    += \
    lmos-tray.ui \
    traysettings.ui

RC_FILE = lmos-tray.rc

RESOURCES = \
    lmos_tray.qrc

LIBS += -L$$OUT_PWD/../libbp/ -L$$OUT_PWD/../libbp/debug/ -L$$OUT_PWD/../libbp/release/ -lbp
QMAKE_LFLAGS    += -Wl,-rpath=$$OUT_PWD/../libbp

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

win32 {
        INCLUDEPATH += $$PWD/../../sub/paho_mqtt_c/src
}

QMAKE_CXXFLAGS += -Wno-narrowing -Wno-missing-field-initializers -fexceptions

#define CLSID_LMOS "{18213698-A9C9-11D1-A220-0060973058F6}"

