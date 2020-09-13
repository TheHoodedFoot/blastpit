# NOTE: When building under Wine, ensure that the build directory
# is outside of the Windows 'My Documents' range, as the space in
# the name can cause problems when compiling

QT += core network gui widgets

win32 {
	QT += axcontainer
	LIBS += -lwsock32
}

TARGET = lmosgui
TEMPLATE = app
CONFIG += widgets

CONFIG += debug c++11
#CONFIG += debug_and_release build_all c++11

QMAKE_CXXFLAGS_RELEASE += -DDEBUG_LEVEL=0
QMAKE_CXXFLAGS_DEBUG   += -DDEBUG_LEVEL=9 -Og -g3

linux {
	#QMAKE_CC = "ccache clang"
	#QMAKE_CXX = "ccache clang++"
	#QMAKE_LINK = "ccache clang++"
}

INCLUDEPATH += $$PWD/../libblastpit
INCLUDEPATH += $$PWD/../submodules/mongoose
INCLUDEPATH += $$PWD/../submodules/sds
INCLUDEPATH += $$PWD/../submodules/mxml

#LIBS += -lwsock32

SOURCES += main.cpp \
	../libblastpit/blastpit.c \
	../libblastpit/websocket.c \
	../libblastpit/xml.c \
	../submodules/mongoose/mongoose.c \
	../submodules/mxml/mxml-attr.c \
	../submodules/mxml/mxml-entity.c \
	../submodules/mxml/mxml-file.c \
	../submodules/mxml/mxml-get.c \
	../submodules/mxml/mxml-index.c \
	../submodules/mxml/mxml-node.c \
	../submodules/mxml/mxml-private.c \
	../submodules/mxml/mxml-search.c \
	../submodules/mxml/mxml-set.c \
	../submodules/mxml/mxml-string.c \
	../submodules/sds/sds.c \
	lmos-tray.cpp \
	lmos.cpp \
	parser.cpp \
	traysettings.cpp

HEADERS += \
	../libblastpit/blastpit.h \
	../libblastpit/websocket.h \
	../libblastpit/xml.h \
	../submodules/mongoose/mongoose.h \
	../submodules/mxml/mxml.h \
	../submodules/sds/sds.h \
	lmos-tray.hpp \
	lmos.hpp \
	parser.hpp \
	traysettings.h

win32 {
	SOURCES += lmosactxlib.cpp
	HEADERS += lmosactxlib.h
}

FORMS += \
    lmos-tray.ui \
    traysettings.ui

RC_FILE = lmos-tray.rc
RESOURCES = lmos_tray.qrc

win32 {
GITVERSION = $$system( \"C:\\Qt\\PortableGit\\bin\\git.exe\" describe --long --dirty --always )
GITBRANCH = $$system( \"C:\\Qt\\PortableGit\\bin\\git.exe\" branch --show-current )
GITDESC = $$system( \"C:\\Qt\\PortableGit\\bin\\git.exe\" log -1 --pretty=format:%s )
GITDATE = $$system( \"C:\\Qt\\PortableGit\\bin\\git.exe\" show -s --format=%ci )
VERSTR = '\"$${GITBRANCH}: $${GITVERSION} ($${GITDESC}) $${GITDATE}\"'
QMAKE_CXXFLAGS += -DVER=\\\"$${VERSTR}\\\"
}

linux {
QMAKE_CXXFLAGS += -DVER=\\\"undefined\\\"
}
