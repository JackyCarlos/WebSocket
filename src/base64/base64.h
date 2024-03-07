/***************************************************************************//**

  @file         base64.h

  @author       Robert Eikmanns

  @date         Thursday, 7 March 2024

  @brief        Declarations for base64 implementaion

*******************************************************************************/

#include <stdint.h>

void base64_encode(const uint8_t *input_data, const uint32_t input_data_length, char *output_data, uint32_t *output_data_length);
void base64_decode(const char *input_data, uint32_t input_data_length, uint8_t *output_data, uint32_t *output_data_length);
