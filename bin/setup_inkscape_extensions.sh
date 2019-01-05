#!/bin/sh

BLASTPIT_EXTENSION_DIR=${HOME}/usr/src/blastpit/src/inkscape
INKSCAPE_EXTENSION_DIR=${HOME}/.config/inkscape/extensions

for extension in ${BLASTPIT_EXTENSION_DIR}/*.inx
do
	ln -sv $extension ${INKSCAPE_EXTENSION_DIR}/
	ln -sv ${extension%.*}.py ${INKSCAPE_EXTENSION_DIR}/
done
