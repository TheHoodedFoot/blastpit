#include "winlinuxsock.h"

#ifdef __unix__
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
void closesocket(int socket)
{
	close(socket);
}
#endif

#ifdef _WIN32
#include <winsock.h>
WSADATA wsaData;
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
		printf("\nSocket creation failed");
		return 0;
	}

	destination.sin_port = htons(13374);
	destination.sin_addr.s_addr = INADDR_ANY;
	if (bind
	    (thisSocket, (struct sockaddr *)&destination,
	     sizeof(destination)) < 0) {
		printf("\nBinding socket failed\n");
		return 0;
	}

	printf("\nListening on 13374...\n");
	if (listen(thisSocket, 0) < 0) {
		printf("\nListening on socket failed\n");
		if (thisSocket)
			close(thisSocket);
		return 0;
	}

	//Accept and incoming connection
	puts("Waiting for incoming connections...");

	struct sockaddr_in clientAddress;
	int clientSize = sizeof(clientAddress);
	thisSocket =
	    accept(thisSocket, (struct sockaddr *)&clientAddress,
		   (unsigned int *)&clientSize);
	if (thisSocket < 0) {
		printf("\nSocket connection failed\n");
		if (thisSocket)
			close(thisSocket);
		return 0;
	}
	printf("\nConnection established\n");

	int newData;
	union netBuffer buffer;
	memset( &buffer.header, 0, sizeof(buffer.header));

	do {
	newData = recv(thisSocket, (void *)&buffer.header, sizeof(buffer.header), 0);
	printf( "%d bytes received\n", newData);
	printf( "Values received: command: %d, checksum: %d, length: %d\n", 
			buffer.header.command,
			buffer.header.checksum,
			buffer.header.length);
		//puts( (char *) &buffer.data );
		//
		//Reply to the client
		for(;;) {
		fgets( (char *) &buffer.data, sizeof(buffer.data)-1, stdin );
		write(thisSocket, buffer.data, strlen(buffer.data));
		}
	} while (buffer.header.command != 9999);
	close(thisSocket);
}
