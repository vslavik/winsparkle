/*
 *  This file is part of WinSparkle (https://winsparkle.org)
 *
 *  Copyright (C) 2017-2018 Ihor Dutchak
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

#include <exception>
#include <string>

namespace winsparkle
{

class BadSignatureException : public std::runtime_error
{
public:
    BadSignatureException() : std::runtime_error("Invalid update signature") {}
    BadSignatureException(const std::string& msg) : std::runtime_error("Invalid update signature: " + msg) {}
};

class SignatureVerifier
{
public:
    // Throws an exception if pem is not a valid DSA public key in PEM format
    static void VerifyDSAPubKeyPem(const std::string &pem);

    // Verify DSA signature of SHA1 hash of the file. Equivalent to:
    // openssl dgst -sha1 -binary < filename | openssl dgst -sha1 -verify dsa_pub.pem -signature signature.bin
    // Throws BadSignatureException on failure.
    static void VerifyDSASHA1SignatureValid(const std::wstring &filename, const std::string &signature_base64);
};

} // namespace winsparkle

#endif // _signatureverifier_h_
