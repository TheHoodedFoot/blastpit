#-------------------------------------------------
#
# Project created by QtCreator 2015-05-30T10:48:51
#
#-------------------------------------------------

TARGET = blastpit
TEMPLATE = lib

CONFIG += c++11
#CONFIG -= qt
#CONFIG += staticlib

QT += core network

QMAKE_CXXFLAGS_DEBUG += -O0
QMAKE_CXXFLAGS_RELEASE += -O2

DEFINES += QT_NO_VERSION_TAGGING        # This prevents an issue with swig

INCLUDEPATH += $$PWD/../../sub/pugixml/src
INCLUDEPATH += $$PWD/../../sub/CRCpp/inc
VPATH += $$PWD/../../sub/pugixml/src
VPATH += $$PWD/../../sub/CRCpp/inc

#VPATH += "$$PWD/../../sub/pugixml/src"

SOURCES += \
	pugixml.cpp \
	element.cpp \
	layer.cpp \
	path.cpp \
	segment.cpp \
	arc.cpp \
	network.cpp \
	text.cpp \
        blastpit.cpp

HEADERS += \
	pugiconfig.hpp \
	pugixml.hpp \
	arc.h \
	common.h \
	element.h \
	layer.h \
	path.h \
	segment.h \
	text.h \
	network.h \
	svg.h \
	blastpit.h \
    ../../sub/CRCpp/inc/CRC.h

#CONFIG( release ) {
#	CONFIG += staticlib
#}

win32 {
	CONFIG += staticlib
}

!win32 {
	target.path = $$PWD/
	INSTALLS += target
}
