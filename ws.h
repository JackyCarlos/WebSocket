#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <poll.h>

typedef struct {
	unsigned int fd;
	unsigned int status;
	char data[256];
} wsConnection;

typedef struct {
	unsigned id;
	unsigned listeningSfd;
	
	unsigned int connMax = 10;
	unsigned int connCount = 0;

	struct wsConnection *connections;
} wsServer;

wsServer *createWsServer(void);
unsigned int get_listener_socket(void);
