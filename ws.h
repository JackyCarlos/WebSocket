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

enum wsstatus {
	INITIALIZING = 1,
	ESTABLISHED = 2
};

typedef struct {
	unsigned int fd;
	unsigned int status;
	struct sockaddr_storage remoteaddr;
} wsConnection;

typedef struct {
	unsigned id;
	unsigned listeningSfd;
	
	unsigned int connMax;
	unsigned int connCount;

	wsConnection *connections;
} wsServer;

wsServer *createWsServer(void);
int get_listener_socket(void);
wsConnection *acceptWsConnection(wsServer *server);
