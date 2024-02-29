#include "utils.h"

char *split(char *str, const char *delim) {	
	static char *string;
	char *str_to_return, *substr;
	int len, len_delim;

	if (str != NULL) {
		string = str;
	}

	if (*string == '\0') {
		return NULL;
	}

	if (delim == NULL || *delim == '\0') {
		str_to_return = string;
		len = strlen(string);
		string += len;
		
		return str_to_return;
	}

	substr = strstr(string, delim);
	
	if (substr == NULL) {
		str_to_return = string;

		len = strlen(string);
		string += len;
		
		return str_to_return;
	}	

	len_delim = strlen(delim);
	str_to_return = string;
	*substr = '\0';
	string += (strlen(string) + len_delim);

	return str_to_return;
}