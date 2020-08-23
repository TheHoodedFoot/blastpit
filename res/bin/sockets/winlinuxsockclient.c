#include "winlinuxsock.h"

#ifdef __unix__
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#endif

#ifdef _WIN32
#include <winsock.h>
WSADATA wsaData;
void closesocket(int socket)
{
	close(socket);
}
#endif

#include <stdio.h>
#include <string.h>

int main(void)
{
	int thisSocket;
	struct sockaddr_in destination;

	destination.sin_family = AF_INET;
	thisSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (thisSocket < 0) {
		printf("Socket creation failed\n");
		return 0;
	}

	destination.sin_port = htons(13374);
	destination.sin_addr.s_addr = inet_addr("127.0.0.1");
	if (connect(thisSocket, (struct sockaddr *)&destination,sizeof(destination))!=0){
		printf("Socket connection failed\n");
		return 0;
	}
	printf("Connected\n");

	union netBuffer buffer;
	memset(&buffer.header, 0, sizeof(buffer.header));
	buffer.header.command = 9999;
	buffer.header.checksum = 4321;
	buffer.header.length = 5332;
	send(thisSocket, (void *)&buffer.header, sizeof(buffer.header), 0);

	close(thisSocket);
}
