#!/bin/sh

for qtfile in Core Gui Widgets
do
	cp -v ${HOME}/.wine/rofin/drive_c/Qt/Qt5.14.2/5.14.2/mingw73_32/bin/Qt5$qtfile.dll \
		${HOME}/projects/blastpit/build/release/release/
done

for file in libgcc_s_dw2-1 libstdc++-6
do
	cp -v ${HOME}/.wine/rofin/drive_c/Qt/Qt5.14.2/5.14.2/mingw73_32/bin/$file.dll \
		${HOME}/projects/blastpit/build/release/release/
done

mkdir -p ${HOME}/projects/blastpit/build/release/release/platforms
cp -v ${HOME}/.wine/rofin/drive_c/Qt/Qt5.14.2/5.14.2/mingw73_32/plugins/platforms/qwindows.dll \
	${HOME}/projects/blastpit/build/release/release/platforms/
