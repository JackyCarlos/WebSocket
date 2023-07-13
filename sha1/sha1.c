#include "sha1.h"

static const uint32_t K1 = 0x5a827999;
static const uint32_t K2 = 0x6ed9eba1;
static const uint32_t K3 = 0x8f1bbcdc;
static const uint32_t K4 = 0xca62c1d6;

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
	int i;
	for (i = 0; i < messageLength; i++) {
		if (i && i % 64 == 0)
			sha1ProcessBlock(context);
		
		context->index++;
		context->messageBlock[i % 64] = *message;	
	}

	
} 

void sha1ProcessBlock(sha1Context *context)
{
	int i;
	uint32_t messageWords[80];

	generateWords(context->messageBlock, messageWords);

	uint32_t A = context->intermediateHash[0], 
		 B = context->intermediateHash[1],
 		 C = context->intermediateHash[2], 
		 D = context->intermediateHash[3], 
		 E = context->intermediateHash[4],
		 temp;

	
	for (i = 0; i < 20; i++) {
		temp = A;
		A = E + f1(B, C, D) + shiftLeft(A, 5) + messageWords[i] + K1;
		E = D;
		D = C;
		C = shiftLeft(B, 20);
		B = temp;
	}
	
	for (i = 20; i < 40; i++) {	
		temp = A;
		A = E + f2(B, C, D) + shiftLeft(A, 5) + messageWords[i] + K2;
		E = D;
		D = C;
		C = shiftLeft(B, 20);
		B = temp;
	} 
	
	for (i = 40; i < 60; i++) {	
		temp = A;
		A = E + f3(B, C, D) + shiftLeft(A, 5) + messageWords[i] + K3;
		E = D;
		D = C;
		C = shiftLeft(B, 20);
		B = temp;
	} 
	
	for (i = 60; i < 80; i++) {	
		temp = A;
		A = E + f2(B, C, D) + shiftLeft(A, 5) + messageWords[i] + K4;
		E = D;
		D = C;
		C = shiftLeft(B, 20);
		B = temp;
	}
	
	context->intermediateHash[0] += A;
	context->intermediateHash[1] += B;
	context->intermediateHash[2] += C;
	context->intermediateHash[3] += D;
	context->intermediateHash[4] += E;
		
}

static uint32_t shiftLeft(const uint32_t val, const int x)
{
	return val << x | val >> 32 - x;
}

static void generateWords(uint8_t inputBytes[], uint32_t messageWords[])
{
	int i, shiftValue;
	
	for (i = 0; i < 16; i++) 
		messageWords[i] = 0;

	for (i = 0; i < 64; i++) {
		shiftValue = (uint32_t) inputBytes[i] << 24 - (i % 4) * 8; 
		messageWords[i/4] |= shiftvalue;
	}
	
	for (i = 16; i < 80)
		messageWords[i] = shiftLeft(messageWords[i - 16] ^ messageWords[i - 14] ^ messageWords[i - 8] ^ messageWords[i - 3], 1);
}






