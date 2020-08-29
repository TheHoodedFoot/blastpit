// Compile with $(CXX) -o selftest -I src/libblastpit -I src/submodules/mongoose -Lbuild -lblastpit

#include <stdio.h>
#include <unistd.h>

#include "blastpit.h"

int main(int argc, char *argv[])
{
	t_Blastpit *bp = blastpitNew();
	connectToServer(bp, "ws://127.0.0.1:8000", 10000);

	if (bp_isConnected(bp))
	{
		printf("Sending kGetPng command\n");
		sendCommand(bp, 1, kGetPng);
		for (int i = 0; i < 10; i++)
		{
			pollMessages(bp);
			if(getMessageCount(bp))
			{
				void *message = popMessage(bp);
				if (message) printf("Message <%s>\n", message);
			}
			usleep(100000);
		}
	}

	disconnectFromServer(bp);

	return 0;
}
