#include "sha1.h"
#include <stdlib.h>

static uint32_t shift_left(uint32_t, const int);
static void generate_words(uint8_t [], uint32_t []);
static uint32_t f1(const uint32_t, const uint32_t, const uint32_t);
static uint32_t f2(const uint32_t, const uint32_t, const uint32_t);
static uint32_t f3(const uint32_t, const uint32_t, const uint32_t);

static const uint32_t K1 = 0x5a827999;
static const uint32_t K2 = 0x6ed9eba1;
static const uint32_t K3 = 0x8f1bbcdc;
static const uint32_t K4 = 0xca62c1d6;

void 
sha1_init(sha1_context_t *context) {
	context->intermediate_hash[0] = 0x67452301;
	context->intermediate_hash[1] = 0xefcdab89;
	context->intermediate_hash[2] = 0x98badcfe;
	context->intermediate_hash[3] = 0x10325476;
	context->intermediate_hash[4] = 0xc3d2e1f0;
	
	context->index = 0;
}

void 
sha1_input(uint8_t *message, uint64_t message_length, sha1_context_t *context) {
	int i;
	context->message_length = message_length * 8;

	for (i = 0; i < message_length; ++i) {
		context->message_block[context->index] = *message++;
		
		if (context->index == 63) {
			sha1_process_block(context);
		} else { 
			context->index++;
		}
	}

	sha1_pad_message(context);
} 

void 
sha1_process_block(sha1_context_t *context) {
	int i;
	uint32_t message_words[80];

	generate_words(context->message_block, message_words);

	uint32_t temp;
	uint32_t A = context->intermediate_hash[0], 
		 B = context->intermediate_hash[1],
 		 C = context->intermediate_hash[2], 
		 D = context->intermediate_hash[3], 
		 E = context->intermediate_hash[4];
		
	for (i = 0; i < 20; ++i) {
		temp = A;
		A = E + f1(B, C, D) + shift_left(A, 5) + message_words[i] + K1;
		E = D;
		D = C;
		C = shift_left(B, 30);
		B = temp;
	}
	
	for (i = 20; i < 40; ++i) {	
		temp = A;
		A = E + f2(B, C, D) + shift_left(A, 5) + message_words[i] + K2;
		E = D;
		D = C;
		C = shift_left(B, 30);
		B = temp;
	} 
	
	for (i = 40; i < 60; ++i) {	
		temp = A;
		A = E + f3(B, C, D) + shift_left(A, 5) + message_words[i] + K3;
		E = D;
		D = C;
		C = shift_left(B, 30);
		B = temp;
	} 
	
	for (i = 60; i < 80; ++i) {	
		temp = A;
		A = E + f2(B, C, D) + shift_left(A, 5) + message_words[i] + K4;
		E = D;
		D = C;
		C = shift_left(B, 30);
		B = temp;
	}
	
	context->intermediate_hash[0] += A;
	context->intermediate_hash[1] += B;
	context->intermediate_hash[2] += C;
	context->intermediate_hash[3] += D;
	context->intermediate_hash[4] += E;

	context->index = 0;		
}

void 
sha1_pad_message(sha1_context_t *context) {
	int i, j;
	
	context->message_block[context->index++] = 0x80;

	if (context->index > 56) {
		for (i = context->index; i < 64; ++i) {
			context->message_block[context->index++] = 0;
		}
		
		sha1_process_block(context);
	}

	for (i = context->index; i < 56; ++i) {
		context->message_block[context->index++] = 0;
	}

	for (j = 7; j >= 0; --j) {
		context->message_block[context->index++] = context->message_length >> j * 8;
	}

	sha1_process_block(context);	
}

void 
sha1_output(uint8_t *message_digest, sha1_context_t *context) {
	int i;
	for (i = 0; i < HASHSIZE; ++i) {
		message_digest[i] = context->intermediate_hash[i/4] >> (3 - i % 4) * 8; 	
	}
}

static uint32_t 
shift_left(uint32_t val, const int x) {
	return val << x | val >> (32 - x);
}

static void 
generate_words(uint8_t input_bytes[], uint32_t message_words[]) {
	int i, shift_value;
	
	for (i = 0; i < 16; ++i) {
		message_words[i] = 0;
	}

	for (i = 0; i < 64; ++i) {
		shift_value = (uint32_t) input_bytes[i] << (24 - (i % 4) * 8); 
		message_words[i/4] |= shift_value;
	}
	
	for (i = 16; i < 80; ++i) {
		message_words[i] = shift_left(message_words[i - 16] ^ message_words[i - 14] ^ message_words[i - 8] ^ message_words[i - 3], 1);
	}
}

static uint32_t 
f1(uint32_t B, uint32_t C, uint32_t D) {
	return (B & C) | (~B & D);
}

static uint32_t 
f2 (uint32_t B, uint32_t C, uint32_t D) {
	return B ^ C ^ D;
}

static uint32_t 
f3(uint32_t B, uint32_t C, uint32_t D) {
	return (B & C) | (B & D) | (C & D);
}
