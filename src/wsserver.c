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
static void init_connections(int);
static int ws_send_message(ws_connection_t *connection, uint8_t *message_bytes, uint64_t message_length, uint8_t message_type);
static void create_close_payload(int code, uint8_t *close_payload, int *close_reason_len);

static void *ws_server_listener_thread(void *);
static void *ws_connection_thread(void *);
static void thread_cleanup_handler(void *);

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

websocket_status_code_t websocket_codes[] = {
	{ 1000, "normal closure" },
	{ 1001, "going away" },
	{ 1002, "protocol error" },
	{ 1007, "data not consistent" },
	{ 1009, "message too big to process" },
	{ 1011, "unexpected serverside condition" }
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
	init_connections(0);

	rc = pthread_create(&listener_thread, NULL, ws_server_listener_thread, NULL);
	if (rc != 0) {
		perror("thread create error");
		return -1;
	}

	printf("websocket server created. Listening on %s:%s\n", host_address, port);

	return 0;
}

static void*
ws_server_listener_thread(void *param) {
	int newfd, rc;
	uint32_t thread_pos;
	socklen_t addrlen;
	struct sockaddr_storage remote_addr;
	ws_connection_t *connection;
	addrlen = sizeof(struct sockaddr_storage);

	for (;;) {
		thread_pos = 0;

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
		connection->message = NULL;

		pthread_t new_thread;
		rc = pthread_create(&new_thread, NULL, ws_connection_thread, (void *) connection);
		if (rc != 0) {
			// change connection state
			free(connection);
			perror("thread create error");
			continue;
		}		

		for (; thread_pos < max_con; ++thread_pos) {
			if (connections[thread_pos] == NULL || connections[thread_pos]->status == CLOSED) {
				break;
			}
		}

		if (thread_pos == max_con) {
			max_con += 10;
			connections = realloc(connections, sizeof(ws_connection_t *) * max_con);
			init_connections(thread_pos);
		}

		connection->thread_id = thread_pos;

		connections[thread_pos] = connection;
		con_count++;
	}

	return (void *) NULL;
}

static void*
ws_connection_thread(void *connection) {
	pthread_cleanup_push(thread_cleanup_handler, connection);

	ws_connection_t *ws_connection = (ws_connection_t *) connection;
	int status;

	status = -1;
	
	while (status != 0) {
		status = ws_handshake(connection);
		if (status == -2) {
			pthread_exit(NULL);
		}
	}

	on_connection(connection);

	ws_connection->status = OPEN;

	// 	0 if successful, -1 if the underlying socket got closed or an 
	//  other error occured, -2 if the client sent an unmasked frame,
	//  -3 if the client sent a message bigger than 64 KB       

	int val;
	for (;;) {
		val = ws_receive_message(ws_connection);		
		
		if (val == 0) {
			on_message(ws_connection);
			free(ws_connection->message);
			ws_connection->message = NULL;
		} else if (val == -1) {
			break;
		} else if (val == -2) {
			uint8_t close_payload[30];
			int close_payload_len;

			create_close_payload(1002, close_payload, &close_payload_len); 

			if(ws_send_message(ws_connection, close_payload, close_payload_len, OPCODE_CON_CLOSE) == -1) {
				break;
			}

			ws_connection->state = CLOSING;
			ws_connection->close_sent = 1;

			
			
		} 

	}

	pthread_cleanup_pop(1);

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

/**
 *  @brief                  receive a websocket message
 *
 *  @param ws_connection    ws_connection_t instance representing an open websocket connection 
 *  @return                 0 if successful, -1 if the underlying socket got closed or an 
 *                          	other error occured, -2 if the client sent an unmasked frame,
 *                          	-3 if the client sent a message bigger than 1 MB                                                      
 */
static int
ws_receive_message(ws_connection_t *ws_connection) {
	uint8_t frame_header[14], mask[4];
	uint8_t fin, rsv, op_code, masked, payload_start;
	unsigned long payload_length;
	int continuation_frame;

	continuation_frame = 0;
	ws_connection->message_length = 0;

	// infinite loop for receiving all frames of a message
	for (;;) {
		// evaluate status
		if (recv_bytes(ws_connection->fd, frame_header, 2) < 0) {
			// error receiving bytes from the socket, clean up the connection
			return -1; 
		}

		fin = frame_header[0] & 0x80;
		rsv = frame_header[0] & 0x70;
		op_code = frame_header[0] & 0x0F;
		masked = frame_header[1] & 0x80;

		// An unfragmented message consists of a single frame with the FIN
        // bit set (Section 5.2) and an opcode other than 0.
		if (masked == 0 || (continuation_frame == 1 && op_code != 0)) {
			// client sent an unmasked frame. consequence unknown
			return -2; 
		} 

		payload_length = frame_header[1] & 0x7F;
		payload_start = 6;

		if (payload_length == 126) {
			payload_start = 8;
		} else if (payload_length == 127) {
			payload_start = 14;
		}

		if (recv_bytes(ws_connection->fd, frame_header + 2, payload_start - 2) < 0) {
			// error receiving bytes from the socket, clean up the connection
			return -1; 
		}
		
		if (payload_length == 126) {
			payload_length = (long) frame_header[2] << 8 | (long) frame_header[3];
		} else if (payload_length == 127) {
			payload_length = 0;
			
			for (int i = 0; i < 8; ++i) {
				payload_length |= (long) frame_header[2 + i] << 8 * (7 - i);
			}
		}

		if (payload_length >= 0x0010000) {
			// server should reply with an error code indicating to close the connection because frame size is bigger than 1 MB
			return -3; 
		}

		if (continuation_frame == 0) {
			ws_connection->message = (uint8_t *) malloc(payload_length);
		} else {
			ws_connection->message = (uint8_t *) realloc(ws_connection->message, ws_connection->message_length + payload_length);
		}
	
		if (recv_bytes(ws_connection->fd, ws_connection->message + ws_connection->message_length, payload_length) < 0) {
			// error receiving bytes from the socket, clean up the connection
			return -1; 
		}

		ws_connection->message_length += payload_length;

		// get Masking-Key
		mask[0] = frame_header[payload_start - 4]; 
		mask[1] = frame_header[payload_start - 3];
		mask[2] = frame_header[payload_start - 2];
		mask[3] = frame_header[payload_start - 1];

		// unmask
		for (int i = 0; i < payload_length; ++i) {
			ws_connection->message[i] ^= mask[i % 4];
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
				// do a clean up of the connection		
				break;
			case OPCODE_PING:
				printf("4");
				// send pong frame
				break;
			case OPCODE_PONG:
				printf("5");
				break;
			default:
				// send close frame	in order to fail the underlaying connection	
				printf("6");
				break;
		}

		if (fin) {
			break;
		} else {
			continuation_frame = 1;
		}
	}

	return 0;
}

/**
 *  @brief		wrapper function to send UTF-8 encoded text                                                
 */
int 
send_ws_message_txt(ws_connection_t *connection, uint8_t *bytes, uint64_t length) {
	return ws_send_message(connection, bytes, length, OPCODE_TEXT);
}

/**
 *  @brief		wrapper function to send arbitrary binary data                                           
 */
int 
send_ws_message_bin(ws_connection_t *connection, uint8_t *bytes, uint64_t length) {
	return ws_send_message(connection, bytes, length, OPCODE_BINARY);
}

/**
 *  @brief						send ws message
 *
 *  @param connection 			the web socket connection struct  
 *  @param message_bytes		the bytes to be transmitted over the established websocket connection
 *  @param message_length		the amount of bytes to send. Most of the time this the length of the message bytes array
 *  @param message_type			the available op codes according to the RFC
 *  @return         			0 if the message has been transmitted successfully or -1 if the underlying connection has been closed                                                  
 */
static int 
ws_send_message(ws_connection_t *connection, uint8_t *message_bytes, uint64_t message_length, uint8_t message_type) {
	if (connection == NULL 
		|| connection->status == CONNECTING 
		|| (connection->status == CLOSING && connection->close_sent == 1)) { // hier fehlt, was wenn die Verbindung vom Peer geclosed wurde, soll es dann nur noch möglich sein ein Close Frame zu schicken? Ich würde sagen ja!

		return -1;
	}

	int frames; // amount of frames to send
	int header_len; 
	uint8_t frame_header[10];
	uint64_t payload_len;

	frames = message_length / MAX_FRAME_SIZE;
	frames += (message_length % MAX_FRAME_SIZE == 0) ? 0 : 1;
	header_len = 2; 

	for (int i = 0; i < frames; ++i) {
		printf("round: %d\n", i);

		// start packing
		frame_header[0] = (message_length <= MAX_FRAME_SIZE) ? 0x80 : 0;
		frame_header[0] |= (i == 0) ? message_type : 0x00;

		// || Buffer Size < 126  
		// MAX_FRAME_SIZE 0x1000
		if (message_length < 126 || MAX_FRAME_SIZE < 126) {
			frame_header[1] = payload_len = (message_length < MAX_FRAME_SIZE) ? message_length : MAX_FRAME_SIZE;
		} else if (message_length <= 0xFFFF || MAX_FRAME_SIZE <= 0xFFFF) {
			frame_header[1] = 126;
			header_len += 2;

			uint16_t extended_payload_len_16 = payload_len = (message_length < MAX_FRAME_SIZE) ? message_length : MAX_FRAME_SIZE;
			extended_payload_len_16 = htons(extended_payload_len_16 & 0xFFFF);
			memcpy(&frame_header[2], &extended_payload_len_16, sizeof(extended_payload_len_16));	
		} else {
			frame_header[1] = 127;
			header_len += 8;

			uint64_t extended_payload_len_64 = payload_len = (message_length < MAX_FRAME_SIZE) ? message_length : MAX_FRAME_SIZE;
			extended_payload_len_64 = htobe64(payload_len);
			memcpy(&frame_header[2], &extended_payload_len_64, sizeof(extended_payload_len_64));
		}

		// send frame header
		if (send(connection->fd, frame_header, header_len, 0) == -1) {
			perror("socket send");
			return -1;
		}
		
		// send frame payload; 
		if (send(connection->fd, message_bytes, payload_len, 0) == -1) {
			perror("socket send");
			return -1;
		}

		message_bytes += MAX_FRAME_SIZE;
		message_length -= MAX_FRAME_SIZE;
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

/**
 *  @brief						create the payload of a closing message. This message consists of a 2 byte unsigned integer and an utf-8 string
 *
 *  @param code					status code to be used	
 *  @param close_payload		array in which to store the payload
 *  @param close_reason_len		the length of the constructed payload
 */
void create_close_payload(int code, uint8_t *close_payload, int *close_payload_len) {
	int i;
	for (i = 0; i < sizeof(websocket_codes); ++i) {
		if (websocket_codes[i].code == code) {
			break;
		}
	}

	close_payload[0] = websocket_codes[i].code >> 8;
	close_payload[1] = websocket_codes[i].code & 0xFF;
	*close_payload_len = strlen(websocket_codes[i].reason);

	memcpy(close_payload + 2, websocket_codes[i].reason, *close_payload_len);
}

static void init_connections(int start_pos) {
	for (int i = start_pos; i < max_con; ++i) {
		connections[i] = NULL;
	}
}

static void thread_cleanup_handler(void *arg) {
	ws_connection_t *ws_connection = (ws_connection_t *) arg;

	connections[ws_connection->thread_id] = NULL;
	if (ws_connection->message != NULL)	free(ws_connection->message); 
	free(ws_connection->message);
	free(ws_connection);
}
