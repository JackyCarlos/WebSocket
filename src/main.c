#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "ws.h"

// Globale Variable für die Verbindung
ws_connection_t *global_connection;

// Diese Funktion wird von der WebSocket-Implementierung aufgerufen, wenn eine neue Verbindung hergestellt wird
void on_connection(ws_connection_t *connection) {
    printf("New connection established!\n");
    global_connection = connection;  // Speichere die Verbindung global, damit andere darauf zugreifen können
}

// Diese Funktion wird aufgerufen, wenn eine Nachricht empfangen wird
void on_message(ws_connection_t *connection) {
    printf("Message received: %s\n", connection->message);
}

// Funktion, die in einem separaten Thread läuft und auf Benutzereingaben wartet, um Nachrichten zu senden
void *input_thread_func(void *arg) {
    char input[256];
    
    while (1) {
        // Eingabe des Benutzers lesen
        if (fgets(input, sizeof(input), stdin) != NULL) {
            // Entferne das '\n' am Ende der Eingabe
            input[strcspn(input, "\n")] = '\0';

            // Nachrichten nur senden, wenn eine Verbindung besteht
            if (send_ws_message_txt(global_connection, (uint8_t *) input, strlen(input)) == -1) {
                printf("No active connection. Waiting for a connection...\n");
            }
        }   
    }

    return NULL;
}

int main() {
    global_connection = NULL;
    pthread_t input_thread;

    // Starte den Thread, der auf Benutzereingaben wartet
    pthread_create(&input_thread, NULL, input_thread_func, NULL);

    // Hier wird die WebSocket-Implementierung initialisiert und auf Verbindungen gewartet.
    // Sobald eine Verbindung hergestellt wird, wird die Funktion `on_connection()` aufgerufen.
    ws_server("localhost", "9999");

    // Warte auf die Beendigung des Eingabe-Threads (in der Praxis läuft das Programm endlos)
    pthread_join(input_thread, NULL);

    return 0;
}
