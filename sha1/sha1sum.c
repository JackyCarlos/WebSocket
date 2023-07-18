#include "sha1.h"
#include <stdint.h>
#include <stdio.h>

int main(int argc, char *argv[]) 
{
	sha1Context context;
	sha1Init(&context);

	char *test = "ABC";

	sha1Input((uint8_t *) test, (uint64_t) 3, &context);

	for (int i = 0; i < 5; i++)
		printf("%08x", context.intermediateHash[i]);
	printf("\n");

	return 0;
}
