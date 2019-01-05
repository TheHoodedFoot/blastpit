#define BUFFERSIZE 1024

typedef struct {
	unsigned int command;
	unsigned int checksum;
	unsigned int length;
} netHeader;

union netBuffer {
	netHeader header;
	char data[BUFFERSIZE];
};

