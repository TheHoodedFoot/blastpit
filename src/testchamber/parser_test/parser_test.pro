TEMPLATE = app
TARGET = parser_test

QT += testlib core network

INCLUDEPATH += .
INCLUDEPATH += ../../libblastpit
INCLUDEPATH += ../../lmos-tray
INCLUDEPATH += ../../../sub/pugixml/src

HEADERS += \
	../../lmos-tray/parser.h \
	../../lmos-tray/lmos.h \
	parser_test.h \

SOURCES += \
	../../lmos-tray/parser.cpp \
	../../lmos-tray/lmos.cpp \
	parser_test.cpp

LIBS += -L$$OUT_PWD/../../libblastpit -lblastpit

# We need to hard-code the path into libblastpit (during development)
QMAKE_LFLAGS += -Wl,-rpath=$$OUT_PWD/../../libblastpit
