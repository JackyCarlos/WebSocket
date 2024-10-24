#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include "ws.h"

void on_connection(ws_connection_t *connection) {
    printf("New connection established!\n");
}

void on_message(ws_connection_t *ws_connection) {
    if (ws_connection->message_type == MESSAGE_TYPE_TXT) {
        DEBUG_PRINT("sending text message\n");
        if (send_ws_message_txt(ws_connection, ws_connection->message, ws_connection->message_length) == -1) {
            if (errno == EPIPE) {
                printf("Error: Broken pipe (EPIPE) - the connection was closed.\n");
            } else {
                perror("send error");
            }
        }
    } else if (ws_connection->message_type == MESSAGE_TYPE_BIN) {
        DEBUG_PRINT("sending binary message\n");
        if (send_ws_message_bin(ws_connection, ws_connection->message, ws_connection->message_length) == -1) {
            if (errno == EPIPE) {
                printf("Error: Broken pipe (EPIPE) - the connection was closed.\n");
            } else {
                perror("send error");
            }
        }
    }
}

int main() {
    signal(SIGPIPE, SIG_IGN);
    ws_server("localhost", "9999");

    pthread_exit(NULL);

    return 0;
}
