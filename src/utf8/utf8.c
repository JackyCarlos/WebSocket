#include <stdint.h>
#include <stddef.h>
#include <stdio.h>

int 
chr_isvalid(uint32_t c)
{
  if (c <= 0x7F) return 1;
  if (0xC080 == c) return 1;   // Accept 0xC080 as representation for '\0'
  if (0xC280 <= c && c <= 0xDFBF) return ((c & 0xE0C0) == 0xC080);
  if (0xEDA080 <= c && c <= 0xEDBFBF) return 0; // Reject UTF-16 surrogates
  if (0xE0A080 <= c && c <= 0xEFBFBF) return ((c & 0xF0C0C0) == 0xE08080);
  if (0xF0908080 <= c && c <= 0xF48FBFBF) return ((c & 0xF8C0C0C0) == 0xF0808080);
  
  return 0;
}

int 
is_valid_utf8(const uint8_t *data, size_t len) {
    size_t i = 0;
    while (i < len) {
        uint8_t byte = data[i];
        uint32_t codepoint = 0;
        size_t remaining = len - i;

        if (byte <= 0x7F) {
            codepoint = byte;
            i += 1;
        } else if ((byte & 0xE0) == 0xC0 && remaining >= 2) {
            uint8_t b1 = data[i + 1];
            if ((b1 & 0xC0) != 0x80) return 0; 
        
            uint32_t codepoint = ((byte & 0x1F) << 6) | (b1 & 0x3F);
            if (codepoint < 0x80) return 0; 
        
            i += 2;
        } else if ((byte & 0xF0) == 0xE0 && remaining >= 3) {
            codepoint = (data[i] << 16) | (data[i + 1] << 8) | data[i + 2];
            i += 3;
        } else if ((byte & 0xF8) == 0xF0 && remaining >= 4) {
            codepoint = (data[i] << 24) | (data[i + 1] << 16) | (data[i + 2] << 8) | data[i + 3];
            i += 4;
        } else {
            return 0; 
        }

        if (!chr_isvalid(codepoint)) {
            return 0; 
        }
    }

    return 1; 
}
