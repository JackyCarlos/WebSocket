#include "sha1.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>


char *hexdigest(sha1Context *);

int main(int argc, char *argv[]) 
{
	sha1Context context;
	sha1Init(&context);

	char *test = "\0";
	unsigned char messageDigest[20];	

	sha1Input((uint8_t *) test, (uint64_t) 0, &context);
	
	printf("%s\n", hexdigest(&context));

	return 0;
}

char *hexdigest(sha1Context *context)
{
	int i;
	char *hexdigest = (char *) malloc(HASHSIZE + 1000);
	char temp[9];

	for (i = 0; i < 5; i++) {
		sprintf(temp, "%08x", context->intermediateHash[i]);
		strcat(hexdigest, temp);
	}

	return hexdigest;
}
