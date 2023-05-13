#!/usr/bin/env sh
set -euo pipefail

PROJECT_ROOT=$(git rev-parse --show-toplevel)

if [ -f "${PROJECT_ROOT}/.run_after_build" ]
then
        source "${PROJECT_ROOT}/.run_after_build"
fi
