#-------------------------------------------------
#
# Project created by QtCreator 2015-05-24T16:24:26
#
#-------------------------------------------------

QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = bpgui
TEMPLATE = app
#CONFIG += debug

#INCLUDEPATH += "$$PWD/../libbp/"
INCLUDEPATH += "../libbp"
INCLUDEPATH += ../../sub/pugixml/src

QMAKE_CXXFLAGS_DEBUG += -O0
QMAKE_CXXFLAGS_RELEASE += -O2

# This is needed to search for local libbp at runtime
QMAKE_RPATHDIR += "../libbp"

SOURCES += \
	lpdialog.cpp \
	main.cpp \
	nulooq.cpp \
	paramdialog.cpp \
        bpgui.cpp

HEADERS  += \
    lpdialog.h \
    nulooq.h \
    paramdialog.h \
    bpgui.h

FORMS    += \
	mainwindow.ui \
	lpdialog.ui \
	paramdialog.ui

LIBS    += -L$$OUT_PWD/../libbp/ -L$$OUT_PWD/../blastpit/release/ -L$$OUT_PWD/../blastpit/debug/ -lbp -lpaho-mqtt3a


RESOURCES += \
    bpgui.qrc

unix {
	DEFINES += SPACENAV
	LIBS    += -lX11 -lspnav 

}
