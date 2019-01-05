TEMPLATE = app
TARGET = network_test

QT += testlib core network

INCLUDEPATH += .
INCLUDEPATH += ../../libblastpit

HEADERS += \
	network_test.h

SOURCES += \
	network_test.cpp

LIBS += -L$$OUT_PWD/../../libblastpit -lblastpit

# We need to hard-code the path into libblastpit (during development)
QMAKE_LFLAGS += -Wl,-rpath=$$OUT_PWD/../../libblastpit
