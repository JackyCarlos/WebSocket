# WebSocket &#x1F50C;

websocket server implementation compliant to [RFC6455](https://www.rfc-editor.org/rfc/rfc6455.html)

## API

The library provides an event-driven API for implementing WebSocket servers. Applications react to connection, message, and disconnection events by implementing the corresponding callbacks.

### Starting a server

A WebSocket server can be started by specifying the host and port:

```c
ws_server("localhost", "9999");
```

`ws_server()` starts the server and listens for incoming WebSocket connections. Each accepted connection is handled in a separate thread.

### Event callbacks

Applications implement three callbacks to react to events occurring on the server:

```c
void on_connection(ws_connection_t *connection);
void on_message(ws_connection_t *connection);
void on_close(ws_connection_t *connection);
```

The provided `ws_connection_t *connection` parameter represents the corresponding connection and provides access to information received messages and the connected client:

```c
connection->remote_addr         // sockadd_storage struct containing information about the tcp connection
connection->message             // array containing bytes of the message
connection->message_type        // text type or binary type
connection->message_length      // length of the received message
```

`on_connection()` is called after a new WebSocket connection has been established.

`on_message()` is called whenever a complete WebSocket message has been received.

`on_close()` is called when a WebSocket connection is closed and can be used to perform connection-specific cleanup.

### Minimal example

```c
#include <arpa/inet.h>
#include <netinet/in.h>
#include "ws.h"

void on_connection(ws_connection_t *connection) {
    printf("somebody connected to the server :)\n");
    return;
}

void on_message(ws_connection_t *ws_connection)
{
    struct sockaddr_in *addr = (struct sockaddr_in *)&ws_connection->remote_addr;
    char ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &addr->sin_addr, ip, sizeof(ip));
    unsigned short port = ntohs(addr->sin_port);

    if (ws_connection->message_type == MESSAGE_TYPE_TXT) {
        printf("client from %s:%hu sent a message: %.*s\n", ip, port, ws_connection->message_length, ws_connection->message);
    }
}

int main() {
    printf("firing up server!\n");
    ws_server("localhost", "1312");

    pthread_exit(NULL);
    return 0;
}
```
