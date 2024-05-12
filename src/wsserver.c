/***************************************************************************//**

  @file         wsserver.c

  @author       Robert Eikmanns

  @date         Thursday, 7 March 2024

  @brief        core functionalities of the websocket server

*******************************************************************************/

#include "ws.h"
#include "sha1.h"
#include "base64.h"
#include "utils.h"
#include "http.h"

static int ws_handshake(ws_connection_t *);
static void build_accept_header(char *header, char *sec_websocket_key);
static int ws_receive_message(ws_connection_t *); 
static int recv_bytes(int fd, uint8_t *mem, uint32_t fetch_bytes);

static void *ws_server_listener_thread(void *);
static void *ws_connection_thread(void *);

static ws_connection_t **connections;
static int listening_fd;
static int con_count = 0, max_con = 10;

enum handshake_headers {
	HOST = 128, 
	UPGRADE = 64,
	CONNECTION = 32,
	WSKEY = 16,
	WSVERSION = 8,
	ORIGIN = 4
};

/**
 *  @brief                  create the websocket server
 *
 *  @param host_address     the host ip address to listen for incomming connections. May be NULL   
 *  @param port             the port to listen on, allowed values: 1024-65535   
 *  @return                 0 if creation was successful, or -1 in case of an error
 */
int 
ws_server(char *host_address, char *port) {
	int rc;
	pthread_t listener_thread;

	listening_fd = get_listener_socket(host_address, port);
	if (listening_fd < 0) {
		return -1;
	}

	connections = (ws_connection_t **) malloc(sizeof(ws_connection_t *) * max_con);
	if (connections == NULL) {
		return -1;
	}

	rc = pthread_create(&listener_thread, NULL, ws_server_listener_thread, NULL);
	if (rc != 0) {
		perror("thread create error");
		return -1;
	}

	return 0;
}

static void*
ws_server_listener_thread(void *param) {
	int newfd, rc;
	socklen_t addrlen;
	struct sockaddr_storage remote_addr;
	ws_connection_t *connection;
	addrlen = sizeof(struct sockaddr_storage);

	for (;;) {
		newfd = accept(listening_fd, (struct sockaddr *) &remote_addr, &addrlen);
		if (newfd == -1) {
			perror("accept error");
			continue;
		}

		connection = (ws_connection_t *) malloc(sizeof(ws_connection_t));
		if (connection == NULL) {
			return NULL;
		}

		connection->fd = newfd;
		connection->status = CONNECTING;
		connection->remote_addr = remote_addr;

		pthread_t new_thread;
		rc = pthread_create(&new_thread, NULL, ws_connection_thread, (void *) connection);
		if (rc != 0) {
			perror("thread create error");
			continue;
		}

		connection->thread = new_thread;
		printf("DEBUG\n");
		fflush(stdout);
		connections[con_count++] = connection;

		if(con_count == max_con) {
			max_con += 10;
			connections = realloc(connections, sizeof(ws_connection_t *) * max_con);
		}
	}

	return (void *) NULL;
}

static void*
ws_connection_thread(void *connection) {
	ws_connection_t *ws_connection = (ws_connection_t *) connection;

	int status;
	status = -1;
	
	while (status != 0) {
		status = ws_handshake(connection);
		if (status == -2) {
			// TODO: tidy up the connection
			pthread_exit(NULL);
		}
	}

	ws_connection->status = OPEN;
	printf("congrats, connection is established\n");

	for (;;) {
		ws_receive_message(ws_connection);
	}

	return (void *) NULL;
}

/*      0                   1                   2                   3
      0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
     +-+-+-+-+-------+-+-------------+-------------------------------+
     |F|R|R|R| opcode|M| Payload len |    Extended payload length    |
     |I|S|S|S|  (4)  |A|     (7)     |             (16/64)           |
     |N|V|V|V|       |S|             |   (if payload len==126/127)   |
     | |1|2|3|       |K|             |                               |
     +-+-+-+-+-------+-+-------------+ - - - - - - - - - - - - - - - +
     |     Extended payload length continued, if payload len == 127  |
     + - - - - - - - - - - - - - - - +-------------------------------+
     |                               |Masking-key, if MASK set to 1  |
     +-------------------------------+-------------------------------+
     | Masking-key (continued)       |          Payload Data         |
     +-------------------------------- - - - - - - - - - - - - - - - +
     :                     Payload Data continued ...                :
     + - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - +
     |                     Payload Data continued ...                |
     +---------------------------------------------------------------+
*/

static int
ws_receive_message(ws_connection_t *ws_connection) {
	uint8_t frame_header[14], mask[4];
	uint8_t fin, rsv, op_code, masked, payload_start;
	unsigned long payload_length;

	// infinite loop for receiving all frames of a message
	for (;;) {
		// evaluate status
		if (recv_bytes(ws_connection->fd, frame_header, 2) < 0) {
			return -3; // server should reply with an error code indicating to close the connection 
		}

		fin = frame_header[0] & 0x80;
		rsv = frame_header[0] & 0x70;
		op_code = frame_header[0] & 0x0F;
		masked = frame_header[1] & 0x80;

		if (masked == 0) {
			return -3; 
		}

		payload_length = frame_header[1] & 0x7F;
		payload_start = 6;

		if (payload_length == 126) {
			payload_start = 8;
		} else if (payload_length == 127) {
			payload_start = 14;
		}

		// evaluate status value
		if (recv_bytes(ws_connection->fd, frame_header + 2, payload_start - 2) < 0) {
			return -3; 
		}
		

		if (payload_length == 126) {
			payload_length = (long) frame_header[2] << 8 | (long) frame_header[3];
		} else if (payload_length == 127) {
			payload_length = 0;
			
			for (int i = 0; i < 8; ++i) {
				payload_length |= (long) frame_header[2 + i] << 8 * (7 - i);
			}
		}

		if (payload_length > 0x00100000) {
			// server should reply with an error code indicating to close the connection because frame size is bigger than 1 MB
			return -3; 
		}

		// get Masking-Key
		mask[0] = frame_header[payload_start - 4]; 
		mask[1] = frame_header[payload_start - 3];
		mask[2] = frame_header[payload_start - 2];
		mask[3] = frame_header[payload_start - 1];

		uint8_t frame_payload[payload_length + 1];

		// evaluate status value
		if (recv_bytes(ws_connection->fd, frame_payload, payload_length) < 0) {
			return -3;
		}

		frame_payload[payload_length] = '\0';

		// unmask
		for (int i = 0; i < payload_length; ++i) {
			frame_payload[i] ^= mask[i % 4];
		}

		switch (op_code) {
			case OPCODE_CONTINUATION: 
				printf("0");
				break;
			case OPCODE_TEXT:
				printf("1");
				break;
			case OPCODE_BINARY:
				printf("2");	
				break;
			case OPCODE_CON_CLOSE:
				printf("3");		
				break;
			case OPCODE_PING:
				printf("4");
				// send pong
				break;
			case OPCODE_PONG:
				printf("5");
				break;
			default:
				// send close frame	in order to fail the underlaying connection	
				printf("6");
				break;
		}

		printf("payload data: %s\n", (char *) frame_payload);
		exit(1);

		if (fin) {
			break;
		}
	}

	return 0;
}

static int recv_bytes(int fd, uint8_t *mem, uint32_t fetch_bytes) {
	uint32_t numbytes;

	numbytes = 0;
	while (fetch_bytes) {
		numbytes = recv(fd, mem, fetch_bytes, 0);
		
		if(numbytes == -1) {
			perror("socket recv");
			return -1;
		} else if (numbytes == 0) {
			return -2;
		}

		mem += numbytes;
		fetch_bytes -= numbytes;
	}

	return 0;
}

/**
 *  @brief          process the web socket handshake 
 *
 *  @param con      the web socket connection to use for the handshake. The connection is bound to an open tcp connection to the client   
 *  @return         0 if the server agrees to exchange data via the websocket connection, or -1 if there is an error when receiving data,
 *                     or -2 if the client closed the tcp connection, or -3 if the client sent a malformed http request, 
 *                     or -4 if the client used an unallowed http method in the request, or -5 in case of missing or corrupt request headers
 */
static int
ws_handshake(ws_connection_t *con) {
	char method[20], http_version[20], data[2048], http_response[200];
	http_header_t *request_headers, response_headers[3];
	int hcount, status, ec;
	char *sec_websocket_key;

	int numbytes;
	
	status = 0;

	numbytes = recv(con->fd, data, 2048, 0);
	if(numbytes == -1) {
		perror("socket recv");
		return -1;
	} else if (numbytes == 0) {
		return -2;
	}

	ec = parse_http_request(data, method, http_version, &request_headers, &hcount);
	if (ec == -1) {
		fprintf(stderr, "malformed http request\n");
		return -3;
	}

	if (strcmp(method, "GET") || !strcmp(http_version, "HTTP/1.0")) {
		fprintf(stderr, "wrong http method\n");
		response_headers[0] = (http_header_t) { "Allow", "GET" };
		build_http_response(http_response, 405, response_headers, 1);

		if (send(con->fd, http_response, strlen(http_response), 0) == -1) {
			perror("socket send");
		}

		return -4; 
	}

	for (int i = 0; i < hcount; ++i) {
		if (!strcmp(request_headers[i].header, "Host")) {
			status |= HOST;
		} else if (!(strcmp(request_headers[i].header, "Upgrade") || strcmp(request_headers[i].value, "websocket"))) {
			status |= UPGRADE;
		} else if (!(strcmp(request_headers[i].header, "Connection") || strcmp(request_headers[i].value, "Upgrade"))) {
			status |= CONNECTION;
		} else if (!strcmp(request_headers[i].header, "Sec-WebSocket-Key") && strlen(request_headers[i].value) == 24) {
			sec_websocket_key = request_headers[i].value;
			status |= WSKEY;
		} else if (!(strcmp(request_headers[i].header, "Sec-WebSocket-Version") || strcmp(request_headers[i].value, "13"))) {
			status |= WSVERSION;
		} else if (!strcmp(request_headers[i].header, "Origin")) {
			status |= ORIGIN;
		} else if (!strcmp(request_headers[i].header, "Sec-WebSocket-Protocol")) {
			;
		} if (!strcmp(request_headers[i].header, "Sec-WebSocket-Extension")) {
			;
		}
	}	

	if (status < 248) {
		fprintf(stderr, "bad request\n");
		response_headers[0] = (http_header_t) { "Sec-WebSocket-Version", "13" };
		((status & WSVERSION) == WSVERSION) ? build_http_response(http_response, 400, NULL, 0) : build_http_response(http_response, 426, response_headers, 1);
	
		if (send(con->fd, http_response, strlen(http_response), 0) == -1) {
			perror("socket send");
		}

		return -5;
	}

	char accept_header[29];
	build_accept_header(accept_header, sec_websocket_key);

	response_headers[0] = (http_header_t) { "Upgrade", "websocket" };
	response_headers[1] = (http_header_t) { "Connection", "Upgrade" };
	response_headers[2] = (http_header_t) { "Sec-WebSocket-Accept", accept_header };

	build_http_response(http_response, 101, response_headers, 3);
	if (send(con->fd, http_response, strlen(http_response), 0) == -1) {
		perror("socket send");
	}

	free(request_headers);
	return 0;
}

/**
 *  @brief                         build the value of the 'Sec-WebSocket-Accept' http response header
 *
 *  @param accept_header           array in which the generated header is to be saved 
 *  @param sec_websocket_accept    the value of the retrieved 'Sec-WebSocket-Key' http request header          
 */
static void build_accept_header(char *accept_header, char *sec_websocket_key) {
	char raw[61];
	uint8_t hash_bytes[20];
	sha1_context_t context;
	uint32_t len;
	
	strcpy(raw, sec_websocket_key);
	strcat(raw, GUID);
	
	// hashing
	sha1_init(&context);
	sha1_input((uint8_t *) raw, 60, &context);
	sha1_output(hash_bytes, &context);

	base64_encode(hash_bytes, 20, accept_header, &len);
}
