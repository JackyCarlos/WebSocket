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

	for(int i = 0; i < MAX_CON; i++) 
		if (connections[i].status == INITIALIZING) {
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

void
ws_handshake(wsConnection *con)
{
	
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
		connections[i].status = INITIALIZING;
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

