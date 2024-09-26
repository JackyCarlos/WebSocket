import websocket

websocket.enableTrace(True)
ws = websocket.create_connection("ws://localhost:9999/websocket")

#long_message = 200*"T"
inp = ""

while (1):
    inp = input("please send via ws: ")
    ws.send(inp)

