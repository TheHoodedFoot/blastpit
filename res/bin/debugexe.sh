#!/usr/bin/env sh
set -euo pipefail

# debugcmd.sh: Vimspector helper that returns the debugger (MIMode) or executable
tail -n1 $(git rev-parse --show-toplevel)/.debugcmd
