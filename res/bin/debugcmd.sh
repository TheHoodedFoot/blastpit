#!/usr/bin/env sh
set -euo pipefail

# debugcmd.sh: Vimspector helper that returns the debugger (MIMode) or executable

if [ "$#" -ne 1 ]
then
	head -n1 $(git rev-parse --show-toplevel)/.debugcmd
	exit 0
fi

if [ "$1" == "executable" ]
then
	tail -n1 $(git rev-parse --show-toplevel)/.debugcmd
fi
