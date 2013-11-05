/*
 * Copyright (c) 2011-2013  University of Texas at Austin. All rights reserved.
 *
 * $COPYRIGHT$
 *
 * Additional copyrights may follow
 *
 * This file is part of PerfExpert.
 *
 * PerfExpert is free software: you can redistribute it and/or modify it under
 * the terms of the The University of Texas at Austin Research License
 *
 * PerfExpert is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE.
 *
 * Author's License follows
 *
 **********************************************************************
 ** Copyright (C) 1990, RSA Data Security, Inc. All rights reserved. **
 **                                                                  **
 ** License to copy and use this software is granted provided that   **
 ** it is identified as the "RSA Data Security, Inc. MD5 Message     **
 ** Digest Algorithm" in all material mentioning or referencing this **
 ** software or this function.                                       **
 **                                                                  **
 ** License is also granted to make and use derivative works         **
 ** provided that such works are identified as "derived from the RSA **
 ** Data Security, Inc. MD5 Message Digest Algorithm" in all         **
 ** material mentioning or referencing the derived work.             **
 **                                                                  **
 ** RSA Data Security, Inc. makes no representations concerning      **
 ** either the merchantability of this software or the suitability   **
 ** of this software for any particular purpose.  It is provided "as **
 ** is" without express or implied warranty of any kind.             **
 **                                                                  **
 ** These notices must be retained in any copies of any part of this **
 ** documentation and/or software.                                   **
 **********************************************************************
 *
 * Adapted by: Leonardo Fialho
 *
 * $HEADER$
 */

#ifndef PERFEXPERT_MD5_H_
#define PERFEXPERT_MD5_H_

#ifndef _STDIO_H
#include <stdio.h>
#endif

#ifndef _STDLIB_H
#include <stdlib.h>
#endif

#ifndef _STRING_H
#include <string.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef struct MD5_CTX {
    unsigned int i[2];
    unsigned int buf[4];
    unsigned char in[64];
    unsigned char digest[16];
} MD5_CTX_t;

#define ROLL(value, bits) (((value) << (bits)) | ((value) >> (32 - (bits))))

#define F(x, y, z) (((x) & (y)) | ((~x) & (z)))
#define G(x, y, z) (((x) & (z)) | ((y) & (~z)))
#define H(x, y, z) ((x) ^ (y) ^ (z))
#define I(x, y, z) ((y) ^ ((x) | (~z)))

#define FF(a, b, c, d, x, s, ac)                         \
    (a) += F ((b), (c), (d)) + (x) + (unsigned int)(ac); \
    (a) = ROLL((a), (s));                                \
    (a) += (b)

#define GG(a, b, c, d, x, s, ac)                         \
    (a) += G ((b), (c), (d)) + (x) + (unsigned int)(ac); \
    (a) = ROLL((a), (s));                                \
    (a) += (b)

#define HH(a, b, c, d, x, s, ac)                         \
    (a) += H ((b), (c), (d)) + (x) + (unsigned int)(ac); \
    (a) = ROLL((a), (s));                                \
    (a) += (b)

#define II(a, b, c, d, x, s, ac)                         \
    (a) += I ((b), (c), (d)) + (x) + (unsigned int)(ac); \
    (a) = ROLL((a), (s));                                \
    (a) += (b)

static unsigned char PADDING[64] = {
    0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

/* md5_transform */
static inline void perfexpert_md5_transform(unsigned int *buf,
    unsigned int *in) {
    unsigned int a = buf[0], b = buf[1], c = buf[2], d = buf[3];

    /* Round 1 */
    #define S11 7
    #define S12 12
    #define S13 17
    #define S14 22
    FF(a, b, c, d, in[ 0], S11, 3614090360); /* 1 */
    FF(d, a, b, c, in[ 1], S12, 3905402710); /* 2 */
    FF(c, d, a, b, in[ 2], S13,  606105819); /* 3 */
    FF(b, c, d, a, in[ 3], S14, 3250441966); /* 4 */
    FF(a, b, c, d, in[ 4], S11, 4118548399); /* 5 */
    FF(d, a, b, c, in[ 5], S12, 1200080426); /* 6 */
    FF(c, d, a, b, in[ 6], S13, 2821735955); /* 7 */
    FF(b, c, d, a, in[ 7], S14, 4249261313); /* 8 */
    FF(a, b, c, d, in[ 8], S11, 1770035416); /* 9 */
    FF(d, a, b, c, in[ 9], S12, 2336552879); /* 10 */
    FF(c, d, a, b, in[10], S13, 4294925233); /* 11 */
    FF(b, c, d, a, in[11], S14, 2304563134); /* 12 */
    FF(a, b, c, d, in[12], S11, 1804603682); /* 13 */
    FF(d, a, b, c, in[13], S12, 4254626195); /* 14 */
    FF(c, d, a, b, in[14], S13, 2792965006); /* 15 */
    FF(b, c, d, a, in[15], S14, 1236535329); /* 16 */

    /* Round 2 */
    #define S21 5
    #define S22 9
    #define S23 14
    #define S24 20
    GG(a, b, c, d, in[ 1], S21, 4129170786); /* 17 */
    GG(d, a, b, c, in[ 6], S22, 3225465664); /* 18 */
    GG(c, d, a, b, in[11], S23,  643717713); /* 19 */
    GG(b, c, d, a, in[ 0], S24, 3921069994); /* 20 */
    GG(a, b, c, d, in[ 5], S21, 3593408605); /* 21 */
    GG(d, a, b, c, in[10], S22,   38016083); /* 22 */
    GG(c, d, a, b, in[15], S23, 3634488961); /* 23 */
    GG(b, c, d, a, in[ 4], S24, 3889429448); /* 24 */
    GG(a, b, c, d, in[ 9], S21,  568446438); /* 25 */
    GG(d, a, b, c, in[14], S22, 3275163606); /* 26 */
    GG(c, d, a, b, in[ 3], S23, 4107603335); /* 27 */
    GG(b, c, d, a, in[ 8], S24, 1163531501); /* 28 */
    GG(a, b, c, d, in[13], S21, 2850285829); /* 29 */
    GG(d, a, b, c, in[ 2], S22, 4243563512); /* 30 */
    GG(c, d, a, b, in[ 7], S23, 1735328473); /* 31 */
    GG(b, c, d, a, in[12], S24, 2368359562); /* 32 */

    /* Round 3 */
    #define S31 4
    #define S32 11
    #define S33 16
    #define S34 23
    HH(a, b, c, d, in[ 5], S31, 4294588738); /* 33 */
    HH(d, a, b, c, in[ 8], S32, 2272392833); /* 34 */
    HH(c, d, a, b, in[11], S33, 1839030562); /* 35 */
    HH(b, c, d, a, in[14], S34, 4259657740); /* 36 */
    HH(a, b, c, d, in[ 1], S31, 2763975236); /* 37 */
    HH(d, a, b, c, in[ 4], S32, 1272893353); /* 38 */
    HH(c, d, a, b, in[ 7], S33, 4139469664); /* 39 */
    HH(b, c, d, a, in[10], S34, 3200236656); /* 40 */
    HH(a, b, c, d, in[13], S31,  681279174); /* 41 */
    HH(d, a, b, c, in[ 0], S32, 3936430074); /* 42 */
    HH(c, d, a, b, in[ 3], S33, 3572445317); /* 43 */
    HH(b, c, d, a, in[ 6], S34,   76029189); /* 44 */
    HH(a, b, c, d, in[ 9], S31, 3654602809); /* 45 */
    HH(d, a, b, c, in[12], S32, 3873151461); /* 46 */
    HH(c, d, a, b, in[15], S33,  530742520); /* 47 */
    HH(b, c, d, a, in[ 2], S34, 3299628645); /* 48 */

    /* Round 4 */
    #define S41 6
    #define S42 10
    #define S43 15
    #define S44 21
    II(a, b, c, d, in[ 0], S41, 4096336452); /* 49 */
    II(d, a, b, c, in[ 7], S42, 1126891415); /* 50 */
    II(c, d, a, b, in[14], S43, 2878612391); /* 51 */
    II(b, c, d, a, in[ 5], S44, 4237533241); /* 52 */
    II(a, b, c, d, in[12], S41, 1700485571); /* 53 */
    II(d, a, b, c, in[ 3], S42, 2399980690); /* 54 */
    II(c, d, a, b, in[10], S43, 4293915773); /* 55 */
    II(b, c, d, a, in[ 1], S44, 2240044497); /* 56 */
    II(a, b, c, d, in[ 8], S41, 1873313359); /* 57 */
    II(d, a, b, c, in[15], S42, 4264355552); /* 58 */
    II(c, d, a, b, in[ 6], S43, 2734768916); /* 59 */
    II(b, c, d, a, in[13], S44, 1309151649); /* 60 */
    II(a, b, c, d, in[ 4], S41, 4149444226); /* 61 */
    II(d, a, b, c, in[11], S42, 3174756917); /* 62 */
    II(c, d, a, b, in[ 2], S43,  718787259); /* 63 */
    II(b, c, d, a, in[ 9], S44, 3951481745); /* 64 */

    buf[0] += a;
    buf[1] += b;
    buf[2] += c;
    buf[3] += d;
}

/* md5_init */
static inline void perfexpert_md5_init(MD5_CTX_t *ctx) {
    ctx->i[0] = ctx->i[1] = (unsigned int)0;
    ctx->buf[0] = (unsigned int)0x67452301;
    ctx->buf[1] = (unsigned int)0xefcdab89;
    ctx->buf[2] = (unsigned int)0x98badcfe;
    ctx->buf[3] = (unsigned int)0x10325476;
}

/* md5_update */
static inline void perfexpert_md5_update(MD5_CTX_t *ctx, unsigned char *inBuf,
    unsigned int inLen) {
    unsigned int in[16];
    unsigned int i, ii;
    int mdi;

    mdi = (int)((ctx->i[0] >> 3) & 0x3F);

    if ((ctx->i[0] + ((unsigned int)inLen << 3)) < ctx->i[0]) {
        ctx->i[1]++;
    }
    ctx->i[0] += ((unsigned int)inLen << 3);
    ctx->i[1] += ((unsigned int)inLen >> 29);

    while (inLen--) {
        ctx->in[mdi++] = *inBuf++;

        if (0x40 == mdi) {
            for (i = 0, ii = 0; i < 16; i++, ii += 4) {
                in[i] = (((unsigned int)ctx->in[ii+3]) << 24) |
                        (((unsigned int)ctx->in[ii+2]) << 16) |
                        (((unsigned int)ctx->in[ii+1]) << 8)  |
                         ((unsigned int)ctx->in[ii]);
            }
            perfexpert_md5_transform(ctx->buf, in);
            mdi = 0;
        }
    }
}

/* md5_final */
static inline void perfexpert_md5_final(MD5_CTX_t *ctx) {
    unsigned int in[16];
    unsigned int i, ii;
    unsigned int padLen;
    int mdi;

    in[14] = ctx->i[0];
    in[15] = ctx->i[1];

    mdi = (int)((ctx->i[0] >> 3) & 0x3F);

    padLen = (mdi < 56) ? (56 - mdi) : (120 - mdi);
    perfexpert_md5_update(ctx, PADDING, padLen);

    for (i = 0, ii = 0; i < 14; i++, ii += 4) {
        in[i] = (((unsigned int)ctx->in[ii+3]) << 24) |
                (((unsigned int)ctx->in[ii+2]) << 16) |
                (((unsigned int)ctx->in[ii+1]) << 8)  |
                 ((unsigned int)ctx->in[ii]);
    }
    perfexpert_md5_transform(ctx->buf, in);

    for (i = 0, ii = 0; i < 4; i++, ii += 4) {
        ctx->digest[ii] =   (unsigned char)(ctx->buf[i] & 0xFF);
        ctx->digest[ii+1] = (unsigned char)((ctx->buf[i] >> 8) & 0xFF);
        ctx->digest[ii+2] = (unsigned char)((ctx->buf[i] >> 16) & 0xFF);
        ctx->digest[ii+3] = (unsigned char)((ctx->buf[i] >> 24) & 0xFF);
    }
}

/* md5_string */
static inline char* perfexpert_md5_string(const char *in) {
    static char rc[33];
    MD5_CTX_t ctx;

    perfexpert_md5_init(&ctx);
    perfexpert_md5_update(&ctx, (unsigned char *)in, strlen(in));
    perfexpert_md5_final(&ctx);

    bzero(rc, 33);
    sprintf(rc,
        "%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x",
        ctx.digest[0],  ctx.digest[1],  ctx.digest[2],  ctx.digest[3],
        ctx.digest[4],  ctx.digest[5],  ctx.digest[6],  ctx.digest[7],
        ctx.digest[8],  ctx.digest[9],  ctx.digest[10], ctx.digest[11],
        ctx.digest[12], ctx.digest[13], ctx.digest[14], ctx.digest[15]);
    return (char *)&rc;
}

#ifdef __cplusplus
}
#endif

#endif /* PERFEXPERT_MD5_H */
