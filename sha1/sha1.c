#include "sha1.h"

int main(int argc, char *argv[])
{
	sha1context *test;
	return 0;
}

void sha1Init(sha1Context *context) 
{
	context->intermediateHash[0] = 0x67452301;
	context->intermediateHash[1] = 0xefcdaB89;
	context->intermediateHash[2] = 0x98badcfe;
	context->intermediateHash[3] = 0x10325476;
	context->intermediateHash[4] = 0xc3d2e1f0;
}

void sha1Input(uint8_t *message, uint32_t messageLength, sha1Context *context) 
{
	for (int i = 0; i < messageLength; i++) {
		if (i && i % 64 == 0)
			sha1ProcessBlock(context);
		
		context->messageBlock[i % 64] = *message;	
	}

	
} 
