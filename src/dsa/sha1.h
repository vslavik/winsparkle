/*
SHA-1 in C
By Steve Reid <steve@edmweb.com>
Minor API changes by Marc Izquierdo <marcizhu@gmail.com>
100% Public Domain
*/

/*
 Single file library. #include it as many times as you need, and
 #define SHA1_IMPLEMENTATION in *one* c/cpp file BEFORE including it
 */

#ifndef __SHA1_H__
#define __SHA1_H__

/******************************************************************************
 *                                   HEADER                                   *
 ******************************************************************************/

#include <stdint.h>
#include <stddef.h>

/** @brief SHA1 context */
typedef struct
{
    uint32_t state[5];
    uint32_t count[2];
    unsigned char buffer[64];
} SHA1_CTX;

/** @brief Alias for SHA1 hash */
typedef uint8_t SHA1_t[20];

/**
 * @brief Reset SHA1 context
 *
 * Resets the given SHA1 context, setting it to a default state. This function
 * can be called at any time, and thus allows to reuse the same context to
 * calculate many SHA1 hashes.
 *
 * @param[in,out] context  SHA1 context
 */
void SHA1_reset(SHA1_CTX* context);

/**
 * @brief Feed data to SHA1 hash
 *
 * This function takes some data and processes it to calculate the SHA1 hash.
 * This function *MUST* be called after a call to @ref SHA1_reset(), and it
 * can be called as many times as necessary in order to feed all the data the
 * user wants. Once all data has been fed, call @ref SHA1_result() to get the
 * final hash value.
 *
 * @param[in,out] context  SHA1 context
 * @param[in]     data     Data to be fed to the algorithm
 * @param[in]     len      Length of the data to be fed
 */
void SHA1_input(SHA1_CTX* context, const unsigned char* data, size_t len);

/**
 * @brief Get the SHA1 hash of the previously-fed data
 *
 * Generates the SHA1 hash based on the data fed previously using the function
 * @ref SHA1_input(). This function leaves the context in an invalid state. Thus,
 * in order to reuse the same context it is necessary to call @ref SHA1_reset().
 * If no more hashes are desired, the context can be left as is, no cleanup is
 * necessary.
 *
 * @param[in,out] context  SHA1 context
 * @param[out]    digest   SHA1 hash output
 */
void SHA1_result(SHA1_CTX* context, SHA1_t digest);

/**
 * @brief Calculate SHA1 of given data
 *
 * Calculates the SHA1 hash of the given data, without the need to create and
 * initialize the SHA1 context. Useful to hash a single block of data at once.
 *
 * @param[out] digest  Array where the SHA1 hash will be stored
 * @param[in]  data    Byte-array of the data to be hashed
 * @param[in]  len     Length of the byte-array of data, in bytes.
 */
void SHA1(SHA1_t digest, const unsigned char* data, size_t len);

#ifdef SHA1_IMPLEMENTATION
/******************************************************************************
 *                               IMPLEMENTATION                               *
 ******************************************************************************/

#include <string.h>
#include <stdint.h>

#include "sha1.h"

#define rol(value, bits) (((value) << (bits)) | ((value) >> (32 - (bits))))

/* blk0() and blk() perform the initial expand. */
/* I got the idea of expanding during the round function from SSLeay */
#if BYTE_ORDER == LITTLE_ENDIAN
#define blk0(i) (block->l[i] = (rol(block->l[i],24)&0xFF00FF00) \
    |(rol(block->l[i],8)&0x00FF00FF))
#elif BYTE_ORDER == BIG_ENDIAN
#define blk0(i) block->l[i]
#else
#error "Endianness not defined!"
#endif
#define blk(i) (block->l[i&15] = rol(block->l[(i+13)&15]^block->l[(i+8)&15] \
    ^block->l[(i+2)&15]^block->l[i&15],1))

/* (R0+R1), R2, R3, R4 are the different operations used in SHA1 */
#define R0(v,w,x,y,z,i) z+=((w&(x^y))^y)+blk0(i)+0x5A827999+rol(v,5);w=rol(w,30);
#define R1(v,w,x,y,z,i) z+=((w&(x^y))^y)+blk(i)+0x5A827999+rol(v,5);w=rol(w,30);
#define R2(v,w,x,y,z,i) z+=(w^x^y)+blk(i)+0x6ED9EBA1+rol(v,5);w=rol(w,30);
#define R3(v,w,x,y,z,i) z+=(((w|x)&y)|(w&x))+blk(i)+0x8F1BBCDC+rol(v,5);w=rol(w,30);
#define R4(v,w,x,y,z,i) z+=(w^x^y)+blk(i)+0xCA62C1D6+rol(v,5);w=rol(w,30);

/* Hash a single 512-bit block. This is the core of the algorithm. */
void SHA1_transform(uint32_t state[5], const unsigned char buffer[64])
{
    typedef union
    {
        unsigned char c[64];
        uint32_t l[16];
    } CHAR64LONG16;

    CHAR64LONG16 block[1]; /* use array to appear as a pointer */
    memcpy(block, buffer, 64);

    /* Copy context->state[] to working vars */
    uint32_t a = state[0];
    uint32_t b = state[1];
    uint32_t c = state[2];
    uint32_t d = state[3];
    uint32_t e = state[4];

    /* 4 rounds of 20 operations each. Loop unrolled. */
    R0(a, b, c, d, e, 0);
    R0(e, a, b, c, d, 1);
    R0(d, e, a, b, c, 2);
    R0(c, d, e, a, b, 3);
    R0(b, c, d, e, a, 4);
    R0(a, b, c, d, e, 5);
    R0(e, a, b, c, d, 6);
    R0(d, e, a, b, c, 7);
    R0(c, d, e, a, b, 8);
    R0(b, c, d, e, a, 9);
    R0(a, b, c, d, e, 10);
    R0(e, a, b, c, d, 11);
    R0(d, e, a, b, c, 12);
    R0(c, d, e, a, b, 13);
    R0(b, c, d, e, a, 14);
    R0(a, b, c, d, e, 15);
    R1(e, a, b, c, d, 16);
    R1(d, e, a, b, c, 17);
    R1(c, d, e, a, b, 18);
    R1(b, c, d, e, a, 19);
    R2(a, b, c, d, e, 20);
    R2(e, a, b, c, d, 21);
    R2(d, e, a, b, c, 22);
    R2(c, d, e, a, b, 23);
    R2(b, c, d, e, a, 24);
    R2(a, b, c, d, e, 25);
    R2(e, a, b, c, d, 26);
    R2(d, e, a, b, c, 27);
    R2(c, d, e, a, b, 28);
    R2(b, c, d, e, a, 29);
    R2(a, b, c, d, e, 30);
    R2(e, a, b, c, d, 31);
    R2(d, e, a, b, c, 32);
    R2(c, d, e, a, b, 33);
    R2(b, c, d, e, a, 34);
    R2(a, b, c, d, e, 35);
    R2(e, a, b, c, d, 36);
    R2(d, e, a, b, c, 37);
    R2(c, d, e, a, b, 38);
    R2(b, c, d, e, a, 39);
    R3(a, b, c, d, e, 40);
    R3(e, a, b, c, d, 41);
    R3(d, e, a, b, c, 42);
    R3(c, d, e, a, b, 43);
    R3(b, c, d, e, a, 44);
    R3(a, b, c, d, e, 45);
    R3(e, a, b, c, d, 46);
    R3(d, e, a, b, c, 47);
    R3(c, d, e, a, b, 48);
    R3(b, c, d, e, a, 49);
    R3(a, b, c, d, e, 50);
    R3(e, a, b, c, d, 51);
    R3(d, e, a, b, c, 52);
    R3(c, d, e, a, b, 53);
    R3(b, c, d, e, a, 54);
    R3(a, b, c, d, e, 55);
    R3(e, a, b, c, d, 56);
    R3(d, e, a, b, c, 57);
    R3(c, d, e, a, b, 58);
    R3(b, c, d, e, a, 59);
    R4(a, b, c, d, e, 60);
    R4(e, a, b, c, d, 61);
    R4(d, e, a, b, c, 62);
    R4(c, d, e, a, b, 63);
    R4(b, c, d, e, a, 64);
    R4(a, b, c, d, e, 65);
    R4(e, a, b, c, d, 66);
    R4(d, e, a, b, c, 67);
    R4(c, d, e, a, b, 68);
    R4(b, c, d, e, a, 69);
    R4(a, b, c, d, e, 70);
    R4(e, a, b, c, d, 71);
    R4(d, e, a, b, c, 72);
    R4(c, d, e, a, b, 73);
    R4(b, c, d, e, a, 74);
    R4(a, b, c, d, e, 75);
    R4(e, a, b, c, d, 76);
    R4(d, e, a, b, c, 77);
    R4(c, d, e, a, b, 78);
    R4(b, c, d, e, a, 79);

    /* Add the working vars back into context.state[] */
    state[0] += a;
    state[1] += b;
    state[2] += c;
    state[3] += d;
    state[4] += e;
}

/* SHA1Init - Initialize new context */
void SHA1_reset(SHA1_CTX* context)
{
    /* SHA1 initialization constants */
    context->state[0] = 0x67452301;
    context->state[1] = 0xEFCDAB89;
    context->state[2] = 0x98BADCFE;
    context->state[3] = 0x10325476;
    context->state[4] = 0xC3D2E1F0;
    context->count[0] = 0;
    context->count[1] = 0;
}

/* Run your data through this. */
void SHA1_input(SHA1_CTX* context, const unsigned char* data, size_t len)
{
    size_t i = 0;
    uint32_t j = context->count[0];

    if ((context->count[0] += (uint32_t)len << 3) < j)
        context->count[1]++;

    context->count[1] += ((uint32_t)len >> 29);
    j = (j >> 3) & 63;

    if ((j + len) > 63)
    {
        memcpy(&context->buffer[j], data, (i = 64 - j));
        SHA1_transform(context->state, context->buffer);

        for (; i + 63 < len; i += 64)
            SHA1_transform(context->state, &data[i]);

        j = 0;
    }

    memcpy(&context->buffer[j], &data[i], len - i);
}

/* Add padding and return the message digest. */
void SHA1_result(SHA1_CTX* context, SHA1_t digest)
{
    unsigned char finalcount[8];

    for (unsigned int i = 0; i < 8; i++)
        finalcount[i] = (unsigned char) ((context->count[(i >= 4 ? 0 : 1)] >> ((3 - (i & 3)) * 8)) & 255); // Endian independent

    unsigned char c = 0200;
    SHA1_input(context, &c, 1);
    while ((context->count[0] & 504) != 448)
    {
        c = 0000;
        SHA1_input(context, &c, 1);
    }

    SHA1_input(context, finalcount, 8); /* Should cause a SHA1Transform() */

    for (unsigned int i = 0; i < 20; i++)
        digest[i] = (unsigned char)((context->state[i >> 2] >> ((3 - (i & 3)) * 8)) & 255);

    /* Wipe variables */
    memset(context, 0, sizeof(*context));
}

void SHA1(SHA1_t digest, const unsigned char* data, size_t len)
{
    SHA1_CTX ctx;

    SHA1_reset(&ctx);
    SHA1_input(&ctx, data, len);
    SHA1_result(&ctx, digest);
}

#endif // SHA1_IMPLEMENTATION
#endif // __SHA1_H__
