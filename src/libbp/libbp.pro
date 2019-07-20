#-------------------------------------------------
#
# Project created by QtCreator 2015-05-30T10:48:51
#
#-------------------------------------------------

TARGET = bp
TEMPLATE = lib

CONFIG += c

QT += core network

QMAKE_CXXFLAGS_DEBUG += -O0 -Wno-old-style-cast
QMAKE_CXXFLAGS_RELEASE += -O2

DEFINES += QT_NO_VERSION_TAGGING        # This prevents an issue with swig

INCLUDEPATH += $$PWD/../../sub/pugixml/src
INCLUDEPATH += $$PWD/../../sub/CRCpp/inc
VPATH += $$PWD/../../sub/pugixml/src
VPATH += $$PWD/../../sub/CRCpp/inc

HEADERS += \
	pugiconfig.hpp \
	pugixml.hpp \
	network.hpp \
	blastpit.h \
	klass.h \
	linkedlist.h \
	message.h \
	mqtt.h \
	new.h \
    t_common.h

SOURCES += \
	pugixml.cpp \
	network.cpp \
	blastpit.c \
	linkedlist.c \
	message.c \
	mqtt.c \
	new.c

win32 {
	#CONFIG += staticlib
        LIBS += -L$$PWD/../../redist/eclipse-paho-mqtt-c-win32-1.3.0/lib
        INCLUDEPATH += $$PWD/../../sub/paho_mqtt_c/src
}

!win32 {
	target.path = $$PWD/
	INSTALLS += target
}

LIBS += -lpaho-mqtt3a
