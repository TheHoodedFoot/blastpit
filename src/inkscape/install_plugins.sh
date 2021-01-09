#!/bin/sh

INSTALL_DIR=${HOME}/.config/inkscape/extensions

for file in *.{inx,py}
do
	ln -vsf $(readlink -e $file) ${INSTALL_DIR}
done
