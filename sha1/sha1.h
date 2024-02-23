#include <stdint.h>

#define 	HASHSIZE	20 		// hashsize in bytes

typedef struct {
	uint32_t 	intermediate_hash[HASHSIZE / 4];
	uint8_t 	message_block[64];		// array holding the next message block
	uint64_t 	message_length;	

	uint32_t 	index;					// index for position in message Block
} sha1_context_t;

void sha1_init(sha1_context_t *);
void sha1_process_block(sha1_context_t *);
void sha1_pad_message(sha1_context_t*);
void sha1_input(uint8_t *, uint64_t message_length, sha1_context_t *);
void sha1_output(uint8_t *, sha1_context_t *);
