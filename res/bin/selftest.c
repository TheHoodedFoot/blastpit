// Compile with $(CXX) -o selftest -I src/libblastpit -I src/submodules/mongoose -Lbuild -lblastpit

#include "blastpit.h"

int main(int argc, char *argv[])
{
	t_Blastpit *bp = blastpitNew();
	connectToServer(bp, "ws://127.0.0.1:8000", "selftest", 10000);

	if (bp_isConnected(bp))
	{
		printf("Sending selftest command\n");
		sendCommand(bp, 1, kSelfTest);
		for (int i = 0; i < 10; i++)
		{
			pollMessages(bp);
			usleep(100000);
			if(getMessageCount(bp)) break;
		}
	}

	void *message = popMessage(bp);

	if (message) printf("Message <%s>\n", message);

	disconnectFromServer(bp);

	return 0;
}
