#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <netdb.h>
#include <poll.h>
#include <stdint.h>
#include <pthread.h>

#define 	MAX_CON 	10
#define 	GUID		"258EAFA5-E914-47DA-95CA-C5AB0DC85B11"

enum ws_status {
	CONNECTING 	= 1,
	OPEN 		= 2,
	CLOSING 	= 4,
	CLOSED 		= 8
};

enum ws_op_code {
	OPCODE_CONTINUATION = 0x00,
	OPCODE_TEXT = 0x01,
	OPCODE_BINARY = 0x02,
	OPCODE_CON_CLOSE = 0x03,
	OPCODE_PING = 0x08,
	OPCODE_PONG = 0x0a
};

typedef struct {
	uint32_t fd;
	uint32_t status;
	struct sockaddr_storage remote_addr;
	pthread_t thread;
} ws_connection_t;

typedef struct {
	
} ws_packet_t;

int ws_server(char *host_address, char *port);
ws_connection_t *accept_ws_connection(void);

int send_ws_frame(ws_connection_t *, char *buf, int len);
int receive_ws_frame(ws_connection_t *, char *buf, int len);
