/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * SHA1 routine optimized to do word accesses rather than byte accesses,
 * and to avoid unnecessary copies into the context array.
 *
 * This was based on the git SHA1 implementation.
 */

#include <xen/bitops.h>
#include <xen/sha1.h>
#include <xen/string.h>
#include <xen/types.h>
#include <xen/unaligned.h>

#define SHA1_BLOCK_SIZE         64
#define SHA1_WORKSPACE_WORDS    16
#define SHA1_WORKSPACE_MASK     (SHA1_WORKSPACE_WORDS - 1)

struct sha1_state {
    uint64_t count;
    uint32_t state[SHA1_DIGEST_SIZE / 4];
    uint8_t buffer[SHA1_BLOCK_SIZE];
};

/* This "rolls" over the 512-bit array named w */
#define W(i) w[(i) & SHA1_WORKSPACE_MASK]

static uint32_t blend(const uint32_t w[SHA1_WORKSPACE_WORDS], size_t i)
{
    return rol32(W(i + 13) ^ W(i + 8) ^ W(i + 2) ^ W(i), 1);
}

/**
 * sha1_transform - single block SHA1 transform
 *
 * @digest: 160 bit digest to update
 * @data:   512 bits of data to hash
 *
 * This function executes SHA-1's internal compression function.  It updates the
 * 160-bit internal state (@digest) with a single 512-bit data block (@data).
 */
static void sha1_transform(uint32_t *digest, const uint8_t *data)
{
    uint32_t a, b, c, d, e, t;
    uint32_t w[SHA1_WORKSPACE_WORDS];
    unsigned int i = 0;

    a = digest[0];
    b = digest[1];
    c = digest[2];
    d = digest[3];
    e = digest[4];

    /* Round 1 - iterations 0-16 take their input from 'data' */
    for ( ; i < 16; ++i )
    {
        t = get_unaligned_be32((uint32_t *)data + i);
        W(i) = t;
        e += t + rol32(a, 5) + (((c ^ d) & b) ^ d) + 0x5a827999U;
        b = ror32(b, 2);
        t = e; e = d; d = c; c = b; b = a; a = t;
    }

    /* Round 1 - tail. Input from 512-bit mixing array */
    for ( ; i < 20; ++i )
    {
        t = blend(w, i);
        W(i) = t;
        e += t + rol32(a, 5) + (((c ^ d) & b) ^ d) + 0x5a827999U;
        b = ror32(b, 2);
        t = e; e = d; d = c; c = b; b = a; a = t;
    }

    /* Round 2 */
    for ( ; i < 40; ++i )
    {
        t = blend(w, i);
        W(i) = t;
        e += t + rol32(a, 5) + (b ^ c ^ d) + 0x6ed9eba1U;
        b = ror32(b, 2);
        t = e; e = d; d = c; c = b; b = a; a = t;
    }

    /* Round 3 */
    for ( ; i < 60; ++i )
    {
        t = blend(w, i);
        W(i) = t;
        e += t + rol32(a, 5) + ((b & c) + (d & (b ^ c))) + 0x8f1bbcdcU;
        b = ror32(b, 2);
        t = e; e = d; d = c; c = b; b = a; a = t;
    }

    /* Round 4 */
    for ( ; i < 80; ++i )
    {
        t = blend(w, i);
        W(i) = t;
        e += t + rol32(a, 5) + (b ^ c ^ d) + 0xca62c1d6U;
        b = ror32(b, 2);
        t = e; e = d; d = c; c = b; b = a; a = t;
    }

    digest[0] += a;
    digest[1] += b;
    digest[2] += c;
    digest[3] += d;
    digest[4] += e;
}

static void sha1_init(struct sha1_state *sctx)
{
    sctx->state[0] = 0x67452301UL;
    sctx->state[1] = 0xefcdab89UL;
    sctx->state[2] = 0x98badcfeUL;
    sctx->state[3] = 0x10325476UL;
    sctx->state[4] = 0xc3d2e1f0UL;
    sctx->count = 0;
}

static void sha1_update(struct sha1_state *sctx, const uint8_t *msg, size_t len)
{
    unsigned int partial = sctx->count % SHA1_BLOCK_SIZE;

    sctx->count += len;

    if ( (partial + len) >= SHA1_BLOCK_SIZE )
    {
        if ( partial )
        {
            unsigned int rem = SHA1_BLOCK_SIZE - partial;

            /* Fill the partial block. */
            memcpy(sctx->buffer + partial, msg, rem);
            msg += rem;
            len -= rem;

            sha1_transform(sctx->state, sctx->buffer);
        }

        for ( ; len >= SHA1_BLOCK_SIZE; len -= SHA1_BLOCK_SIZE )
        {
            sha1_transform(sctx->state, msg);
            msg += SHA1_BLOCK_SIZE;
        }
        partial = 0;
    }

    /* Remaining data becomes partial. */
    memcpy(sctx->buffer + partial, msg, len);
}

static void sha1_final(struct sha1_state *sctx, uint8_t out[SHA1_DIGEST_SIZE])
{
    const int bit_offset = SHA1_BLOCK_SIZE - sizeof(__be64);
    unsigned int partial = sctx->count % SHA1_BLOCK_SIZE;

    __be32 *digest = (__be32 *)out;
    unsigned int i;

    /* Start padding */
    sctx->buffer[partial++] = 0x80;

    if ( partial > bit_offset )
    {
        /* Need one extra block, so properly pad this one with zeroes */
        memset(sctx->buffer + partial, 0x0, SHA1_BLOCK_SIZE - partial);
        sha1_transform(sctx->state, sctx->buffer);
        partial = 0;
    }
    /* Pad up to the location of the bit count */
    memset(sctx->buffer + partial, 0x0, bit_offset - partial);

    /* Append the bit count */
    put_unaligned_be64(sctx->count << 3, &sctx->buffer[bit_offset]);
    sha1_transform(sctx->state, sctx->buffer);

    /* Store state in digest */
    for ( i = 0; i < SHA1_DIGEST_SIZE / sizeof(__be32); i++ )
        put_unaligned_be32(sctx->state[i], &digest[i]);
}

void sha1_hash(uint8_t digest[SHA1_DIGEST_SIZE], const void *msg, size_t len)
{
    struct sha1_state sctx;

    sha1_init(&sctx);
    sha1_update(&sctx, msg, len);
    sha1_final(&sctx, digest);
}
