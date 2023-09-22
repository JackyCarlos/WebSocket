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

#define 	MAX_CON 	10

enum wsstatus {
	CONNECTING = 1,
	OPEN = 2,
	CLOSING = 4,
	CLOSED = 8
};

typedef struct {
	unsigned int fd;
	unsigned int status;
	struct sockaddr_storage remoteaddr;
} wsConnection;

typedef struct {
	char *header;
	char *value;
} HTTP_header;

int ws_server(void);
int get_listener_socket(void);
wsConnection *accept_ws_connection(void);
static void setup_connections(void);

int ws_handshake(wsConnection *);
int send_ws_frame(wsConnection *, char *buf, int len);
int receive_ws_frame(wsConnection *, char *buf, int len);

// http stuff
void parse_http_request(char *data, char *method, HTTP_header *, int *count);
