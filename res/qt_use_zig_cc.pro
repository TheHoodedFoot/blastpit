# Use:
#
# qmake -o Makefile <project.pro>
# 
# The executable can then be run using 32-bit wine

SOURCES += hello.cpp

TARGET = helloworld

QMAKE_CC = "zig cc"
QMAKE_CFLAGS = "-target i386-windows-gnu"
QMAKE_CXX = "zig c++"
QMAKE_CXXFLAGS = "-target i386-windows-gnu"
QMAKE_LINK = "zig c++"
QMAKE_LFLAGS = "-target i386-windows-gnu"

QMAKE_LIBDIR += "/home/thf/.wine/rofin/drive_c/Qt/Qt5.14.2/5.14.2/mingw73_32/lib"
QMAKE_LIBDIR += "/home/thf/.wine/rofin/drive_c/Qt/Qt5.14.2/Tools/mingw730_32/i686-w64-mingw32/lib"

QMAKE_LIBS_OPENGL = "/home/thf/.wine/rofin/drive_c/Qt/Qt5.14.2/5.14.2/mingw73_32/lib/liblibGLESv2.a"
QMAKE_LIBS_OPENGL_ES2 = "/home/thf/.wine/rofin/drive_c/Qt/Qt5.14.2/5.14.2/mingw73_32/lib/liblibGLESv2.a"
