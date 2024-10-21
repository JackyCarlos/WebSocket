import websocket
import time

# Funktion, die auf Nachrichten reagiert und diese ausgibt
def on_message(ws, message):
    print(f"Received message: {message}")

# Funktion, die auf Fehler reagiert
def on_error(ws, error):
    print(f"Error: {error}")

# Funktion, die auf Verbindungsaufbau reagiert
def on_open(ws):
    print("WebSocket connection opened.")
    
    # Beispiel: Schicke einen Ping nach dem Verbindungsaufbau
    def send_ping():
        while True:
            print("Sending Ping")
            ws.sock.ping()  # Schickt einen Ping-Frame an den Server
            time.sleep(10)  # Warte 10 Sekunden vor dem nächsten Ping

    # Starte einen neuen Thread, um Pings regelmäßig zu senden
    import threading
    ping_thread = threading.Thread(target=send_ping)
    ping_thread.start()

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
