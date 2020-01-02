#!/bin/sh

#DESTDIR=/c/projects/blastpit/build-blastpit-Desktop_Qt_5_11_1_MinGW_32bit-Release/lmos-tray/release
QTVERSION=5.12.3
MINGW=73
DESTDIR=/c/projects/blastpit/build/lmos-tray/debug/
BUILD=debug

cp -v /c/projects/blastpit/build*/libbp/debug/bp.dll ${DESTDIR}
cp -v /c/projects/blastpit/redist/eclipse-paho-mqtt-c-win32-1.3.0/lib/paho-mqtt3a.dll ${DESTDIR}
cp -v /c/Qt/Qt${QTVERSION}/${QTVERSION}/mingw${MINGW}_32/bin/Qt5{Core,Test,Gui,Network,Widgets}*.dll ${DESTDIR}
