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
#include <stdint.h>

#define 	MAX_CON 	10

enum ws_status {
	CONNECTING 	= 1,
	OPEN 		= 2,
	CLOSING 	= 4,
	CLOSED 		= 8
};

typedef struct {
	uint32_t fd;
	uint32_t status;
	struct sockaddr_storage remoteaddr;
} ws_connection_t;

typedef struct {
	char *header;
	char *value;
} http_header_t;

int ws_server(void);
static int get_listener_socket(void);
static ws_connection_t *accept_ws_connection(void);
static void setup_connections(void);

int ws_handshake(ws_connection_t *);
int send_ws_frame(ws_connection_t *, char *buf, int len);
int receive_ws_frame(ws_connection_t *, char *buf, int len);

// http stuff
void parse_http_request(char *data, char *method, HTTP_header *, int *count);
