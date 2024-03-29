/***************************************************************************//**

  @file         utils.c

  @author       Robert Eikmanns

  @date         Thursday, 7 March 2024

  @brief        Implementations of helper functions

*******************************************************************************/

#include <stddef.h>
#include <string.h>
#include <netdb.h>
#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include "utils.h"

/**
 *  @brief                      split a string on a given string
 *
 *  @param str                  pointer to the string to be split
 *  @param delim                pointer to the delimiter string
 */
char 
*split(char *str, const char *delim) {	
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

/**
 *  @brief                         create a socket listening for incoming tcp connections 
 *
 *  @param host_address            the host ip address to listen for incomming connections. May be NULL
 *  @param port                    the port to listen on, allowed values: 1024-65535     
 */
int 
get_listener_socket(char *host_address, char *port) {
	int listener, yes, rv;
	struct addrinfo hints, *ai, *p;

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	if (host_address == NULL) hints.ai_flags = AI_PASSIVE;

	rv = getaddrinfo(host_address, port, &hints, &ai);

	if (rv != 0) {
		fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(errno));
		return -1;
	}

	for (p = ai; p != NULL; p = p->ai_next) {
		listener = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
		if (listener < 0) {
			perror("socket error");
			continue;
		}

		if(setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
			perror("setsockopt error");
			close(listener);
			return -1;
		}

		if (bind(listener, p->ai_addr, p->ai_addrlen) < 0) {
			perror("bind error");
			close(listener);
			continue;
		}

		break;
	}

	freeaddrinfo(ai);
	if (p == NULL) {
		return -1;
	}

	if (listen(listener, 10) == -1) {
		fprintf(stderr, "listen error: %s\n", strerror(errno));
	}

	return listener;
}
