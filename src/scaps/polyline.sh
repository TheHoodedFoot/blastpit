#!/usr/bin/env sh
set -euo pipefail

IP_ADDR=127.0.0.1
IP_PORT=1350

###############
#  Rectangle  #
###############

echo "Rectangle"

# Select entity type
printf "ScCciSetStringValue(36,\"ScRectangle2D\")\\nScCciCreateEntity(\"MyRectangle\",\"\",3,50,50,0,25,25,0,\"\")\n" > /dev/tcp/${IP_ADDR}/${IP_PORT}

##############
#  Polyline  #
##############

echo "Polyline"

# Select entity type
printf "ScCciSetStringValue(36,\"ScPolyLine2D\")\\n" > /tmp/cmd.out

# Add first point
printf "ScCciSetDoubleValue(66,50)\\n" >> /tmp/cmd.out
printf "ScCciSetDoubleValue(67,50)\\n" >> /tmp/cmd.out
printf "ScCciSetDoubleValue(95,0)\\n" >> /tmp/cmd.out
printf "ScCciExecCommand(65)\\n" >> /tmp/cmd.out

# Add second point
printf "ScCciSetDoubleValue(66,60)\\n" >> /tmp/cmd.out
printf "ScCciSetDoubleValue(67,60)\\n" >> /tmp/cmd.out
printf "ScCciSetDoubleValue(95,0)\\n" >> /tmp/cmd.out
printf "ScCciExecCommand(65)\\n" >> /tmp/cmd.out

# Add third point
printf "ScCciSetDoubleValue(66,70)\\n" >> /tmp/cmd.out
printf "ScCciSetDoubleValue(67,40)\\n" >> /tmp/cmd.out
printf "ScCciSetDoubleValue(95,0)\\n" >> /tmp/cmd.out
printf "ScCciExecCommand(65)\\n" >> /tmp/cmd.out

# Add closing point at same location as starting point
printf "ScCciSetDoubleValue(66,50)\\n" >> /tmp/cmd.out
printf "ScCciSetDoubleValue(67,50)\\n" >> /tmp/cmd.out
printf "ScCciSetDoubleValue(95,0)\\n" >> /tmp/cmd.out
printf "ScCciExecCommand(65)\\n" >> /tmp/cmd.out

# Add additional empty polyline because ???
printf "ScCciExecCommand(66)\\n" >> /tmp/cmd.out

# Add to drawing
printf "ScCciCreateEntity(\"MyPolyline\",\"\",3,50,50,0,25,25,0,\"\")\n" >> /tmp/cmd.out

cat /tmp/cmd.out > /dev/tcp/${IP_ADDR}/${IP_PORT}
