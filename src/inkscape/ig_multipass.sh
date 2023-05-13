#!/usr/bin/env sh
set -euo pipefail

# Hack to get the final argument, which for Inkscape extensions is the filename
for FILENAME; do true; done

# We redirect STDOUT and STDERR otherwise Inkscape hangs until exit
${HOME}/projects/blastpit/build/ig_multipass ${FILENAME} >/dev/null 2>&1 &
