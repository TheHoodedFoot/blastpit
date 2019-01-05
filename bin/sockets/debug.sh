#!/usr/bin/env bash

autoreconf --force --install
./configure CFLAGS="-ggdb3 -O0" CXXFLAGS="-ggdb3 -O0"

