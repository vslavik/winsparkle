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

#pragma comment(lib, "crypt32.lib")

#include "error.h"
#include "settings.h"
#include "utils.h"

#define SHA_DIGEST_LENGTH 20

// Some declarations from OpenSSL
extern "C"
{

struct SHA_CTX
{
    // Fields are not relevant. Only struct size.
    unsigned char data[96];
};
typedef struct bio_st BIO;
typedef struct dsa_st DSA;

typedef int (*PSHA1_Init)(SHA_CTX *c);
typedef int (*PSHA1_Update)(SHA_CTX *c, const void *data, size_t len);
typedef int (*PSHA1_Final)(unsigned char *md, SHA_CTX *c);

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

            SHA1_Init = (PSHA1_Init)::GetProcAddress(ms_libeay32, "SHA1_Init");
            SHA1_Update = (PSHA1_Update)::GetProcAddress(ms_libeay32, "SHA1_Update");
            SHA1_Final = (PSHA1_Final)::GetProcAddress(ms_libeay32, "SHA1_Final");
            DSA_verify = (PDSA_verify)::GetProcAddress(ms_libeay32, "DSA_verify");
            BIO_new_mem_buf = (PBIO_new_mem_buf)::GetProcAddress(ms_libeay32, "BIO_new_mem_buf");
            BIO_free = (PBIO_free)::GetProcAddress(ms_libeay32, "BIO_free");
            PEM_read_bio_DSA_PUBKEY = (PPEM_read_bio_DSA_PUBKEY)::GetProcAddress(ms_libeay32, "PEM_read_bio_DSA_PUBKEY");
            DSA_free = (PDSA_free)::GetProcAddress(ms_libeay32, "DSA_free");
            ERR_get_error = (PERR_get_error)::GetProcAddress(ms_libeay32, "ERR_get_error");
            ERR_error_string = (PERR_error_string)::GetProcAddress(ms_libeay32, "ERR_error_string");

            if (! (SHA1_Init &&
              SHA1_Update &&
              SHA1_Final &&
              DSA_verify &&
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

    void FileSHA1(const std::wstring &filename, unsigned char(&sha1)[SHA_DIGEST_LENGTH])
    {
        FILE *f = _wfopen(filename.c_str(), L"rb");
        if (!f)
            throw std::runtime_error(WideToAnsi(L"Failed to open file " + filename));

        SHA1Context sc;

        const int BUF_SIZE = 8192;
        unsigned char buf[BUF_SIZE];

        while (size_t read_bytes = fread(buf, 1, BUF_SIZE, f))
        {
            sc.Update(buf, read_bytes);
        }

        if (ferror(f))
        {
            fclose(f);
            throw std::runtime_error(WideToAnsi(L"Failed to read file " + filename));
        }

        fclose(f);
        sc.Final(sha1);
    }

    void BufferSHA1(const void *buffer, size_t buffer_len, unsigned char(&sha1)[SHA_DIGEST_LENGTH])
    {
        SHA1Context sc;
        sc.Update(buffer, buffer_len);
        sc.Final(sha1);
    }

    void SetDSAPubKey(const std::string &pem)
    {
        m_dsaPub.ReadFromPem(pem);
    }

    void VerifyDSASHA1Signature(const std::wstring &filename, const std::string &signature)
    {
        unsigned char sha1[SHA_DIGEST_LENGTH];

        FileSHA1(filename, sha1);
        // SHA1 of SHA1 of file
        BufferSHA1(sha1, SHA_DIGEST_LENGTH, sha1);

        DSAPub &pubKey(PubKey());

        const int code = DSA_verify(0, sha1, SHA_DIGEST_LENGTH, (const unsigned char*)signature.c_str(), signature.size(), pubKey);

        if (code == -1) // OpenSSL error
            throw std::runtime_error(ERR_error_string(ERR_get_error(), nullptr));

        if (code != 1)
            throw std::runtime_error("DSA Signature not match!");
    }

private:
    static HMODULE ms_libeay32;

    static PSHA1_Init SHA1_Init;
    static PSHA1_Update SHA1_Update;
    static PSHA1_Final SHA1_Final;
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

    class SHA1Context
    {
        SHA_CTX sc;
        bool was_read;
    public:
        SHA1Context()
        {
            SHA1_Init(&sc);
            was_read = false;
        }

        int Update(const void *data, size_t len)
        {
            return SHA1_Update(&sc, data, len);
        }

        int Final(unsigned char (&sha1)[SHA_DIGEST_LENGTH])
        {
            was_read = true;
            return SHA1_Final(sha1, &sc);
        }

        ~SHA1Context()
        {
            if (!was_read)
            {
                unsigned char void_data[SHA_DIGEST_LENGTH];
                SHA1_Final(void_data, &sc);
            }
        }

    }; // SHA1Context

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

PSHA1_Init TinySSL::SHA1_Init = NULL;
PSHA1_Update TinySSL::SHA1_Update = NULL;
PSHA1_Final TinySSL::SHA1_Final = NULL;
PDSA_verify TinySSL::DSA_verify = NULL;
PBIO_new_mem_buf TinySSL::BIO_new_mem_buf = NULL;
PBIO_free TinySSL::BIO_free = NULL;
PPEM_read_bio_DSA_PUBKEY TinySSL::PEM_read_bio_DSA_PUBKEY = NULL;
PDSA_free TinySSL::DSA_free = NULL;
PERR_get_error TinySSL::ERR_get_error = NULL;
PERR_error_string TinySSL::ERR_error_string = NULL;

std::string Base64ToBin(const std::string &base64)
{
    DWORD nDestinationSize;
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
