# WebSocket &#x1F50C;

A WebSocket server library written in C compliant to [RFC6455](https://www.rfc-editor.org/rfc/rfc6455.html). The project provides an event-driven API and supports multiple concurrent client connections. The WebSocket protocol implementation, including the opening handshake and frame processing, is implemented without relying on an external WebSocket library.

## Features

- RFC 6455 WebSocket server implementation
- Text and binary messages
- Fragmented messages
- Ping/Pong and Close control frames
- Concurrent client connections
- Event-driven API
- UTF-8 validation
- Protocol compliance tested with Autobahn

## API / Usage

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

This minimal example server informs the user when a client connects. For that it prints a message containing the clients ip address and the used port. The server then prints every text message it receives from the client and responds with a static thank you note. In case the client closes the websocket connection the server again informs the user by printing a message containing the clients ip address and the used port. 

```c
#include <arpa/inet.h>
#include <netinet/in.h>
#include "ws.h"

void get_client_address(
    ws_connection_t *ws_connection,
    char *ip,
    size_t ip_size,
    unsigned short *port
) {
    struct sockaddr_in *addr =
        (struct sockaddr_in *)&ws_connection->remote_addr;

    inet_ntop(AF_INET, &addr->sin_addr, ip, ip_size);
    *port = ntohs(addr->sin_port);
}

void on_connection(ws_connection_t *connection) {
    char ip[INET_ADDRSTRLEN];
    unsigned short port;

    get_client_address(connection, ip, sizeof(ip), &port);

    printf("%s:%hu has connected\n", ip, port);
}

void on_message(ws_connection_t *ws_connection)
{
    char ip[INET_ADDRSTRLEN];
    unsigned short port;

    get_client_address(ws_connection, ip, sizeof(ip), &port);

    if (ws_connection->message_type == MESSAGE_TYPE_TXT) {
        printf(
            "Client from %s:%hu sent a message: %.*s\n",
            ip,
            port,
            ws_connection->message_length,
            ws_connection->message
        );
    }

    send_ws_message_txt(ws_connection, (uint8_t *) "Thank you for your message", 26);
}

void on_close(ws_connection_t *ws_connection) {
    char ip[INET_ADDRSTRLEN];
    unsigned short port;

    get_client_address(ws_connection, ip, sizeof(ip), &port);

    printf("%s:%hu has disconnected\n", ip, port);
}

int main() {
    printf("firing up server!\n");

    ws_server("localhost", "9999");

    pthread_exit(NULL);

    return 0;
}
```

## Implementation

The server is implemented directly on top of TCP sockets without relying in an external WebSocket library. Each accepted client connection is represented by a `ws_connection_t` handle. And each client connection is handled in a separate thread. The connection thread performs the websocket handshake and subsequently processes incoming WebSocket frames for the lifetime of the connection.

### Opening Handshake 🤝

Before WebSocket communication can begin, the server performs an HTTP-based opening handshake. The incoming HTTP request, containing a header indicating the client's intention to switch protocols (`Upgrade: websocket`), is parsed and the client's `Sec-WebSocket-Key` value is combined with the WebSocket GUID. The resulting value is hashed with SHA-1 and Base64 encoded to generate the `Sec-WebSocket-Accept` response header. The server then responds with an `HTTP 101 Switching Protocols` response, completing the protocol upgrade. In doing so, the implementation makes use of implementations of Base64 and SHA-1 included in the project.

<Bild>

### Frame processing

After the handshake, communication is handled using websocket frames. Incoming frame headers are parsed to determine the opcode, payload length, masking information and fragmentation state. Client payloads are unmasked before being processed by the application.

Websocket messages may consist of a single frame or multiple fragmented frames. Fragmented messages are reconstructed and, in the case of text messages, validated for correct UTF-8 encoding before they are passed to the application through `on_message()`.

Control frames are handled separately from application data. `Ping` frames are answered with the corresponding `Pong` frame, while `Close` frames initiate connection termination.

## Protocol Compliance ✔️

The websocket protocol compliance is tested using the [autobahn-testsuite](https://github.com/crossbario/autobahn-testsuite/tree/b79698ff02cb967e1ed89ba730cf29a52d6b3f3d). The test suite checks whether the server implementation is compliant to RFC6455. Big thanks to [oberstet](https://github.com/oberstet) for this awesome testing tool.

First, build and start the test server:

```bash
make autobahn-test
./testing/autobahn-server
```

The test server runs on the host and is configured as an echo server for the autobahn tests. In a second terminal, change into the testing directory and create the report directory:

```bash
cd testing
mkdir -p reports
```

Run the autobahn fuzzing client using Docker:

```bash
docker run -it --rm \
    --network host \
    -v "$(pwd)/config:/config" \
    -v "$(pwd)/reports:/reports" \
    crossbario/autobahn-testsuite:latest \
    wstest -m fuzzingclient -s /config/fuzzingclient.json
```

The test configuration is located at `testing/config/fuzzingclient.json`. The websocket implementation is tested against all test cases except the limits/performance and compression test cases. After the test suite has finished, the generated report can be found at `testing/reports/servers/index.html`. Open the report in a browser to inspect the results of the individual WebSocket protocol test cases.

## Building 🏗️

Build the library using:

```bash
make
```

This creates the static library:

```bash
libwsserver.a
```

To use the library in another C project, include the public header:

```c
#include "ws.h"
```

Assuming the WebSocket repository is located inside your project:

```text
my-project/
├── main.c
└── WebSocket/
    ├── include/
    │   └── ws.h
    └── libwsserver.a
```

link against `libwsserver.a`:

```zsh
gcc main.c \
    -I./WebSocket/include \
    -L./WebSocket \
    -lwsserver \
    -lpthread \
    -o server
```

## Limitations

- No TLS / wss support
- websocket extensions are not supported
