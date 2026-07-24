/*
 *  This file is part of WinSparkle (https://winsparkle.org)
 *
 *  Copyright (C) 2017-2020 Ihor Dutchak
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

#ifndef _signatureverifier_h_
#define _signatureverifier_h_

#include <cstdint>
#include <stdexcept>
#include <string>

namespace winsparkle
{

class SignatureVerifier
{
public:
    // Throws an exception if pem is not a valid DSA public key in PEM format
    static void VerifyDSAPubKeyPem(const std::string &pem);

    // Throws an exception if pem is not a valid EdDSA public key in base64 format
    static void VerifyEdDSAPubKey(const std::string& pubkey_base64);

    // Verify DSA signature of SHA1 hash of the file. Equivalent to:
    // openssl dgst -sha1 -binary < filename | openssl dgst -sha1 -verify dsa_pub.pem -signature signature.bin
    // Returns true if the signature is valid, false otherwise.
    static bool IsDSASHA1SignatureValid(const std::string& dsa_pubkey_pem, const std::string& signature_base64, const std::wstring& filename);
    static bool IsDSASHA1SignatureValid(const std::string& dsa_pubkey_pem, const std::string& signature_base64, const uint8_t *buffer, size_t length);

    // Verify EdDSA signature of the file.
    // Returns true if the signature is valid, false otherwise.
    static bool IsEdDSASignatureValid(const std::string& pubkey_base64, const std::string& signature_base64, const std::wstring& filename);
    static bool IsEdDSASignatureValid(const std::string& pubkey_base64, const std::string& signature_base64, const uint8_t *buffer, size_t length);
};

} // namespace winsparkle

#endif // _signatureverifier_h_
