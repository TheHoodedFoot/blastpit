TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += main.cpp

LIBS += -lole32 -loleaut32 -luuid

include(deployment.pri)
qtcAddDeployment()

