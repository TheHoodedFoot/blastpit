#!/usr/bin/env sh
set -euo pipefail

# Select entity type
printf "ScCciSetStringValue(36,\"ScRectangle2D\"\n" | nc -w1 localhost 1350

# Send dimensions [name, parent, pen, cen_x, cen_y, cen_z, size_x, size_y, 0, ""]
printf "ScCciCreateEntity(\"MyEntity\",\"\",3,50,50,0,40,40,0,\"\")" | nc -w1 localhost 1350
