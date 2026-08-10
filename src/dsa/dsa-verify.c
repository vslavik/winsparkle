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

#include <stdlib.h>
#include <string.h>

#include "der.h"
#include "dsa-verify.h"
#include "tommath.h"

#define SHA1_IMPLEMENTATION
#include "sha1.h"

#define MP_OP(op) if ((op) != MP_OKAY) goto error;

static int _dsa_verify_hash(mp_int* hash, mp_int* keyP, mp_int* keyQ, mp_int* keyG, mp_int* keyY, mp_int* r, mp_int* s)
{
    mp_int w, v, u1, u2;
    if (mp_init_multi(&w, &v, &u1, &u2, NULL) != MP_OKAY)
        return DSA_VERIFICATION_FAILED;

    // Check 0 < r < q and 0 < s < q
    if (mp_iszero(r) || mp_iszero(s) || mp_cmp(r, keyQ) != MP_LT || mp_cmp(s, keyQ) != MP_LT)
    {
        mp_clear_multi(&w, &v, &u1, &u2, NULL);
        return DSA_VERIFICATION_FAILED;
    }

    // w := s^-1 mod q
    MP_OP(mp_invmod(s, keyQ, &w));

    // u1 := H(m) * w mod q
    MP_OP(mp_mulmod(hash, &w, keyQ, &u1));

    // u2 := r * w mod q
    MP_OP(mp_mulmod(r, &w, keyQ, &u2));

    // v := g^u1 * y^u2 mod p mod q
    MP_OP(mp_exptmod(keyG, &u1, keyP, &u1)); // u1 := g^u1 mod p
    MP_OP(mp_exptmod(keyY, &u2, keyP, &u2)); // u2 := y^u2 mod p
    MP_OP(mp_mulmod(&u1, &u2, keyP, &v));    // v := u1 * u2 mod p
    MP_OP(mp_mod(&v, keyQ, &v));             // v := v mod q

    // Signature is valid if r == v
    int ret = (mp_cmp(r, &v) == MP_EQ ? DSA_VERIFICATION_OK : DSA_VERIFICATION_FAILED);
    mp_clear_multi(&w, &v, &u1, &u2, NULL);

    return ret;

error:
    mp_clear_multi(&w, &v, &u1, &u2, NULL);
    return DSA_VERIFICATION_FAILED;
}

int dsa_verify_blob_der(const unsigned char* data, size_t data_len,
                        const unsigned char* pubkey, size_t pubkey_len,
                        const unsigned char* sig, size_t sig_len)
{
    SHA1_t sha1sum;
    SHA1(sha1sum, data, data_len);

    SHA1_t sha1sum_of_sum;
    SHA1(sha1sum_of_sum, (const unsigned char*)sha1sum, sizeof(SHA1_t));

    return dsa_verify_hash_der(sha1sum_of_sum, pubkey, pubkey_len, sig, sig_len);
}

int dsa_verify_hash_der(const SHA1_t sha1, const unsigned char* pubkey, size_t pubkey_len, const unsigned char* sig, size_t sig_len)
{
    if (sig_len > 1000)
        return DSA_VERIFICATION_FAILED;

    // Parse public key
    mp_int keyP, keyQ, keyG, keyY, r, s, hash;
    if (mp_init_multi(&keyP, &keyQ, &keyG, &keyY, &r, &s, &hash, NULL) != MP_OKAY)
        return DSA_VERIFICATION_FAILED;

    int ret = DSA_VERIFICATION_FAILED;

    if (parse_der_pubkey(pubkey, pubkey_len, &keyP, &keyQ, &keyG, &keyY) == 0)
        goto error;

    // Parse signature
    if (parse_der_signature(sig, sig_len, &r, &s) == 0)
        goto error;

    // Read hash, verify data
    MP_OP(mp_from_ubin(&hash, sha1, sizeof(SHA1_t)));

    ret = _dsa_verify_hash(&hash, &keyP, &keyQ, &keyG, &keyY, &r, &s);

error:
    mp_clear_multi(&keyP, &keyQ, &keyG, &keyY, &r, &s, &hash, NULL);

    return ret;
}
