#include <pthread.h>
#include <stdio.h>
#include "ws.h"

void on_connection(ws_connection_t *connection) {
    printf("New connection established!\n");
}

void on_message(ws_connection_t *connection) {
    ;
}

int main() {
    ws_server("localhost", "9999");

    pthread_exit(NULL);

    return 0;
}
