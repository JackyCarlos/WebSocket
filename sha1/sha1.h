#include <stdint.h>

#define 	HASHSIZE	20 		// hashsize in bytes

typedef struct {
	uint32_t 	intermediateHash[HASHSIZE / 4];
	uint8_t 	messageBlock[64];			// array holding the next message block

} sha1Context;

void sha1Init(sha1Context *);
void sha1ProcessBlock(sha1Context *);
void sha1PadMessage(sha1Context *);
void sha1Input(uint8_t *message, sha1Context *);
void sha1Output(sha1Context *);
