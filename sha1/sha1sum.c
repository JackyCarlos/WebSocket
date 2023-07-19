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
	
	if (argc == 1) {
		int i, j = 0;
		char input[1000];
		while ((i = getchar()) != EOF && j < 10000)
			input[j++] = i;

		sha1Input((uint8_t *) input, j, &context);
		printf("%s\n", hexdigest(&context));

		return 0;
	}

	while (--argc) {
		FILE *f;

		if((f = fopen(*++argv, "r")) == NULL) {
			printf("sha1sum: can't open %s. No such file\n", *argv);
			continue;
		}

		fseek(f, 0L, SEEK_END);
		long size = ftell(f);
		fseek(f, 0L, SEEK_SET);
		
		int i, j = 0;
		char input[size];
		
		while ((i = getc(f)) != EOF)
			input[j++] = i;

		sha1Input((uint8_t *) input, size, &context);
		printf("%s  %s\n", hexdigest(&context), *argv);
		fclose(f);

		sha1Init(&context);
	}
	return 0;
}

char *hexdigest(sha1Context *context)
{
	int i;
	char *hexdigest = (char *) malloc(2 * HASHSIZE + 1);
	char temp[9];

	for (i = 0; i < 5; i++) {
		sprintf(temp, "%08x", context->intermediateHash[i]);
		strcat(hexdigest, temp);
	}

	return hexdigest;
}

/*
	sha1Context context;
	sha1Init(&context);

	char *test = "   ";
	unsigned char messageDigest[20];	

	sha1Input((uint8_t *) test, 3, &context);
	
	//printf("%s\n", hexdigest(&context));

	sha1Output(messageDigest, &context);
	char hexdigest[41];
	char temp[3];

	for (int i = 0; i < HASHSIZE; i++) {
		sprintf(temp, "%02x", messageDigest[i]);
		strcat(hexdigest, temp);
	}

	printf("%s\n", hexdigest);

	return 0;
*/
