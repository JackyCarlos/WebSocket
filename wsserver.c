/*
 *	wsserver.c -- simple web socket server dealing with multiple ws connections simultaneously
 */


/*
 * geplantes Interface: 
 * createWsServer(); 			starte einen neuen Server mit eine listening socket 			Return value: fd listening socket
 * acceptWsConnection(); 		akzeptiere neue Connections auf dem server						Return value: fd connection
 * sendData();					sende Daten auf einer bestimmten Connection						Return value: success wenn Senden von Daten erfolgreich war
 * recvData();					empfange Daten auf einer bestimmten Connection 					Return value: success wenn Lesen von Daten erfolgreich war
 *
 * */

#include "ws.h"

unsigned serverID = 0;



wsConnection
*acceptWsConnection(wsServer *server)
{
	int newfd;
	socklen_t addrlen = sizeof(struct sockaddr_storage);
	wsConnection *con;

	for(int i = 0; i < server->connMax; i++) 
		if (server->connections[i].status == INITIALIZING) {
			con = &server->connections[i];
			break;
		}

	newfd = accept(server->listeningSfd, (struct sockaddr *) &con->remoteaddr, &addrlen);
	if (newfd == -1)
		return NULL;
	
	con->fd = newfd;

	return con;
}

wsServer 
*createWsServer(void) 
{
	int listener;

	wsServer *server = (wsServer *) malloc(sizeof (wsServer));
	server->connections = (wsConnection *) malloc(sizeof (wsConnection) * server->connMax);

	server->listeningSfd = get_listener_socket();

	if (server->listeningSfd < 0)
		return NULL;

	server->id = serverID++;
	setupConnections(server);

	return server;
}

void 
setupConnections(wsServer *server)
{

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











