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

#ifndef _DSA_VERIFY_H_
#define _DSA_VERIFY_H_

#include <stdint.h>
#include <stddef.h>

/** @brief Alias for SHA1 hash */
typedef uint8_t SHA1_t[20];

enum
{
    DSA_VERIFICATION_OK        =  1, ///< Verification successful
    DSA_VERIFICATION_FAILED    =  0, ///< Verification failed
};

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Verify a given SHA1 hash, key & signature in DER form
 *
 * This function verifies a SHA1 hash using the given public key and signature.
 *
 * @param sha1        SHA1 hash to be verified
 * @param pubkey      Binary DER representation of the public key
 * @param pubkey_len  Length of the public key
 * @param sig         Binary DER representation of the signature of the file
 * @param sig_len     Length of the signature
 *
 * @returns Returns 1 (@ref DSA_VERIFICATION_OK) on success, 0 (@ref DSA_VERIFICATION_FAILED)
 * on verification failure or error.
 */
int dsa_verify_hash_der(const SHA1_t sha1, const unsigned char* pubkey, size_t pubkey_len, const unsigned char* sig, size_t sig_len);

/**
 * Verify a given blob.
 *
 * This function verifies a blob of data using the given public key and signature,
 * provided in binary DER format.
 *
 * @param data        Pointer to the beginning of the data blob
 * @param data_len    Length of the data blob
 * @param pubkey      Binary DER representation of the public key
 * @param pubkey_len  Length of the public key
 * @param sig         Binary DER representation of the signature of the file
 * @param sig_len     Length of the signature
 *
 * @returns Returns 1 (@ref DSA_VERIFICATION_OK) on success, 0 (@ref DSA_VERIFICATION_FAILED)
 * on verification failure or error.
 */
int dsa_verify_blob_der(const unsigned char* data, size_t data_len,
                        const unsigned char* pubkey, size_t pubkey_len,
                        const unsigned char* sig, size_t sig_len);

#ifdef __cplusplus
}
#endif

#endif
