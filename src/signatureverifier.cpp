/*
 *  This file is part of WinSparkle (https://winsparkle.org)
 *
 *  Copyright (C) 2017-2020 Ihor Dutchak
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

#include "signatureverifier.h"

#include "error.h"
#include "mmap.h"
#include "base64.h"

#include "ed25519.h"
#include "dsa/dsa-verify.h"

#include <stdexcept>


namespace winsparkle
{

void SignatureVerifier::VerifyDSAPubKeyPem(const std::string& pem)
{
    if (pem.empty())
        throw std::invalid_argument("Invalid public key size.");
    // at least check that the key is valid PEM:
    auto der = DecodePEMToDER(pem);
}

void SignatureVerifier::VerifyEdDSAPubKey(const std::string& pubkey_base64)
{
    const std::vector<uint8_t> pubkey = DecodeBase64(pubkey_base64);
    if (pubkey.size() != 32)
    {
        throw std::invalid_argument("Invalid public key size.");
    }
}

bool SignatureVerifier::IsDSASHA1SignatureValid(const std::string& dsa_pubkey_pem, const std::string& signature_base64, const uint8_t *buffer, size_t length)
{
    try
    {
        if (signature_base64.size() == 0 || signature_base64.size() > 1000)
        {
            LogError("Malformed DSA signature.");
            return false;
        }

        auto pubkey = DecodePEMToDER(dsa_pubkey_pem);
        auto signature = DecodeBase64(signature_base64);
        if (pubkey.empty() || signature.empty())
            return false;

        return dsa_verify_blob_der(buffer, length, pubkey.data(), pubkey.size(), signature.data(), signature.size()) == DSA_VERIFICATION_OK;
    }
    catch (const std::invalid_argument&)
    {
        return false;
    }
}

bool SignatureVerifier::IsDSASHA1SignatureValid(const std::string& dsa_pubkey_pem, const std::string& signature_base64, const std::wstring& filename)
{
    return WithMappedFile
    (
        filename,
        [&](const uint8_t* buffer, size_t length) {
            return IsDSASHA1SignatureValid(dsa_pubkey_pem, signature_base64, buffer, length);
        }
    );
}

bool SignatureVerifier::IsEdDSASignatureValid(const std::string& pubkey_base64, const std::string& signature_base64, const uint8_t *buffer, size_t length)
{
    if (signature_base64.size() == 0)
    {
        LogError("Missing EdDSA signature!");
        return false;
    }

    try
    {
        auto signature = DecodeBase64(signature_base64);

        if (signature.size() != 64)
        {
            LogError("Invalid signature size.");
            return false;
        }

        const std::vector<uint8_t> pubkey = DecodeBase64(pubkey_base64);
        if (pubkey.size() != 32)
        {
            LogError("Invalid public key size.");
            return false;
        }

        return ed25519_verify(signature.data(), buffer, length, pubkey.data()) == 1;
    }
    catch (const std::invalid_argument&)
    {
        return false;
    }
}

bool SignatureVerifier::IsEdDSASignatureValid(const std::string& pubkey_base64, const std::string& signature_base64, const std::wstring& filename)
{
    return WithMappedFile
           (
               filename,
               [&](const uint8_t *buffer, size_t length) {
                   return IsEdDSASignatureValid(pubkey_base64, signature_base64, buffer, length);
               }
           );
}

} // namespace winsparkle
