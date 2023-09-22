/*
 *	wsserver.c -- a simple web socket server 
 */

#include "ws.h"

static wsConnection connections[MAX_CON];
static int listening_fd;
static int con_count = 0;

wsConnection
*accept_ws_connection()
{
	int newfd;
	socklen_t addrlen = sizeof(struct sockaddr_storage);
	wsConnection *con;

	for (int i = 0; i < MAX_CON; i++) 
		if (connections[i].status == CONNECTING) {
			con = &connections[i];
			break;
		}

	newfd = accept(listening_fd, (struct sockaddr *) &con->remoteaddr, &addrlen);
	if (newfd == -1)
		return NULL;
	
	con->fd = newfd;

	ws_handshake(con);

	return con;
}

int
ws_handshake(wsConnection *con)
{
	char method[20], data[2048];
	HTTP_header request_headers[20];
	int hcount, i, status;

	recv(con->fd, data, 2048, 0);
	parse_http_request(data, method, request_headers, &hcount);

	if (strcmp(method, "GET"))
		return -1; 

	for (i = 0; i < hcount; i++)
		if (!strcmp(request_headers[i].header, "Host")
			status |= HOST;
		else if (!strcmp(request_headers[i].header, "Upgrade") && !strcmp(request_headers[i].value, "websocket"))
			status |= UPGRADE;
		else if (!strcmp(request_headers[i].header, "Connection") && !strcmp(request_headers[i].value, "Upgrade"))
			status |= CONNECTION;
		else if (!strcmp(request_headers[i].header, "Sec-WebSocket-Key"))
			status |= WSKEY;
		else if (!strcmp(request_headers[i].header, "Sec-WebSocket-Version") && !strcmp(request_header[i].value, "13"))
			status |= WSVERSION
		else if (!strcmp(request_headers[i].header, "Origin"))
			status |= ORIGIN
		else if (!strcmp(request_headers[i].header, "Sec-WebSocket-Protocol"))
			;
		else if (!strcmp(request_headers[i].header, "Sec-WebSocket-Extension"))
			;	

	if (status < 248)
		return -2;
	
	con->status = OPEN;
	return 0;
}

int
ws_server() 
{
	listening_fd = get_listener_socket();
	if (listening_fd < 0)
		return -1;

	setup_connections();

	return 0;
}

static void 
setup_connections()
{
	for (int i = 0; i < MAX_CON; i++) {
		connections[i].status = CONNECTING;
	}
}


// returns -1 on failure, on success return created socket fd
int 
get_listener_socket(void)
{
	int listener, yes, rv;

	struct addrinfo hints, *ai, *p;

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	if ((rv = getaddrinfo(NULL, "9999", &hints, &ai)) != 0)
		return -1;

	for (p = ai; p != NULL; p = p->ai_next) {
		listener = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
		if (listener < 0)
			continue;

		setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));

		if (bind(listener, p->ai_addr, p->ai_addrlen) < 0) {
			close(listener);
			continue;
		}

		break;
	}

	freeaddrinfo(ai);
	if (p == NULL || listen(listener, 10) == -1)
		return -1;
	return listener;
}

