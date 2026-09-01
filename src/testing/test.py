import websocket

websocket.enableTrace(True)

ws = websocket.create_connection("ws://localhost:9999/websocket")

while True:
    inp = input("please send via ws: ")

    if inp == "control":
        ws.ping("hello")

        frame = ws.recv_frame()

        print("received opcode:", frame.opcode)
        print("received data:", frame.data)
    else:
        ws.send(inp)
        response = ws.recv()
        print("server:", response)