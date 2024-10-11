import websocket

# Funktion, die auf Nachrichten reagiert und diese ausgibt
def on_message(ws, message):
    print(f"Received message: {message}")

# Funktion, die auf Fehler reagiert
def on_error(ws, error):
    print(f"Error: {error}")

# Funktion, die auf Verbindungsaufbau reagiert
def on_open(ws):
    print("WebSocket connection opened.")

# Funktion, die auf Verbindungsabbruch reagiert
def on_close(ws, close_status_code, close_msg):
    print("WebSocket connection closed.")

# WebSocket-Konfiguration
if __name__ == "__main__":
    websocket.enableTrace(True)  # Aktiviert die Debug-Ausgabe

    # Initialisiere WebSocketApp und registriere Callback-Funktionen
    ws = websocket.WebSocketApp("ws://localhost:9999/websocket",
                                on_message=on_message,
                                on_error=on_error,
                                on_open=on_open,
                                on_close=on_close)

    # Starte die Verbindung und warte auf Nachrichten
    ws.run_forever()