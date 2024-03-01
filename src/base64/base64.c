#include <stdlib.h>
#include "base64.h"

static void create_output_alphabet(void);

static const uint8_t input_alphabet[] = { 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 
                                       'U', 'V', 'W', 'X', 'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 
                                       'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z', '0', '1', '2', '3', '4', '5', '6', '7', 
                                       '8', '9', '+', '/' };
static uint8_t *output_alphabet = NULL;

char * 
base64_encode(const char *input_data, uint32_t input_data_length) {
    uint32_t output_length, triple;
    int j, padding;
    char *output_data;
    
    output_length = (input_data_length + 2) / 3 * 4;
    output_data = (char *) malloc(output_length + 1);
    padding = input_data_length % 3;

    if (output_data == NULL) {
        return NULL;
    }

    j = 0;
    
    for (int i = 0; i < input_data_length;) {
        triple  = (i < input_data_length) ? input_data[i++] << 16 : 0;
        triple |= (i < input_data_length) ? input_data[i++] << 8 : 0;
        triple |= (i < input_data_length) ? input_data[i++] : 0;
     
        output_data[j++] = input_alphabet[triple >> 18 & 0x3F];
        output_data[j++] = input_alphabet[triple >> 12 & 0x3F];
        output_data[j++] = input_alphabet[triple >> 6 & 0x3F];
        output_data[j++] = input_alphabet[triple & 0x3F];
    }

    output_data[j] = '\0';

    for (int i = 0; padding && i < 3 - padding; ++i) {
        output_data[--j] = '=';
    }

    return output_data;
}

char *
base64_decode(const char *input_data, uint32_t input_data_length) {
    char *output_data;
    uint32_t output_length, quadruple;
    int j; 

    if (output_alphabet == NULL) {
        create_output_alphabet();
    }
    
    output_length = input_data_length / 4 * 3;

    while (input_data[input_data_length - 1] == '=') {
        --output_length;
        --input_data_length;
    }

    output_data = (char *) malloc(output_length + 1);

    if (output_data == NULL) {
        return NULL;
    }

    j = 0;

    for (int i = 0; i < input_data_length;) {
        quadruple  = output_alphabet[input_data[i++]] << 18;
        quadruple |= (i < input_data_length) ? output_alphabet[input_data[i++]] << 12 : 0;
        quadruple |= (i < input_data_length) ? output_alphabet[input_data[i++]] << 6 : 0;
        quadruple |= (i < input_data_length) ? output_alphabet[input_data[i++]] : 0;

        output_data[j++] = quadruple >> 16;
        output_data[j++] = quadruple >> 8 & 0xFF;
        output_data[j++] = quadruple & 0xFF;
    }

    output_data[j] = '\0';

    return output_data;
}

static void 
create_output_alphabet(void) {
    output_alphabet = (char *) malloc(256);
    if (output_alphabet == NULL) {
        return;
    }
    
    for (int i = 0; i < 64; ++i) {
        output_alphabet[input_alphabet[i]] = i; 
    }
}

