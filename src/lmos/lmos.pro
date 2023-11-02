# NOTE: When building under Wine, ensure that the build directory
# is outside of the Windows 'My Documents' range, as the space in
# the name can cause problems when compiling

QT += core network gui widgets

win32 {
	QT += axcontainer
	LIBS += -lwsock32
}

TARGET = lmostray
TEMPLATE = app
CONFIG += widgets

#CONFIG += debug c++11
#CONFIG += release c++11
CONFIG += debug_and_release build_all c++11

QMAKE_CXXFLAGS_RELEASE += -DDEBUG_LEVEL=0
QMAKE_CXXFLAGS_DEBUG   += -DDEBUG_LEVEL=5 -O0 -g3

linux {
#	QMAKE_CC = "ccache clang"
#	QMAKE_CXX = "ccache clang++"
#	QMAKE_LINK = "ccache clang++"
	LIBS += -L. -lblastpit
}

INCLUDEPATH += $$PWD/../libblastpit
INCLUDEPATH += $$PWD/../submodules/mongoose
INCLUDEPATH += $$PWD/../submodules/sds
INCLUDEPATH += $$PWD/../submodules/mxml

SOURCES += main.cpp \
	lmos-tray.cpp \
	lmos.cpp \
	parser.cpp \
	traysettings.cpp

HEADERS += \
	../libblastpit/blastpit.h \
	../libblastpit/websocket.h \
	../libblastpit/xml.h \
	lmos-tray.hpp \
	lmos.hpp \
	parser.hpp \
	traysettings.h

LIBS += \
	$$OUT_PWD/blastpit.o \
	$$OUT_PWD/mongoose.o \
	$$OUT_PWD/sds.o \
	$$OUT_PWD/websocket.o \
	$$OUT_PWD/xml.o \
	$$OUT_PWD/mxml-attr.o \
	$$OUT_PWD/mxml-entity.o \
	$$OUT_PWD/mxml-file.o \
	$$OUT_PWD/mxml-get.o \
	$$OUT_PWD/mxml-index.o \
	$$OUT_PWD/mxml-node.o \
	$$OUT_PWD/mxml-private.o \
	$$OUT_PWD/mxml-search.o \
	$$OUT_PWD/mxml-set.o \
	$$OUT_PWD/mxml-string.o \

win32 {
	SOURCES += lmosactxlib.cpp
	HEADERS += lmosactxlib.h
	LIBS += -L $$PWD/../../build -lwsock32
}


FORMS += \
    lmos-tray.ui \
    traysettings.ui

RC_FILE = lmos-tray.rc
RESOURCES = lmos_tray.qrc

win32 {
GITVERSION = $$system( type $$OUT_PWD\git_version.txt )
GITBRANCH = $$system( type $$OUT_PWD\git_branch.txt )
GITDESC = $$system( type $$OUT_PWD\git_description.txt )
GITDATE = $$system( type $$OUT_PWD\git_date.txt )
VERSTR = '\"$${GITBRANCH}: $${GITVERSION} ($${GITDESC}) $${GITDATE}\"'
QMAKE_CXXFLAGS += -DVER=\\\"$${VERSTR}\\\"
}

linux {
QMAKE_CXXFLAGS += -DVER=\\\"undefined\\\"
}
