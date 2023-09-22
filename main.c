#include "ws.h"

int
main(int argc, char *argv[])
{
	/*
	if (ws_server() == -1)
		return -1;

	wsConnection *wsc = accept_ws_connection();
	*/

	char request[] = "GET /chat HTTP/1.1\r\n"
					 "Host: localhost:1234\r\n"
					 "Upgrade: websocket\r\n"
					 "Connection: Upgrade\r\n"
					 "Sec-WebSocket-Key: AQIDBAUGBwgJCgsMDQ4PEC==\r\n"
					 "Sec-WebSocket-Version: 13\r\n"
					 "Sec-WebSocket-Protocol: porn69\r\n"
					 "Sec-WebSocket-Extensions: wwf\r\n"
					 "Dummy-Header: trulala\r\n"
					 "\r\n"
					 "{test: belastend}\r\n";

	char method[20];
	HTTP_header http_headers[20];
	parse_http_request(request, method, http_headers);

	printf("http method is: %s\n", method);

	for (int i = 0; i < 9; i++) {
		printf("Header: %s --- Wert: %s\n", http_headers[i].header, http_headers[i].value);
	}

	/*
	printf("%s\n\n\n", request);
	
	char *found = strstr(request, "\r\n\r\n");
	*found = '\0';

	printf("%s", request);

	
	char *header = strtok(request, "\r\n");
	while (header != NULL) {
		printf("Header: %s\n", header);
		header = strtok(NULL, "\r\n");
	}
	*/
	
	return 0; 
}
