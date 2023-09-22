#include "ws.h"

void 
parse_http_request(char *request, char *method, HTTP_header *request_headers) 
{
	char *request_line, *raw_header, *header;
	int pos;
	char *raw_headers[20];
	
	// get rid of the http payload
	char *header_end = strstr(request, "\r\n\r\n");
	*header_end = '\0';

	pos = -1;
	raw_header = strtok(request, "\r\n");
	while (raw_header != NULL) {
		if (pos == -1) {
			request_line = raw_header;
			pos++;	
		} else 
			raw_headers[pos++] = raw_header;
		raw_header = strtok(NULL, "\r\n");
	}

	// extract http method
	// strcpy(method, strtok(request_line, " "));
	strcpy(method, strtok(request_line, " "));

	for (int i = 0; i < pos; i++) {
		raw_header = raw_headers[i];
		request_headers[i].header = strtok(raw_header, ": ");
		request_headers[i].value = strtok(NULL, "");
	}
}
