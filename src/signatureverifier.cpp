/*
 *  This file is part of WinSparkle (https://winsparkle.org)
 *
 *  Copyright (C) 2009-2017 Vaclav Slavik
 *  Copyright (C) 2017 Ihor Dutchak
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

#include <stdexcept>

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <Wincrypt.h>

#ifdef _MSC_VER
#pragma comment(lib, "crypt32.lib")
#endif

#include "error.h"
#include "settings.h"
#include "utils.h"

#define SHA_DIGEST_LENGTH 20

// Some declarations from OpenSSL
extern "C"
{

typedef struct bio_st BIO;
typedef struct dsa_st DSA;

typedef int (*PDSA_verify)(int type, const unsigned char *dgst, int dgst_len,
                           const unsigned char *sigbuf, int siglen, DSA *dsa);

typedef BIO * (*PBIO_new_mem_buf)(const void *buf, int len);
typedef int (*PBIO_free)(BIO *a);

typedef DSA * (*PPEM_read_bio_DSA_PUBKEY)(BIO *bp, DSA **x, /*pem_password_cb*/void *cb, void *u);
typedef void (*PDSA_free)(DSA *r);

typedef unsigned long (*PERR_get_error)(void);
typedef char * (*PERR_error_string)(unsigned long e, char *buf);

}

namespace winsparkle
{

namespace
{

class CFile
{
    FILE *f;
    CFile(const CFile &);
    CFile &operator=(const CFile &);
public:
    CFile(FILE *file): f(file) {}

    operator FILE*()
    {
        return f;
    }

    ~CFile()
    {
        if (f)
            fclose(f);
    }
};

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
        if (!CryptHashData(handle, (CONST BYTE  *)buffer, buffer_len, 0))
            throw Win32Exception("Failed to hash data");
    }

    void hashFile(const std::wstring &filename)
    {
        CFile f (_wfopen(filename.c_str(), L"rb"));
        if (!f)
            throw std::runtime_error(WideToAnsi(L"Failed to open file " + filename));

        const int BUF_SIZE = 8192;
        unsigned char buf[BUF_SIZE];

        while (size_t read_bytes = fread(buf, 1, BUF_SIZE, f))
        {
            hashData(buf, read_bytes);
        }

        if (ferror(f))
            throw std::runtime_error(WideToAnsi(L"Failed to read file " + filename));
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
        if (!ms_libeay32)
        {
            ms_libeay32 = ::LoadLibraryW(L"libeay32.dll");
            if (!ms_libeay32)
                throw std::runtime_error("Failed to load libeay32.dll");

            DSA_verify = (PDSA_verify)::GetProcAddress(ms_libeay32, "DSA_verify");
            BIO_new_mem_buf = (PBIO_new_mem_buf)::GetProcAddress(ms_libeay32, "BIO_new_mem_buf");
            BIO_free = (PBIO_free)::GetProcAddress(ms_libeay32, "BIO_free");
            PEM_read_bio_DSA_PUBKEY = (PPEM_read_bio_DSA_PUBKEY)::GetProcAddress(ms_libeay32, "PEM_read_bio_DSA_PUBKEY");
            DSA_free = (PDSA_free)::GetProcAddress(ms_libeay32, "DSA_free");
            ERR_get_error = (PERR_get_error)::GetProcAddress(ms_libeay32, "ERR_get_error");
            ERR_error_string = (PERR_error_string)::GetProcAddress(ms_libeay32, "ERR_error_string");

            if (! (DSA_verify &&
              BIO_new_mem_buf &&
              BIO_free &&
              PEM_read_bio_DSA_PUBKEY &&
              DSA_free &&
              ERR_get_error &&
              ERR_error_string) )
            {
                CloseLib();
                throw std::runtime_error("Failed to load libeay32.dll symbols");
            }

        }
        static TinySSL instance;
        return instance;
    }

    static void CloseLib()
    {
        if (ms_libeay32)
        {
            ::FreeLibrary(ms_libeay32);
            ms_libeay32 = NULL;
        }
    }

    ~TinySSL()
    {
        CloseLib();
    }

    void SetDSAPubKey(const std::string &pem)
    {
        m_dsaPub.ReadFromPem(pem);
    }

    void VerifyDSASHA1Signature(const std::wstring &filename, const std::string &signature)
    {
        unsigned char sha1[SHA_DIGEST_LENGTH];

        {
            WinCryptRSAContext ctx;
            // SHA1 of file
            {
                WinCryptSHA1Hash hash(ctx);
                hash.hashFile(filename);
                hash.sha1Val(sha1);
            }
            // SHA1 of SHA1 of file
            {
                WinCryptSHA1Hash hash(ctx);
                hash.hashData(sha1, ARRAYSIZE(sha1));
                hash.sha1Val(sha1);
            }
        }

        DSAPub &pubKey(PubKey());

        const int code = DSA_verify(0, sha1, ARRAYSIZE(sha1), (const unsigned char*)signature.c_str(), signature.size(), pubKey);

        if (code == -1) // OpenSSL error
            throw std::runtime_error(ERR_error_string(ERR_get_error(), nullptr));

        if (code != 1)
            throw std::runtime_error("DSA Signature not match!");
    }

private:
    static HMODULE ms_libeay32;

    static PDSA_verify DSA_verify;
    static PBIO_new_mem_buf BIO_new_mem_buf;
    static PBIO_free BIO_free;
    static PPEM_read_bio_DSA_PUBKEY PEM_read_bio_DSA_PUBKEY;
    static PDSA_free DSA_free;
    static PERR_get_error ERR_get_error;
    static PERR_error_string ERR_error_string;

    class BIOWrap
    {
        BIO* bio = NULL;

    public:
        BIOWrap(const std::string &mem_buf)
        {
            bio = BIO_new_mem_buf(mem_buf.c_str(), int(mem_buf.size()));
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

    class DSAPub
    {
        DSA *dsa = NULL;

    public:
        DSAPub()
        {
        }

        void ReadFromPem(const std::string &pem_key)
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

    DSAPub m_dsaPub;
    DSAPub &PubKey()
    {
        if (static_cast<DSA*>(m_dsaPub) == NULL)
        {
            std::string pem = Settings::GetDSAPubKeyPem(); // side effect - it will call SignatureVerifier::SetDSAPubKeyPem
            if (static_cast<DSA*>(m_dsaPub) == NULL) // usualy should not happen, but double check
            {
                m_dsaPub.ReadFromPem(pem);
            }
        }
        return m_dsaPub;
    }

}; // TinySSL

HMODULE TinySSL::ms_libeay32 = NULL;

PDSA_verify TinySSL::DSA_verify = NULL;
PBIO_new_mem_buf TinySSL::BIO_new_mem_buf = NULL;
PBIO_free TinySSL::BIO_free = NULL;
PPEM_read_bio_DSA_PUBKEY TinySSL::PEM_read_bio_DSA_PUBKEY = NULL;
PDSA_free TinySSL::DSA_free = NULL;
PERR_get_error TinySSL::ERR_get_error = NULL;
PERR_error_string TinySSL::ERR_error_string = NULL;

std::string Base64ToBin(const std::string &base64)
{
    DWORD nDestinationSize = 0;
    std::string bin;

    bool ok = false;

    if (CryptStringToBinaryA(&base64[0], base64.size(), CRYPT_STRING_BASE64, NULL, &nDestinationSize, NULL, NULL))
    {
        bin.resize(nDestinationSize);
        if (CryptStringToBinaryA(&base64[0], base64.size(), CRYPT_STRING_BASE64, (BYTE *)&bin[0], &nDestinationSize, NULL, NULL))
        {
            ok = true;
        }
    }

    if (!ok)
        throw std::runtime_error("Failed to decode base64 string");

    return bin;
}

} // anonynous

void SignatureVerifier::SetDSAPubKeyPem(const std::string &pem)
{
    TinySSL::inst().SetDSAPubKey(pem);
}

bool SignatureVerifier::DSASHA1SignatureValid(const std::wstring &filename, const std::string &signature_base64)
{
    try
    {
        if (signature_base64.size() == 0)
            throw std::runtime_error("Missing DSA signature!");
        TinySSL::inst().VerifyDSASHA1Signature(filename, Base64ToBin(signature_base64));
        return true;
    }
    CATCH_ALL_EXCEPTIONS
    return false;
}

} // namespace winsparkle
