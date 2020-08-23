#!/bin/sh

#DESTDIR=/c/projects/blastpit/build-blastpit-Desktop_Qt_5_11_1_MinGW_32bit-Release/lmos-tray/release
QTVERSION=$(ls /c/Qt | grep -E 'Qt[\.[:digit:]]+' | sed -E 's/Qt([\.[:digit:]]+)/\1/')
#QTVERSION=$(ls /c/Qt | grep -E 'Qt[\.[:digit:]]+/' | sed -E 's/Qt([\.[:digit:]]+)/\1/')
#MINGW=73
MINGW=$(ls /c/Qt/Qt${QTVERSION}/${QTVERSION} | grep mingw)
BUILDDIR=/c/projects/blastpit/build/lmos-tray
BUILD=debug

for BUILDTYPE in release debug
do
#cp -v /c/projects/blastpit/build*/libbp/debug/bp.dll ${DESTDIR}
	cp -v --no-clobber /c/projects/blastpit/redist/eclipse-paho-mqtt-c-win32-1.3.0/lib/paho-mqtt3a.dll ${BUILDDIR}/${BUILDTYPE}/
done
cp -v --no-clobber /c/Qt/Qt${QTVERSION}/${QTVERSION}/${MINGW}/bin/Qt5{Core,Test,Gui,Network,Widgets}*.dll ${BUILDDIR}/${BUILDTYPE}/
#cp -v --no-clobber /c/Qt/Qt${QTVERSION}/${QTVERSION}/mingw${MINGW}_32/bin/Qt5{Core,Test,Gui,Network,Widgets}*.dll ${DESTDIR}
