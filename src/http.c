/***************************************************************************//**

  @file         http.c

  @author       Robert Eikmanns

  @date         Thursday, 7 March 2024

  @brief        http parsing and building http responses

*******************************************************************************/

#include "ws.h"
#include "utils.h"

typedef struct {
	int status_code;
	char *information; 
} http_response_status_code_t;

http_response_status_code_t http_codes[] = {
	{ 400, "Bad Request" },
	{ 405, "Method Not Allowed" },
	{ 101, "Switching Protocols" },
	{ 426, "Upgrade Required" }
};

static char *http_response_base = "HTTP/1.1 %d %s\r\n"
						   		 "Server: null\r\n"
						   		 "Content-Length: 0\r\n";

/**
 *  @brief                      parse an http response and extract the corresponding headers
 *
 *  @param request              pointer to the raw request bytes
 *  @param method               pointer to the array where to store the given http method
 *  @param http_version         pointer to the array where to store the given http version
 *  @param request_headers      pointer to the array of request_headers of the calling instance
 *  @param count				pointer to the integer storing the amount of parsed headers of the calling instance
 *  @return                     0 if the parsing was successful, or -1 in case of a parsing error
 */
int 
parse_http_request(char *request, char *method, char *http_version, http_header_t **request_headers, int *count) {	
	char *request_line, *header_end, *raw_header;
	int header_count, max_header;
	char **raw_headers;

	header_count = 0;
	max_header = 20;
	raw_headers = malloc(max_header * sizeof(char *));
	
	// get rid of the http payload
	header_end = strstr(request, "\r\n\r\n");
	if (header_end == NULL) {
		return -1;
	}
	*header_end = '\0';

	request_line = split(request, "\r\n");
	if (request_line == NULL) {
		return -1;
	}

	raw_header = split(NULL, "\r\n");

	while (raw_header != NULL) {
		raw_headers[header_count++] = raw_header;

		if (header_count == max_header) {
			max_header += 10;
			raw_headers = realloc(raw_headers, max_header * sizeof(char *));
		} 
		raw_header = split(NULL, "\r\n");
	}
	*count = header_count;

	// extract http method
	strcpy(method, strtok(request_line, " "));
	strtok(NULL, " ");
	strcpy(http_version, strtok(NULL, " "));

	*request_headers = malloc(header_count * sizeof(http_header_t));

	for (int i = 0; i < header_count; ++i) {
		raw_header = raw_headers[i];
		(*request_headers)[i].header = split(raw_header, ": ");
		(*request_headers)[i].value = split(NULL, "");
	}

	free(raw_headers);
	return 0;
}

/**
 *  @brief                      build an http response out of a status code and some response headers
 *
 *  @param http_response        pointer to the array storing the build http response
 *  @param status_code          the http status code to send
 *  @param response_headers     pointer to response headers
 *  @param hcount               amount of headers to send
 */
void build_http_response(char *http_response, int status_code, http_header_t *response_headers, int hcount) {
	int i;

	if (!(status_code == 101 || status_code == 400 || status_code == 405 || status_code == 426)) {
		return;
	}

	for (i = 0; i < sizeof(http_codes); ++i) {
		if (http_codes[i].status_code == status_code) {
			break;
		}
	}

	sprintf(http_response, http_response_base, http_codes[i].status_code, http_codes[i].information);

	for (int j = 0; j < hcount; ++j) {
		char header[100];
		sprintf(header, "%s: %s\r\n", response_headers[j].header, response_headers[j].value);
		strcat(http_response, header);
	}

	strcat(http_response, "\r\n");
}

