#!/usr/bin/env sh
set -euo pipefail

## Is there a .git directory?

if [ -d .git ]
then
	# We are in the root of an actual git repository
	echo $(readlink -e .git)
else
	# We are in a worktree
	cd ..
	echo $(git rev-parse --show-toplevel)/.git
fi
