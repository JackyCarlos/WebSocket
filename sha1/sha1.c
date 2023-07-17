#include "sha1.h"

static uint32_t shiftLeft(uint32_t, const int);
static void generateWords(uint8_t [], uint32_t []);
static uint32_t f1(uint32_t, uint32_t, uint32_t);
static uint32_t f2(uint32_t, uint32_t, uint32_t);
static uint32_t f3(uint32_t, uint32_t, uint32_t);

static const uint32_t K1 = 0x5a827999;
static const uint32_t K2 = 0x6ed9eba1;
static const uint32_t K3 = 0x8f1bbcdc;
static const uint32_t K4 = 0xca62c1d6;

void sha1Init(sha1Context *context) 
{
	context->intermediateHash[0] = 0x67452301;
	context->intermediateHash[1] = 0xefcdaB89;
	context->intermediateHash[2] = 0x98badcfe;
	context->intermediateHash[3] = 0x10325476;
	context->intermediateHash[4] = 0xc3d2e1f0;
	
	context->index = 0;
}

void sha1Input(uint8_t *message, uint64_t messageLength, sha1Context *context) 
{
	int i;
	context->messageLength = messageLength * 8;

	for (i = 0; i < messageLength; i++) {
		context->messageBlock[context->index] = *message++;
		
		if (context->index == 63)
			sha1ProcessBlock(context);
		else 
			context->index++;
	}

	sha1PadMessage(context);
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
		C = shiftLeft(B, 30);
		B = temp;
	}
	
	for (i = 20; i < 40; i++) {	
		temp = A;
		A = E + f2(B, C, D) + shiftLeft(A, 5) + messageWords[i] + K2;
		E = D;
		D = C;
		C = shiftLeft(B, 30);
		B = temp;
	} 
	
	for (i = 40; i < 60; i++) {	
		temp = A;
		A = E + f3(B, C, D) + shiftLeft(A, 5) + messageWords[i] + K3;
		E = D;
		D = C;
		C = shiftLeft(B, 30);
		B = temp;
	} 
	
	for (i = 60; i < 80; i++) {	
		temp = A;
		A = E + f2(B, C, D) + shiftLeft(A, 5) + messageWords[i] + K4;
		E = D;
		D = C;
		C = shiftLeft(B, 30);
		B = temp;
	}
	
	context->intermediateHash[0] += A;
	context->intermediateHash[1] += B;
	context->intermediateHash[2] += C;
	context->intermediateHash[3] += D;
	context->intermediateHash[4] += E;

	context->index = 0;		
}

void sha1PadMessage(sha1Context *context)
{
	int i;
	
	context->messageBlock[context->index++] = 0x80;

	if (context->index > 56) {
		for (i = context->index; i < 64; i++)
			context->message[context->index++]
		
		sha1ProcessBlock(context);
	}

	for (i = context->index; i < 56; i++)
		context->messageBlock[context->index++] = 0;

	// message length to bytes functionality

	sha1ProcessBlock(context);	
}

static uint32_t shiftLeft(uint32_t val, const int x)
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
		messageWords[i/4] |= shiftValue;
	}
	
	for (i = 16; i < 80; i++)
		messageWords[i] = shiftLeft(messageWords[i - 16] ^ messageWords[i - 14] ^ messageWords[i - 8] ^ messageWords[i - 3], 1);
}

static uint32_t f1(uint32_t B, uint32_t C, uint32_t D) 
{
	return B & C | ~B & D;
}

static uint32_t f2 (uint32_t B, uint32_t C, uint32_t D) 
{
	return B ^ C ^ D;
}

static uint32_t f3(uint32_t B, uint32_t C, uint32_t D) 
{
	return B & C | B & D | C & D;
}

