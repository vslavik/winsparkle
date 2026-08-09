/*
 *  This file is part of the dsa-verify library (https://github.com/marcizhu/dsa-verify)
 *
 *  Copyright (C) 2021 Marc Izquierdo
 *  Copyright (C) 2026 Vaclav Slavik
 *
 *  Permission is hereby granted, free of charge, to any person obtaining a
 *  copy of this software and associated documentation files (the "Software"),
 *  to deal in the Software without restriction, including without limitation
 *  the rights to use, copy, modify, merge, publish, distribute, sublicense,
 *  and/or sell copies of the Software, and to permit persons to whom the
 *  Software is furnished to do so, subject to the following conditions:
 *
 *  The above copyright notice and this permission notice shall be included in
 *  all copies or substantial portions of the Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 *  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 *  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 *  DEALINGS IN THE SOFTWARE.
 *
 */

#include <stddef.h>
#include <string.h>

#include "der.h"

#define BASE64_PAD '='

/* ASCII order for BASE 64 decode, 255 in unused character */
static const unsigned char base64de[] =
{
    /* nul, soh, stx, etx, eot, enq, ack, bel, */
       255, 255, 255, 255, 255, 255, 255, 255,
    /*  bs,  ht,  nl,  vt,  np,  cr,  so,  si, */
       255, 255, 255, 255, 255, 255, 255, 255,
    /* dle, dc1, dc2, dc3, dc4, nak, syn, etb, */
       255, 255, 255, 255, 255, 255, 255, 255,
    /* can,  em, sub, esc,  fs,  gs,  rs,  us, */
       255, 255, 255, 255, 255, 255, 255, 255,
    /*  sp, '!', '"', '#', '$', '%', '&', ''', */
       255, 255, 255, 255, 255, 255, 255, 255,
    /* '(', ')', '*', '+', ',', '-', '.', '/', */
       255, 255, 255,  62, 255, 255, 255,  63,
    /* '0', '1', '2', '3', '4', '5', '6', '7', */
        52,  53,  54,  55,  56,  57,  58,  59,
    /* '8', '9', ':', ';', '<', '=', '>', '?', */
        60,  61, 255, 255, 255, 255, 255, 255,
    /* '@', 'A', 'B', 'C', 'D', 'E', 'F', 'G', */
       255,   0,   1,  2,   3,   4,   5,    6,
    /* 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', */
         7,   8,   9,  10,  11,  12,  13,  14,
    /* 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', */
        15,  16,  17,  18,  19,  20,  21,  22,
    /* 'X', 'Y', 'Z', '[', '\', ']', '^', '_', */
        23,  24,  25, 255, 255, 255, 255, 255,
    /* '`', 'a', 'b', 'c', 'd', 'e', 'f', 'g', */
       255,  26,  27,  28,  29,  30,  31,  32,
    /* 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', */
        33,  34,  35,  36,  37,  38,  39,  40,
    /* 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', */
        41,  42,  43,  44,  45,  46,  47,  48,
    /* 'x', 'y', 'z', '{', '|', '}', '~', del, */
        49,  50,  51, 255, 255, 255, 255, 255
};

size_t base64_decode(const char* in, size_t inlen, unsigned char* out)
{
    size_t j = 0;
    size_t ignored = 0;

    for (size_t i = 0; i < inlen; i++)
    {
        if (in[i] == BASE64_PAD)
            break;

        if (in[i] == '\n' || in[i] == '\r' || in[i] == '\t' || in[i] == ' ')
        {
            ignored++;
            continue;
        }

        unsigned char c_in = (unsigned char)in[i];
        if (c_in >= sizeof(base64de))
            return 0;

        unsigned char c = base64de[c_in];
        if (c == 255)
            return 0;

        switch((i - ignored) & 0x3)
        {
            case 0:
                out[j] = (c << 2) & 0xFF;
                break;
            case 1:
                out[j++] |= (c >> 4) & 0x3;
                out[j] = (c & 0xF) << 4; 
                break;
            case 2:
                out[j++] |= (c >> 2) & 0xF;
                out[j] = (c & 0x3) << 6;
                break;
            case 3:
                out[j++] |= c;
                break;
        }
    }

    return j;
}

/* Parses DER length at given buffer position, storing it in length.
   Checks bounds and returns 1 if a valid length was parsed, 0 on invalid input. */
static int _parse_length(const unsigned char** der, const unsigned char *end, size_t *length)
{
    if (*der >= end)
        return 0;

    if(((**der & 0x80) >> 7) == 0)
    {
        (*der) += 1;
        *length = ((*der)[-1] & ~0x80);
        return *length <= (size_t)(end - *der);
    }

    else
    {
        unsigned int bytes = (*(*der)++ & ~0x80);
        if (bytes > sizeof(size_t))
            return 0;

        size_t ret = 0;

        for(unsigned int i = 0; i < bytes; i++)
        {
            if (*der >= end)
                return 0;
            ret = (ret << 8) | *(*der)++;
        }

        *length = ret;
        return *length <= (size_t)(end - *der);
    }

    *length = 0;
    return 1;
}

#define ASN1_EXPECT_TAG(tag)                      \
    {                                             \
        if (der >= end || *der != (tag))          \
            return 0;                             \
        der++;                                    \
    }

#define SEQUENCE          ASN1_EXPECT_TAG(0x30)
#define INTEGER           ASN1_EXPECT_TAG(0x02)
#define OBJECT_IDENTIFIER ASN1_EXPECT_TAG(0x06)
#define BIT_STRING        ASN1_EXPECT_TAG(0x03)

int parse_der_pubkey(const unsigned char* der, size_t len, mp_int* keyP, mp_int* keyQ, mp_int* keyG, mp_int* keyY)
{
    static unsigned char ansi_x9_57[] = { 0x2A, 0x86, 0x48, 0xCE, 0x38, 0x04, 0x01 };
    const unsigned char* end = der + len;

    SEQUENCE
    {
        size_t length = 0;
        if (!_parse_length(&der, end, &length))
            return 0;

        SEQUENCE
        {
            size_t length = 0;
            if (!_parse_length(&der, end, &length))
                return 0;

            OBJECT_IDENTIFIER
            {
                size_t length = 0;
                if (!_parse_length(&der, end, &length))
                    return 0;

                if (length != sizeof(ansi_x9_57) || (length == sizeof(ansi_x9_57) && memcmp(der, ansi_x9_57, length) != 0))
                    return 0;

                der += length;
            }

            SEQUENCE
            {
                size_t length = 0;
                if (!_parse_length(&der, end, &length))
                    return 0;

                INTEGER
                {
                    // p
                    size_t length = 0;
                    if (!_parse_length(&der, end, &length))
                        return 0;

                    if (mp_from_ubin(keyP, der, length) != MP_OKAY)
                        return 0;

                    der += length;
                }

                INTEGER
                {
                    // q
                    size_t length = 0;
                    if (!_parse_length(&der, end, &length))
                        return 0;

                    if (mp_from_ubin(keyQ, der, length) != MP_OKAY)
                        return 0;

                    der += length;
                }

                INTEGER
                {
                    // g
                    size_t length = 0;
                    if (!_parse_length(&der, end, &length))
                        return 0;

                    if (mp_from_ubin(keyG, der, length) != MP_OKAY)
                        return 0;

                    der += length;
                }
            }
        }

        BIT_STRING
        {
            size_t length = 0;
            if (!_parse_length(&der, end, &length))
                return 0;

            if (length == 0 || *der++ != 0)
                return 0;

            INTEGER
            {
                // y
                size_t length = 0;
                if (!_parse_length(&der, end, &length))
                    return 0;

                return (mp_from_ubin(keyY, der, length) == MP_OKAY);
            }
        }
    }

    return 0;
}

int parse_der_signature(const unsigned char* der, size_t len, mp_int* r, mp_int* s)
{
    const unsigned char* end = der + len;

    SEQUENCE
    {
        size_t length = 0;
        if (!_parse_length(&der, end, &length))
            return 0;

        INTEGER
        {
            // r
            size_t length = 0;
            if (!_parse_length(&der, end, &length))
                return 0;

            if (mp_from_ubin(r, der, length) != MP_OKAY)
                return 0;

            der += length;
        }

        INTEGER
        {
            // s
            size_t length = 0;
            if (!_parse_length(&der, end, &length))
                return 0;

            return (mp_from_ubin(s, der, length) == MP_OKAY);
        }
    }

    return 0;
}

size_t pem2der(const char* pem, size_t len, unsigned char* out)
{
    const char* end = pem + len;
    const char* s1 = strstr(pem, "-----BEGIN");
    const char* s2 = strstr(pem, "-----END");

    if(s1 == NULL)
        return 0;

    if(s2 == NULL)
        return 0;

    s1 += 10;

    while(s1 < end && *s1 != '-')
        s1++;

    while(s1 < end && *s1 == '-')
        s1++;

    if(*s1 == '\r') s1++;
    if(*s1 == '\n') s1++;

    if(s2 <= s1 || s2 > end)
        return 0;

    return base64_decode(s1, (size_t)(s2 - s1), (unsigned char*)out);
}
