#!/bin/sh

ROOT_DIR=$(git rev-parse --show-toplevel)

sudo cp -v ${ROOT_DIR}/src/libbp/libbp.so.1 /usr/lib64/
sudo cp -v ${ROOT_DIR}/src/libbp/_blastpy.so src/libbp/blastpy.py /usr/lib64/python3.7/site-packages
