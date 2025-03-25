/*
 *  This file is part of WinSparkle (https://winsparkle.org)
 *
 *  Copyright (C) 2025 Vaclav Slavik
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

#include "winsparkle-version.h"

#include <argparse/argparse.hpp>
#include <ed25519.h>

#include <iostream>
#include <fstream>
#include <stdexcept>
#include <string>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <wincrypt.h>
#ifdef _MSC_VER
#pragma comment(lib, "crypt32.lib")
#endif


std::string base64_encode(const uint8_t* data, size_t len)
{
    DWORD base64_len = 0;
    if (!CryptBinaryToStringA(data, (DWORD)len, CRYPT_STRING_BASE64 | CRYPT_STRING_NOCRLF, NULL, &base64_len))
    {
        throw std::runtime_error("Failed to encode as base64");
    }

    std::string str(base64_len, '\0');
    if (!CryptBinaryToStringA(data, (DWORD)len, CRYPT_STRING_BASE64 | CRYPT_STRING_NOCRLF, str.data(), &base64_len))
    {
        throw std::runtime_error("Failed to encode as base64");
    }

    return str;
}

std::vector<uint8_t> base64_decode(const std::string& base64)
{
    DWORD size = 0;

    if (!CryptStringToBinaryA(&base64[0], (DWORD)base64.size(), CRYPT_STRING_BASE64, NULL, &size, NULL, NULL))
    {
        throw std::runtime_error("Failed to decode base64 string");
    }

    std::vector<uint8_t> bin(size);
    if (!CryptStringToBinaryA(&base64[0], (DWORD)base64.size(), CRYPT_STRING_BASE64, (BYTE*)&bin[0], &size, NULL, NULL))
    {
        throw std::runtime_error("Failed to decode base64 string");
    }

    return bin;
}


struct KeyData
{
    uint8_t public_key[32];
    uint8_t private_key[64];
};


KeyData load_private_key(const std::string& private_key_file)
{
    std::ifstream file(private_key_file);
    if (!file)
    {
        throw std::runtime_error("Failed to read private key file");
    }
    std::string seed_str;
    file >> seed_str;
    file.close();

    auto seed = base64_decode(seed_str);
    if (seed.size() != 32)
    {
        throw std::runtime_error("Invalid private key size");
    }

    KeyData key;
    ed25519_create_keypair(key.public_key, key.private_key, seed.data());
    return key;
}


void print_public_key(const KeyData& key)
{
    auto pubkey = base64_encode(key.public_key, sizeof(key.public_key));

    std::cout
        << "Public key: " << pubkey << std::endl
        << std::endl;
    std::cout
        << "Add the public key to the resource file like this:" << std::endl
        << std::endl
        << "    EdDSAPub EDDSA {\"" << pubkey << "\"}" << std::endl
        << std::endl
        << "or use the API to set it:" << std::endl
        << std::endl
        << "    win_sparkle_set_eddsa_public_key(\"" << pubkey << "\");" << std::endl
        << std::endl;
}


void generate_key(const std::string& private_key_file)
{
    // Generate a new key pair
    uint8_t seed[32];

    if (ed25519_create_seed(seed) != 0)
    {
        throw std::runtime_error("Failed to generate private key");
    }

    KeyData key;
    ed25519_create_keypair(key.public_key, key.private_key, seed);

    std::ofstream file(private_key_file);
    if (!file)
    {
        throw std::runtime_error("Failed to open file for writing");
    }
    file << base64_encode(seed, sizeof(seed));
    file.close();

    std::cout << "Private key saved to " << private_key_file << std::endl;
    print_public_key(key);
}


int main(int argc, char* argv[])
{
    std::string private_key_file;

    argparse::ArgumentParser program("winsparkle-tool", WIN_SPARKLE_VERSION_STRING);
    program.add_description("WinSparkle companion tool");

    argparse::ArgumentParser generate_key_cmd("generate-key", "", argparse::default_arguments::help);
    generate_key_cmd.add_description("generate a new EdDSA key pair");
    generate_key_cmd.add_argument("-f", "--file")
        .help("file to save the private key")
        .metavar("KEYFILE")
        .required()
        .store_into(private_key_file);
    program.add_subparser(generate_key_cmd);

    argparse::ArgumentParser public_key_cmd("public-key", "", argparse::default_arguments::help);
    public_key_cmd.add_description("print EdDSA public key information");
    public_key_cmd.add_argument("-f", "--private-key-file")
        .help("file with the private key")
        .metavar("KEYFILE")
        .required()
        .store_into(private_key_file);
    program.add_subparser(public_key_cmd);

    try
    {
        program.parse_args(argc, argv);
        if (!program)
        {
            std::cerr << program << std::endl;
            return 1;
        }
    }
    catch (const std::runtime_error& err)
    {
        std::cerr << err.what() << std::endl;
        std::cerr << program << std::endl;
        return 1;
    }

    try
    {
        if (program.is_subcommand_used(generate_key_cmd))
        {
            generate_key(private_key_file);
        }
        else if (program.is_subcommand_used(public_key_cmd))
        {
            print_public_key(load_private_key(private_key_file));
        }
    catch (const std::exception& e)
    {
        std::cerr << e.what() << std::endl;
        return 1;
    }

    return 0;
}
