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

#include "../debug/debug.h"

#define 	MAX_CON 		10
#define 	GUID			"258EAFA5-E914-47DA-95CA-C5AB0DC85B11"
#define 	MAX_FRAME_SIZE	0x0010000

enum ws_status {
	CONNECTING 	= 1,
	OPEN 		= 2,
	CLOSING 	= 4,
	CLOSED 		= 8
};

enum ws_op_code {
	OPCODE_CONTINUATION = 0x00,
	OPCODE_TEXT 		= 0x01,
	OPCODE_BINARY 		= 0x02,
	OPCODE_CON_CLOSE 	= 0x08,
	OPCODE_PING 		= 0x09,
	OPCODE_PONG 		= 0x0a
};

enum ws_message_type {
	MESSAGE_TYPE_TXT = 0x01,
	MESSAGE_TYPE_BIN = 0x02
};

typedef struct {
	uint16_t code;
	char *reason;
} websocket_status_code_t;

typedef struct {
	uint32_t fd;
	uint32_t status;
	struct sockaddr_storage remote_addr;
	int close_sent;
	uint8_t thread_id;

	uint8_t *message;
	uint64_t message_length;
	uint8_t message_type;
} ws_connection_t;


int ws_server(char *host_address, char *port);
ws_connection_t *accept_ws_connection(void);

// "user" space functions
void on_message(ws_connection_t *);
void on_connection(ws_connection_t *);
int send_ws_message_txt(ws_connection_t *, uint8_t *bytes, uint64_t length);
int send_ws_message_bin(ws_connection_t *, uint8_t *bytes, uint64_t length);
