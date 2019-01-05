@echo off

set BUILDTYPE=release
set STATICFLAG=-static
set VERSION=5.7.0

set QTDIR=C:\Qt\qt-everywhere-opensource-src-%VERSION%
set INSTALLDIR=C:\Qt\Qt-%VERSION%-%BUILDTYPE%
set BUILDDIR=C:\Qt\build

set PATH=%PATH%;%INSTALLDIR%\qmake
set PATH=%PATH%;%SystemRoot%\System32
set PATH=%PATH%;%QTDIR%\qtbase
set PATH=%PATH%;%INSTALLDIR%\qtbase\bin
set QMAKESPEC=win32-g++

rmdir /Q /S %BUILDDIR%
mkdir %BUILDDIR%
cd %BUILDDIR%
%QTDIR%\configure -prefix %INSTALLDIR% -%BUILDTYPE% %STATICFLAG% -opensource -confirm-license -platform win32-g++ -opengl desktop ^
 -nomake examples^
 -nomake tests^
 -skip qt3d^
 -skip qtandroidextras^
 -skip qtcanvas3d^
 -skip qtconnectivity^
 -skip qtdoc^
 -skip qtgraphicaleffects^
 -skip qtlocation^
 -skip qtmacextras^
 -skip qtmultimedia^
 -skip qtquickcontrols^
 -skip qtquickcontrols2^
 -skip qtscript^
 -skip qtsensors^
 -skip qtserialbus^
 -skip qtserialport^
 -skip qttranslations^
 -skip qtwayland^
 -skip qtwebchannel^
 -skip qtwebengine^
 -skip qtwebsockets^
 -skip qtwebview^
 -skip qtx11extras^
 -skip qtxmlpatterns^

echo MinGW/Qt only environment:
echo -- QTDIR set to %QTDIR%
echo -- PATH set to %PATH%
echo -- QMAKESPEC set to %QMAKESPEC%
echo
echo Before running this configure script, you should have edited the mkspecs and set the -static -static-libgcc flags for QMAKE_LFLAGS_RELEASE in qt-everywhere-opensource.../qtbase/mkspecs/win32-g++/qmake.conf
echo
echo
echo Now run:
echo mingw32-make -j2
echo mingw32-make install
echo
echo To reconfigure, run mingw32-make confclean
