#include "ws.h"

void 
parse_http_request(char *request, char *method, char *http_version, http_header_t **request_headers, int *count) 
{	
	char *request_line, *raw_header, *header;
	int header_count, max_header;
	char **raw_headers;

	header_count = 0;
	max_header = 20;
	raw_headers = malloc(max_header * sizeof(char *));
	
	// get rid of the http payload
	char *header_end = strstr(request, "\r\n\r\n");
	*header_end = '\0';

	request_line = strtok(request, "\r\n");
	raw_header = strtok(NULL, "\r\n");

	while (raw_header != NULL) {
		raw_headers[header_count++] = raw_header;

		if (header_count == max_header) {
			max_header += 10;
			raw_headers = realloc(raw_headers, max_header * sizeof(char *));
		} 
		raw_header = strtok(NULL, "\r\n");
	}
	*count = header_count;

	// extract http method
	strcpy(method, strtok(request_line, " "));
	strtok(NULL, " ");
	strcpy(http_version, strtok(NULL, " "));

	*request_headers = malloc(header_count * sizeof(http_header_t));

	for (int i = 0; i < header_count; ++i) {
		raw_header = raw_headers[i];
		(*request_headers)[i].header = strtok(raw_header, ": ");
		(*request_headers)[i].value = strtok(NULL, "");
	}

	free(raw_headers);
}

void build_http_reponse(int status_code, http_header_t *reponse_headers) {
	
	
	return;
}
