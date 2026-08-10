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

#include <openssl/dsa.h>
#include <openssl/err.h>
#include <openssl/pem.h>
#include <openssl/sha.h>

#include <ed25519.h>

#include <stdexcept>

#include "wrapwin.h"
#include <wincrypt.h>

#ifdef _MSC_VER
#pragma comment(lib, "crypt32.lib")
#endif


namespace winsparkle
{

namespace
{

class WinCryptRSAContext
{
    HCRYPTPROV handle;

    WinCryptRSAContext(const WinCryptRSAContext &);
    WinCryptRSAContext &operator=(const WinCryptRSAContext &);
public:
    WinCryptRSAContext()
    {
        if (!CryptAcquireContextW(&handle, NULL, NULL, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT))
            throw Win32Exception("Failed to create crypto context");
    }

    ~WinCryptRSAContext()
    {
        if (!CryptReleaseContext(handle, 0))
            LogError("Failed to release crypto context");
    }

    operator HCRYPTPROV()
    {
        return handle;
    }
};

class WinCryptSHA1Hash
{
    HCRYPTHASH handle;

    WinCryptSHA1Hash(const WinCryptSHA1Hash&);
    WinCryptSHA1Hash& operator=(const WinCryptSHA1Hash &);
public:
    WinCryptSHA1Hash(WinCryptRSAContext &context)
    {
        if (!CryptCreateHash(HCRYPTPROV(context), CALG_SHA1, 0, 0, &handle))
            throw Win32Exception("Failed to create crypto hash");
    }

    ~WinCryptSHA1Hash()
    {
        if (handle)
        {
            if (!CryptDestroyHash(handle))
            {
                LogError("Failed to destroy crypto hash");
            }
        }
    }

    void hashData(const void *buffer, size_t buffer_len)
    {
        if (!CryptHashData(handle, (CONST BYTE  *)buffer, (DWORD)buffer_len, 0))
            throw Win32Exception("Failed to hash data");
    }

    void sha1Val(unsigned char(&sha1)[SHA_DIGEST_LENGTH])
    {
        DWORD hash_len = SHA_DIGEST_LENGTH;
        if (!CryptGetHashParam(handle, HP_HASHVAL, sha1, &hash_len, 0))
            throw Win32Exception("Failed to get SHA1 val");
    }

};

/**
    Light-weight dynamic loader of OpenSSL library.
    Loads only minimum required symbols, just enough to verify DSA SHA1 signature of the file.
 */
class TinySSL
{
    TinySSL() {}
public:
    static TinySSL &inst()
    {
        static TinySSL instance;
        return instance;
    }

    ~TinySSL()
    {
    }

    bool VerifyDSASHA1Signature(const std::string& dsa_pubkey_pem, const uint8_t *buffer, size_t length, const std::vector<uint8_t>& signature)
    {
        unsigned char sha1[SHA_DIGEST_LENGTH];

        if (signature.empty())
        {
            LogError("Missing DSA signature!");
            return false;
        }

        {
            WinCryptRSAContext ctx;
            // SHA1 of file
            {
                WinCryptSHA1Hash hash(ctx);
                hash.hashData(buffer, length);
                hash.sha1Val(sha1);
            }
            // SHA1 of SHA1 of file
            {
                WinCryptSHA1Hash hash(ctx);
                hash.hashData(sha1, ARRAYSIZE(sha1));
                hash.sha1Val(sha1);
            }
        }

        DSAPub pubKey(dsa_pubkey_pem);

        const int code = DSA_verify(0, sha1, ARRAYSIZE(sha1), signature.data(), (int)signature.size(), pubKey);

        return code == 1;
    }

private:
    class BIOWrap
    {
        BIO* bio;

        BIOWrap(const BIOWrap &);
        BIOWrap &operator=(const BIOWrap &);

    public:
        BIOWrap(const std::string &mem_buf)
            : bio(BIO_new_mem_buf(mem_buf.c_str(), int(mem_buf.size())))
        {
            if (!bio)
                throw std::invalid_argument("Cannot set PEM key mem buffer");
        }

        operator BIO*()
        {
            return bio;
        }

        ~BIOWrap()
        {
            BIO_free(bio);
        }

    }; // BIOWrap

public:
    class DSAPub
    {
        DSA *dsa;

        DSAPub(const DSAPub &);
        DSAPub &operator=(const DSAPub &);

    public:
        DSAPub(const std::string &pem_key)
            : dsa(NULL)
        {
            BIOWrap bio(pem_key);
            if (!PEM_read_bio_DSA_PUBKEY(bio, &dsa, NULL, NULL))
            {
                throw std::invalid_argument("Cannot read DSA public key from PEM");
            }
        }

        operator DSA*()
        {
            return dsa;
        }

        ~DSAPub()
        {
            if (dsa)
                DSA_free(dsa);
        }

    }; // DSAWrap

}; // TinySSL

} // anonynous namespace


void SignatureVerifier::VerifyDSAPubKeyPem(const std::string& pem)
{
    // DSAPub::DSAPub() throw if not valid
    TinySSL::DSAPub dsa_pub(pem);
    (void)dsa_pub;
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
    std::vector<uint8_t> signature;
    try
    {
        signature = DecodeBase64(signature_base64);
    }
    catch (const std::invalid_argument&)
    {
        return false;
    }

    return TinySSL::inst().VerifyDSASHA1Signature(dsa_pubkey_pem, buffer, length, signature);
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
    try
    {
        if (signature_base64.size() == 0 || signature_base64.size() > 1000)
        {
            LogError("Malformed EdDSA signature.");
            return false;
        }

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
