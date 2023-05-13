#!/usr/bin/env sh
set -euo pipefail

PROJECT=scaps
PREFIX=${HOME}/wine/${PROJECT}

if [ ! -d ${PREFIX} ]
then
	echo "The directory ${PREFIX} does not exist."
	exit 1
fi

# Abort if alphacam running
if pgrep awireaps
then
	echo "Alphacam is running. Not killing wine running services."
else
	# Existing running services prevent demonstration mode
	pkill --full "C:\\\windows\\\system32" || /bin/true
	pkill wineserver || /bin/true
fi

env \
	WINEPREFIX="${PREFIX}" \
	wine \
	${PREFIX}/drive_c/scaps/sam2d/samlight/sam_light.exe

