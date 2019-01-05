#-------------------------------------------------
#
# Project created by QtCreator 2015-12-31T13:36:11
#
#-------------------------------------------------

QT       += network

QT       -= gui

TARGET = test
CONFIG   += console #debug
CONFIG   -= app_bundle

TEMPLATE = app

QMAKE_CXXFLAGS_DEBUG += -O0
QMAKE_CXXFLAGS_RELEASE += -O2

INCLUDEPATH += "$$PWD/../libblastpit/"
INCLUDEPATH += "$$PWD/../lmos-tray/"
INCLUDEPATH += $$PWD/../../sub/pugixml/src

#DEPENDPATH += "$$PWD/../libblastpit"
VPATH += "$$PWD/../libblastpit"
VPATH += "$$PWD/../lmos-tray"

SOURCES += libblastpit/runner.cpp \
	   libblastpit/main.cpp \
	   network.cpp \
	   parser.cpp \
	   lmos.cpp \

DEFINES += SRCDIR=\\\"$$PWD/\\\"

HEADERS  += \
	common.h \
	libblastpit/runner.h \
	blastpit.h \
	element.h \
	layer.h \
	network.h \
	parser.h \
	lmos.h \

LIBS    += -L$$OUT_PWD/../libblastpit/ -L$$OUT_PWD/../libblastpit/release/ -L$$OUT_PWD/../libblastpit/debug/ -lCppUTest -lCppUTestExt -lblastpit 
QMAKE_LFLAGS    += -Wl,-rpath=$$OUT_PWD/../libblastpit

test.depends = libblastpit

!win32 {
	#QMAKE_CXX = ccache clang++
	#QMAKE_CXXFLAGS = -O0 -ggdb3
	#QMAKE_CXXFLAGS_DEBUG += -O0 -ggdb3
}
