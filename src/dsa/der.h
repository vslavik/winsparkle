/*
 *  This file is part of the dsa-verify library (https://github.com/marcizhu/dsa-verify)
 *
 *  Copyright (C) 2021 Marc Izquierdo
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

#ifndef _PEM2DER_
#define _PEM2DER_

#include <stddef.h>

#include "mp_math.h"

/**
 * @brief Returns an upper bound of the number of bytes used by a base-64 encoded string
 *
 * Unlike decoders that only write completed output bytes, base64_decode()
 * eagerly initializes the next output byte while processing a partial
 * four-character group. Round up to a complete group so those writes fit.
 */
#define BASE64_DECODE_OUT_SIZE(s)  ((unsigned int)((((s) + 3) / 4) * 3))

/**
 * @brief Decode base64 data
 *
 * Reads base64 data and outputs the binary contents to `out`. Returns 0 on error,
 * or the number of bytes used on success. `out` is expected to be at least
 * `BASE64_DECODE_OUT_SIZE(inlen)` bytes long.
 *
 * @param[in]  in     Input base64 data
 * @param[in]  inlen  Length of the base64 data
 * @param[out] out    Output array where the decoded data will be stored
 *
 * @returns Returns 0 on error, or the number of bytes written on success.
 *
 * @see @ref BASE64_DECODE_OUT_SIZE()
 */
size_t base64_decode(const char* in, size_t inlen, unsigned char* out);

/**
 * @brief Parse a public key in DER format
 *
 * Parses a public key in DER format and returns its parameters (p, q, g, y).
 * This function doesn't implement a full DER parser, but rather relies on the
 * given key following the standardized format as specified in RFC 3279.
 *
 * @param[in]  der   Input key data, in DER format.
 * @param[in]  len   Length of the input DER data
 * @param[out] keyP  Output pointer where the P parameter of the key will be stored
 * @param[out] keyQ  Output pointer where the Q parameter of the key will be stored
 * @param[out] keyG  Output pointer where the G parameter of the key will be stored
 * @param[out] keyY  Output pointer where the Y parameter of the key will be stored
 *
 * @return Returns 0 on error, 1 on success
 *
 * @see @ref parse_der_signature()
 */
int parse_der_pubkey(const unsigned char* der, size_t len, mp_int* keyP, mp_int* keyQ, mp_int* keyG, mp_int* keyY);

/**
 * @brief Parse a signature in DER format
 *
 * Parses a signature in DER format and returns its parameters (r, s). This
 * function doesn't implement a full DER parser, but rather relies on the given
 * key following the standardized format as specified in RFC 3279.
 *
 * @param[in]  der  Input key data, in DER format.
 * @param[in]  len  Length of the input DER data
 * @param[out] r    Output pointer where the r parameter of the key will be stored
 * @param[out] s    Output pointer where the s parameter of the key will be stored
 *
 * @return Returns 0 on error, 1 on success
 *
 * @see @ref parse_der_pubkey()
 */
int parse_der_signature(const unsigned char* der, size_t len, mp_int* r, mp_int* s);

/**
 * @brief Parses a PEM file, removed the armoring and returns DER-encoded data
 *
 * This function accepts a standard PEM public/private key file, strips the
 * "-----BEGIN..." and "-----END..." lines, base64-decodes its contents and
 * returns that.
 *
 * @param[in]  pem  Null-terminated with PEM-encoded key data
 * @param[in]  len  Length of the null-terminated string
 * @param[out] out  Where to store the DER data. It should be at least
 *                  `BASE64_DECODE_OUT_SIZE(len)` bytes long.
 *
 * @returns Returns 0 on error, otherwise returns the number of bytes the DER
 * encoding uses.
 */
size_t pem2der(const char* pem, size_t len, unsigned char* out);

#endif
