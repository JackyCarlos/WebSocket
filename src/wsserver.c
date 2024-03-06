/*
 *	wsserver.c -- a simple web socket server 
 */

#include "ws.h"
#include "sha1.h"
#include "base64.h"

static int get_listener_socket(void);
static void build_accept_header(char *header, char *sec_websocket_key);

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

int
ws_server(void) {
	listening_fd = get_listener_socket();
	if (listening_fd < 0) {
		return -1;
	}

	connections = malloc(sizeof(ws_connection_t *) * max_con);

	return 0;
}

ws_connection_t
*accept_ws_connection(void) {	
	int newfd;
	socklen_t addrlen;
	struct sockaddr_storage remote_addr;
	ws_connection_t *connection;

	addrlen = sizeof(struct sockaddr_storage);

	newfd = accept(listening_fd, (struct sockaddr *) &remote_addr, &addrlen);
	if (newfd == -1) {
		perror("accept error");
		return NULL;
	}

	connection = (ws_connection_t *) malloc(sizeof(ws_connection_t));
	connection->fd = newfd;
	connection->status = CONNECTING;
	connection->remote_addr = remote_addr;

	printf("success?? %d\n", ws_handshake(connection));

	connections[con_count++] = connection;

	if(con_count == max_con) {
		max_con += 10;
		connections = realloc(connections, sizeof(ws_connection_t *) * max_con);
	}

	return connection;
}

int
ws_handshake(ws_connection_t *con) {
	char method[20], http_version[20], data[2048], http_response[200];
	http_header_t *request_headers, response_headers[3];
	int hcount, status, ec;
	char *sec_websocket_key;
	
	status = 0;

	if(recv(con->fd, data, 2048, 0) == -1) {
		perror("socket recv");
		return -1;
	}

	ec = parse_http_request(data, method, http_version, &request_headers, &hcount);
	if (ec == -1) {
		fprintf(stderr, "malformed http request\n");
		return -2;
	}

	if (strcmp(method, "GET") || !strcmp(http_version, "HTTP/1.0")) {
		fprintf(stderr, "wrong http method\n");
		response_headers[0] = (http_header_t) { "Allow", "GET" };
		build_http_response(http_response, 405, response_headers, 1);
		
		if (send(con->fd, http_response, strlen(http_response), 0) == -1) {
			perror("socket send");
		}

		return -3; 
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

		return -4;
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

	con->status = OPEN;
	free(request_headers);
	return 0;
}

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

int 
get_listener_socket(void) {
	int listener, yes, rv;
	struct addrinfo hints, *ai, *p;

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	rv = getaddrinfo(NULL, "9999", &hints, &ai);

	if (rv != 0) {
		fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(errno));
		return -1;
	}

	for (p = ai; p != NULL; p = p->ai_next) {
		listener = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
		if (listener < 0) {
			perror("socket error");
			continue;
		}

		if(setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
			perror("setsockopt error");
			close(listener);
			return -1;
		}

		if (bind(listener, p->ai_addr, p->ai_addrlen) < 0) {
			perror("bind error");
			close(listener);
			continue;
		}

		break;
	}

	freeaddrinfo(ai);
	if (p == NULL) {
		return -1;
	}

	if (listen(listener, 10) == -1) {
		fprintf(stderr, "listen error: %s\n", strerror(errno));
	}

	return listener;
}