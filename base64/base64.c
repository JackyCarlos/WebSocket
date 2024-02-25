#include <stdlib.h>
#include <stdio.h>
#include "base64.h"

static const char input_alphabet[] = { 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 
                                       'U', 'V', 'W', 'X', 'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 
                                       'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z', '0', '1', '2', '3', '4', '5', '6', '7', 
                                       '8', '9', '+', '/' };

char *base64_encode(const char *input_data, uint32_t input_data_length) {
    uint32_t output_length, triple;
    int j;
    char *output_data;
    
    output_length = (input_data_length + 2) / 3 * 4;
    output_data = (char *) malloc(output_length + 1);

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

    for (int i = 0; i < 3 - input_data_length % 3; ++i) {
        output_data[--j] = '=';
    }

    return output_data;
}