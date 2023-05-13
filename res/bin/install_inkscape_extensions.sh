#!/bin/sh

INSTALL_DIR=${HOME}/.config/inkscape/extensions
SOURCE_DIR=${HOME}/projects/blastpit/src/inkscape

for file in ${SOURCE_DIR}/*.{inx,py,sh}
do
	echo "Creating link to $file in ${INSTALL_DIR}..."
	ln -vsf $(readlink -e $file) ${INSTALL_DIR}
done
