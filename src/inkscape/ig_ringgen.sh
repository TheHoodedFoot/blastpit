#!/usr/bin/env sh
set -euo pipefail

# Hack to get the final argument, which for Inkscape extensions is the filename
for FILENAME; do true; done

${HOME}/projects/blastpit/build/ig_ringgen ${FILENAME}
