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
#define 	GUID		"258EAFA5-E914-47DA-95CA-C5AB0DC85B11"

enum ws_status {
	CONNECTING 	= 1,
	OPEN 		= 2,
	CLOSING 	= 4,
	CLOSED 		= 8
};

typedef struct {
	uint32_t fd;
	uint32_t status;
	struct sockaddr_storage remote_addr;
} ws_connection_t;

typedef struct {
	char *header;
	char *value;
} http_header_t;

int ws_server(void);
ws_connection_t *accept_ws_connection(void);

int send_ws_frame(ws_connection_t *, char *buf, int len);
int receive_ws_frame(ws_connection_t *, char *buf, int len);

// http stuff
int parse_http_request(char *data, char *method, char *http_version, http_header_t **, int *count);
void build_http_response(char *http_response, int status_code, http_header_t *response_headers, int hcount);
