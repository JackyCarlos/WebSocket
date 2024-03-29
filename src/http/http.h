typedef struct {
	char *header;
	char *value;
} http_header_t;

int parse_http_request(char *data, char *method, char *http_version, http_header_t **, int *count);
void build_http_response(char *http_response, int status_code, http_header_t *response_headers, int hcount);