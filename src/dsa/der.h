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

#include "tommath.h"

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

#endif
