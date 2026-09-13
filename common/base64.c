#include "base64.h"
#include "assert.h"
#include "stdio.h"

uint8_t lut[] =
{'A','B','C','D','E','F','G','H','I','J','K','L','M','N','O','P','Q','R','S','T','U','V','W','X','Y','Z','a','b','c','d','e','f','g','h','i','j','k','l','m','n','o','p','q','r','s','t','u','v','w','x','y','z','0','1','2','3','4','5','6','7','8','9','-','_','='};

uint32_t BASE64_Encode(uint8_t * input_buffer,
        uint32_t input_len,
        uint8_t * output_buffer)
{
    (void)input_buffer;
    (void)input_len;
    (void)output_buffer;

    assert(input_buffer);
    assert(output_buffer);
    uint32_t out_len = 0;
    
    uint32_t jdx = 0;
    uint32_t kdx = 0;
    uint32_t mdx = 0;
    for(uint32_t idx = 0u; idx < input_len; idx+=3)
    {
        uint32_t word = 0u;
        uint32_t shift = 16;
        uint32_t kdx = 0;
        for(; (kdx < 3u) && (jdx < input_len);kdx++,jdx++)
        {
            word |= (input_buffer[jdx] << shift);
            shift -= 8;
        }
        uint8_t out[4];
        out[0] = word >> 18;
        out[1] = (word >> 12) & 0x3F;
        out[2] = (kdx > 1) ? (word >> 6) & 0x3F : 64;
        out[3] = (kdx > 2) ? word & 0x3F : 64;

        for(uint32_t ndx = 0; ndx < 4; ndx++)
        {
            output_buffer[mdx++] = lut[out[ndx]];
        }
    }
    return mdx;
}
