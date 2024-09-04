#!/bin/sh

SOURCE=ems_node.c
DESTINATION=ems_node_generated.js
MAIN=ems_node.js

emcc ${SOURCE} -o ${DESTINATION} -sMODULARIZE -sEXPORTED_RUNTIME_METHODS=ccall

node ${MAIN}
