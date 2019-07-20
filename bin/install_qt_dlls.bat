set QTLIBSDIR=C:\Qt\Qt5.11.0\5.11.0\mingw53_32\bin
set PROJECTDIR=C:\projects\blastpit
set LIBDIR=%PROJECTDIR%\build\libbp\debug
set TESTDIR=%PROJECTDIR%\build\testchamber\blastpit_test\debug
set PAHODIR=%PROJECTDIR%\sub\eclipse-paho-mqtt-c-win32-1.3.0\lib
set VCDIR=C:\Qt\OpenSSL-Win32\bin

COPY %LIBDIR%\bp.dll %TESTDIR%\
COPY %QTLIBSDIR%\Qt5Cored.dll %TESTDIR%\
COPY %QTLIBSDIR%\Qt5Networkd.dll %TESTDIR%\
COPY %QTLIBSDIR%\Qt5Guid.dll %TESTDIR%\
COPY %QTLIBSDIR%\Qt5Testd.dll %TESTDIR%\
COPY %PAHODIR%\paho-mqtt3a.dll %TESTDIR%\
COPY %VCDIR%\msvcr120.dll %TESTDIR%\
