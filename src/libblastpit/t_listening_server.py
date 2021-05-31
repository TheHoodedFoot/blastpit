#!/usr/bin/env python3

#
# THIS IS CURRENTLY CAUSING SEGFAULTS
#

import blastpy as bp

# from myconfig import BP_NET_TIMEOUT

# The main Blastpit object
pit = bp.blastpitNew()

# bp.serverCreate(pit, "8080")
# bp.pollMessages(pit)
# bp.disconnectFromServer(pit)
# bp.serverDestroy(pit)
bp.blastpitDelete(pit)
