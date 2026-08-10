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
