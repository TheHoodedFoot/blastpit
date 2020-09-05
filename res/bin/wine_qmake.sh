#!/bin//sh

PROJECT_ROOT=$(git rev-parse --show-toplevel)
BUILDDIR=${PROJECT_ROOT}/build
WINEDIR=${HOME}/.wine/rofin

mkdir ${BUILDDIR}
cd ${BUILDDIR}

env WINEPREFIX="${WINEDIR}" \
	WINEARCH="win32" \
	WINEPATH="C:\\Qt\\Qt5.14.2\\Tools\\mingw730_32\\bin" \
	wine ${WINEDIR}/drive_c/Qt/Qt5.14.2/5.14.2/mingw73_32/bin/qmake.exe \
	-o Makefile \
	..\\src\\blastpit.pro