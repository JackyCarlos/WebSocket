#include "ws.h"

int
main(int argc, char *argv[])
{
	if (ws_server("localhost", "9999") == -1) {
		return -1;
	}
		
	

	pthread_exit(NULL);
}

void on_connection(ws_connection_t *connection) {
	printf("new connection!\n");
}

void on_message(ws_connection_t *connection) {
	printf("message received: %s\n", connection->message);
}
