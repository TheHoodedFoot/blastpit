#!/usr/bin/env sh
set -euo pipefail

# Helper for Vimspector that returns the debugger (MIMode)
head -n1 $(git rev-parse --show-toplevel)/.debugcmd
