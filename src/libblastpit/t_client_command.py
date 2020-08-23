#!/usr/bin/env python3

import blastpy as bp
from myconfig import BP_NET_TIMEOUT

# The main Blastpit object
pit = bp.blastpitNew()

# bp.connectToServer(pit, "localhost:8080", BP_NET_TIMEOUT)

# Disabled due to segfault
# bp.sendCommand(pit, 123, bp.kStatus)

# result = bp.getReply(pit, 123)
# print("Result of getReply is %s" % result)

# bp.disconnectFromServer(pit)
