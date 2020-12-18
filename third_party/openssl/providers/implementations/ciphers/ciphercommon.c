/*
 * Copyright 2019-2020 The OpenSSL Project Authors. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://www.openssl.org/source/license.html
 */

/*
 * Generic dispatch table functions for ciphers.
 */

/* For SSL3_VERSION */
#include <openssl/ssl.h>
#include "ciphercommon_local.h"
#include "prov/provider_ctx.h"
#include "prov/providercommon.h"
#include "prov/providercommonerr.h"

/*-
 * Generic cipher functions for OSSL_PARAM gettables and settables
 */
static const OSSL_PARAM cipher_known_gettable_params[] = {
    OSSL_PARAM_uint(OSSL_CIPHER_PARAM_MODE, NULL),
    OSSL_PARAM_size_t(OSSL_CIPHER_PARAM_KEYLEN, NULL),
    OSSL_PARAM_size_t(OSSL_CIPHER_PARAM_IVLEN, NULL),
    OSSL_PARAM_size_t(OSSL_CIPHER_PARAM_BLOCK_SIZE, NULL),
    OSSL_PARAM_ulong(OSSL_CIPHER_PARAM_FLAGS, NULL),
    { OSSL_CIPHER_PARAM_TLS_MAC, OSSL_PARAM_OCTET_PTR, NULL, 0, OSSL_PARAM_UNMODIFIED },
    OSSL_PARAM_END
};
const OSSL_PARAM *cipher_generic_gettable_params(void *provctx)
{
    return cipher_known_gettable_params;
}

int cipher_generic_get_params(OSSL_PARAM params[], unsigned int md,
                              unsigned long flags,
                              size_t kbits, size_t blkbits, size_t ivbits)
{
    OSSL_PARAM *p;

    p = OSSL_PARAM_locate(params, OSSL_CIPHER_PARAM_MODE);
    if (p != NULL && !OSSL_PARAM_set_uint(p, md)) {
        ERR_raise(ERR_LIB_PROV, PROV_R_FAILED_TO_SET_PARAMETER);
        return 0;
    }
    p = OSSL_PARAM_locate(params, OSSL_CIPHER_PARAM_FLAGS);
    if (p != NULL && !OSSL_PARAM_set_ulong(p, flags)) {
        ERR_raise(ERR_LIB_PROV, PROV_R_FAILED_TO_SET_PARAMETER);
        return 0;
    }
    p = OSSL_PARAM_locate(params, OSSL_CIPHER_PARAM_KEYLEN);
    if (p != NULL && !OSSL_PARAM_set_size_t(p, kbits / 8)) {
        ERR_raise(ERR_LIB_PROV, PROV_R_FAILED_TO_SET_PARAMETER);
        return 0;
    }
    p = OSSL_PARAM_locate(params, OSSL_CIPHER_PARAM_BLOCK_SIZE);
    if (p != NULL && !OSSL_PARAM_set_size_t(p, blkbits / 8)) {
        ERR_raise(ERR_LIB_PROV, PROV_R_FAILED_TO_SET_PARAMETER);
        return 0;
    }
    p = OSSL_PARAM_locate(params, OSSL_CIPHER_PARAM_IVLEN);
    if (p != NULL && !OSSL_PARAM_set_size_t(p, ivbits / 8)) {
        ERR_raise(ERR_LIB_PROV, PROV_R_FAILED_TO_SET_PARAMETER);
        return 0;
    }
    return 1;
}

CIPHER_DEFAULT_GETTABLE_CTX_PARAMS_START(cipher_generic)
CIPHER_DEFAULT_GETTABLE_CTX_PARAMS_END(cipher_generic)

CIPHER_DEFAULT_SETTABLE_CTX_PARAMS_START(cipher_generic)
OSSL_PARAM_uint(OSSL_CIPHER_PARAM_TLS_VERSION, NULL),
OSSL_PARAM_size_t(OSSL_CIPHER_PARAM_TLS_MAC_SIZE, NULL),
CIPHER_DEFAULT_SETTABLE_CTX_PARAMS_END(cipher_generic)

/*
 * Variable key length cipher functions for OSSL_PARAM settables
 */

int cipher_var_keylen_set_ctx_params(void *vctx, const OSSL_PARAM params[])
{
    PROV_CIPHER_CTX *ctx = (PROV_CIPHER_CTX *)vctx;
    const OSSL_PARAM *p;

    if (!cipher_generic_set_ctx_params(vctx, params))
        return 0;
    p = OSSL_PARAM_locate_const(params, OSSL_CIPHER_PARAM_KEYLEN);
    if (p != NULL) {
        size_t keylen;

        if (!OSSL_PARAM_get_size_t(p, &keylen)) {
            ERR_raise(ERR_LIB_PROV, PROV_R_FAILED_TO_GET_PARAMETER);
            return 0;
        }
        ctx->keylen = keylen;
    }
    return 1;
}

CIPHER_DEFAULT_SETTABLE_CTX_PARAMS_START(cipher_var_keylen)
OSSL_PARAM_size_t(OSSL_CIPHER_PARAM_KEYLEN, NULL),
CIPHER_DEFAULT_SETTABLE_CTX_PARAMS_END(cipher_var_keylen)

/*-
 * AEAD cipher functions for OSSL_PARAM gettables and settables
 */
static const OSSL_PARAM cipher_aead_known_gettable_ctx_params[] = {
    OSSL_PARAM_size_t(OSSL_CIPHER_PARAM_KEYLEN, NULL),
    OSSL_PARAM_size_t(OSSL_CIPHER_PARAM_IVLEN, NULL),
    OSSL_PARAM_size_t(OSSL_CIPHER_PARAM_AEAD_TAGLEN, NULL),
    OSSL_PARAM_octet_string(OSSL_CIPHER_PARAM_IV, NULL, 0),
    OSSL_PARAM_octet_string(OSSL_CIPHER_PARAM_IV_STATE, NULL, 0),
    OSSL_PARAM_octet_string(OSSL_CIPHER_PARAM_AEAD_TAG, NULL, 0),
    OSSL_PARAM_size_t(OSSL_CIPHER_PARAM_AEAD_TLS1_AAD_PAD, NULL),
    OSSL_PARAM_octet_string(OSSL_CIPHER_PARAM_AEAD_TLS1_GET_IV_GEN, NULL, 0),
    OSSL_PARAM_END
};
const OSSL_PARAM *cipher_aead_gettable_ctx_params(ossl_unused void *provctx)
{
    return cipher_aead_known_gettable_ctx_params;
}

static const OSSL_PARAM cipher_aead_known_settable_ctx_params[] = {
    OSSL_PARAM_size_t(OSSL_CIPHER_PARAM_AEAD_IVLEN, NULL),
    OSSL_PARAM_octet_string(OSSL_CIPHER_PARAM_AEAD_TAG, NULL, 0),
    OSSL_PARAM_octet_string(OSSL_CIPHER_PARAM_AEAD_TLS1_AAD, NULL, 0),
    OSSL_PARAM_octet_string(OSSL_CIPHER_PARAM_AEAD_TLS1_IV_FIXED, NULL, 0),
    OSSL_PARAM_octet_string(OSSL_CIPHER_PARAM_AEAD_TLS1_SET_IV_INV, NULL, 0),
    OSSL_PARAM_END
};
const OSSL_PARAM *cipher_aead_settable_ctx_params(ossl_unused void *provctx)
{
    return cipher_aead_known_settable_ctx_params;
}

void cipher_generic_reset_ctx(PROV_CIPHER_CTX *ctx)
{
    if (ctx != NULL && ctx->alloced) {
        OPENSSL_free(ctx->tlsmac);
        ctx->alloced = 0;
        ctx->tlsmac = NULL;
    }
}

static int cipher_generic_init_internal(PROV_CIPHER_CTX *ctx,
                                        const unsigned char *key, size_t keylen,
                                        const unsigned char *iv, size_t ivlen,
                                        int enc)
{
    ctx->num = 0;
    ctx->bufsz = 0;
    ctx->updated = 0;
    ctx->enc = enc ? 1 : 0;

    if (!ossl_prov_is_running())
        return 0;

    if (iv != NULL && ctx->mode != EVP_CIPH_ECB_MODE) {
        if (!cipher_generic_initiv(ctx, iv, ivlen))
            return 0;
    }
    if (key != NULL) {
        if ((ctx->flags & EVP_CIPH_VARIABLE_LENGTH) == 0) {
            if (keylen != ctx->keylen) {
                ERR_raise(ERR_LIB_PROV, PROV_R_INVALID_KEYLEN);
                return 0;
            }
        } else {
            ctx->keylen = keylen;
        }
        return ctx->hw->init(ctx, key, ctx->keylen);
    }
    return 1;
}

int cipher_generic_einit(void *vctx, const unsigned char *key, size_t keylen,
                         const unsigned char *iv, size_t ivlen)
{
    return cipher_generic_init_internal((PROV_CIPHER_CTX *)vctx, key, keylen,
                                        iv, ivlen, 1);
}

int cipher_generic_dinit(void *vctx, const unsigned char *key, size_t keylen,
                         const unsigned char *iv, size_t ivlen)
{
    return cipher_generic_init_internal((PROV_CIPHER_CTX *)vctx, key, keylen,
                                        iv, ivlen, 0);
}

/* Max padding including padding length byte */
#define MAX_PADDING 256

int cipher_generic_block_update(void *vctx, unsigned char *out, size_t *outl,
                                size_t outsize, const unsigned char *in,
                                size_t inl)
{
    size_t outlint = 0;
    PROV_CIPHER_CTX *ctx = (PROV_CIPHER_CTX *)vctx;
    size_t blksz = ctx->blocksize;
    size_t nextblocks;

    if (ctx->tlsversion > 0) {
        /*
         * Each update call corresponds to a TLS record and is individually
         * padded
         */

        /* Sanity check inputs */
        if (in == NULL
                || in != out
                || outsize < inl
                || !ctx->pad) {
            ERR_raise(ERR_LIB_PROV, PROV_R_CIPHER_OPERATION_FAILED);
            return 0;
        }

        if (ctx->enc) {
            unsigned char padval;
            size_t padnum, loop;

            /* Add padding */

            padnum = blksz - (inl % blksz);

            if (outsize < inl + padnum) {
                ERR_raise(ERR_LIB_PROV, PROV_R_CIPHER_OPERATION_FAILED);
                return 0;
            }

            if (padnum > MAX_PADDING) {
                ERR_raise(ERR_LIB_PROV, PROV_R_CIPHER_OPERATION_FAILED);
                return 0;
            }
            padval = (unsigned char)(padnum - 1);
            if (ctx->tlsversion == SSL3_VERSION) {
                if (padnum > 1)
                    memset(out + inl, 0, padnum - 1);
                *(out + inl + padnum - 1) = padval;
            } else {
                /* we need to add 'padnum' padding bytes of value padval */
                for (loop = inl; loop < inl + padnum; loop++)
                    out[loop] = padval;
            }
            inl += padnum;
        }

        if ((inl % blksz) != 0) {
            ERR_raise(ERR_LIB_PROV, PROV_R_CIPHER_OPERATION_FAILED);
            return 0;
        }


        /* Shouldn't normally fail */
        if (!ctx->hw->cipher(ctx, out, in, inl)) {
            ERR_raise(ERR_LIB_PROV, PROV_R_CIPHER_OPERATION_FAILED);
            return 0;
        }

        if (ctx->alloced) {
            OPENSSL_free(ctx->tlsmac);
            ctx->alloced = 0;
            ctx->tlsmac = NULL;
        }

        /* This only fails if padding is publicly invalid */
        *outl = inl;
        if (!ctx->enc
                && !tlsunpadblock(ctx->libctx, ctx->tlsversion, out, outl,
                                  blksz, &ctx->tlsmac, &ctx->alloced,
                                  ctx->tlsmacsize, 0)) {
            ERR_raise(ERR_LIB_PROV, PROV_R_CIPHER_OPERATION_FAILED);
            return 0;
        }
        return 1;
    }

    if (ctx->bufsz != 0)
        nextblocks = fillblock(ctx->buf, &ctx->bufsz, blksz, &in, &inl);
    else
        nextblocks = inl & ~(blksz-1);

    /*
     * If we're decrypting and we end an update on a block boundary we hold
     * the last block back in case this is the last update call and the last
     * block is padded.
     */
    if (ctx->bufsz == blksz && (ctx->enc || inl > 0 || !ctx->pad)) {
        if (outsize < blksz) {
            ERR_raise(ERR_LIB_PROV, PROV_R_OUTPUT_BUFFER_TOO_SMALL);
            return 0;
        }
        if (!ctx->hw->cipher(ctx, out, ctx->buf, blksz)) {
            ERR_raise(ERR_LIB_PROV, PROV_R_CIPHER_OPERATION_FAILED);
            return 0;
        }
        ctx->bufsz = 0;
        outlint = blksz;
        out += blksz;
    }
    if (nextblocks > 0) {
        if (!ctx->enc && ctx->pad && nextblocks == inl) {
            if (!ossl_assert(inl >= blksz)) {
                ERR_raise(ERR_LIB_PROV, PROV_R_OUTPUT_BUFFER_TOO_SMALL);
                return 0;
            }
            nextblocks -= blksz;
        }
        outlint += nextblocks;
        if (outsize < outlint) {
            ERR_raise(ERR_LIB_PROV, PROV_R_OUTPUT_BUFFER_TOO_SMALL);
            return 0;
        }
    }
    if (nextblocks > 0) {
        if (!ctx->hw->cipher(ctx, out, in, nextblocks)) {
            ERR_raise(ERR_LIB_PROV, PROV_R_CIPHER_OPERATION_FAILED);
            return 0;
        }
        in += nextblocks;
        inl -= nextblocks;
    }
    if (inl != 0 && !trailingdata(ctx->buf, &ctx->bufsz, blksz, &in, &inl)) {
        /* ERR_raise already called */
        return 0;
    }

    *outl = outlint;
    return inl == 0;
}

int cipher_generic_block_final(void *vctx, unsigned char *out, size_t *outl,
                               size_t outsize)
{
    PROV_CIPHER_CTX *ctx = (PROV_CIPHER_CTX *)vctx;
    size_t blksz = ctx->blocksize;

    if (!ossl_prov_is_running())
        return 0;

    if (ctx->tlsversion > 0) {
        /* We never finalize TLS, so this is an error */
        ERR_raise(ERR_LIB_PROV, PROV_R_CIPHER_OPERATION_FAILED);
        return 0;
    }

    if (ctx->enc) {
        if (ctx->pad) {
            padblock(ctx->buf, &ctx->bufsz, blksz);
        } else if (ctx->bufsz == 0) {
            *outl = 0;
            return 1;
        } else if (ctx->bufsz != blksz) {
            ERR_raise(ERR_LIB_PROV, PROV_R_WRONG_FINAL_BLOCK_LENGTH);
            return 0;
        }

        if (outsize < blksz) {
            ERR_raise(ERR_LIB_PROV, PROV_R_OUTPUT_BUFFER_TOO_SMALL);
            return 0;
        }
        if (!ctx->hw->cipher(ctx, out, ctx->buf, blksz)) {
            ERR_raise(ERR_LIB_PROV, PROV_R_CIPHER_OPERATION_FAILED);
            return 0;
        }
        ctx->bufsz = 0;
        *outl = blksz;
        return 1;
    }

    /* Decrypting */
    if (ctx->bufsz != blksz) {
        if (ctx->bufsz == 0 && !ctx->pad) {
            *outl = 0;
            return 1;
        }
        ERR_raise(ERR_LIB_PROV, PROV_R_WRONG_FINAL_BLOCK_LENGTH);
        return 0;
    }

    if (!ctx->hw->cipher(ctx, ctx->buf, ctx->buf, blksz)) {
        ERR_raise(ERR_LIB_PROV, PROV_R_CIPHER_OPERATION_FAILED);
        return 0;
    }

    if (ctx->pad && !unpadblock(ctx->buf, &ctx->bufsz, blksz)) {
        /* ERR_raise already called */
        return 0;
    }

    if (outsize < ctx->bufsz) {
        ERR_raise(ERR_LIB_PROV, PROV_R_OUTPUT_BUFFER_TOO_SMALL);
        return 0;
    }
    memcpy(out, ctx->buf, ctx->bufsz);
    *outl = ctx->bufsz;
    ctx->bufsz = 0;
    return 1;
}

int cipher_generic_stream_update(void *vctx, unsigned char *out, size_t *outl,
                                 size_t outsize, const unsigned char *in,
                                 size_t inl)
{
    PROV_CIPHER_CTX *ctx = (PROV_CIPHER_CTX *)vctx;

    if (inl == 0) {
        *outl = 0;
        return 1;
    }

    if (outsize < inl) {
        ERR_raise(ERR_LIB_PROV, PROV_R_OUTPUT_BUFFER_TOO_SMALL);
        return 0;
    }

    if (!ctx->hw->cipher(ctx, out, in, inl)) {
        ERR_raise(ERR_LIB_PROV, PROV_R_CIPHER_OPERATION_FAILED);
        return 0;
    }

    *outl = inl;
    /*
     * Remove any TLS padding. Only used by cipher_aes_cbc_hmac_sha1_hw.c and
     * cipher_aes_cbc_hmac_sha256_hw.c
     */
    if (!ctx->enc && ctx->removetlspad > 0) {
        /* The actual padding length */
        *outl -= out[inl - 1] + 1;

        /* MAC and explicit IV */
        *outl -= ctx->removetlspad;
    }

    return 1;
}
int cipher_generic_stream_final(void *vctx, unsigned char *out, size_t *outl,
                                size_t outsize)
{
    if (!ossl_prov_is_running())
        return 0;

    *outl = 0;
    return 1;
}

int cipher_generic_cipher(void *vctx,
                          unsigned char *out, size_t *outl, size_t outsize,
                          const unsigned char *in, size_t inl)
{
    PROV_CIPHER_CTX *ctx = (PROV_CIPHER_CTX *)vctx;

    if (!ossl_prov_is_running())
        return 0;

    if (outsize < inl) {
        ERR_raise(ERR_LIB_PROV, PROV_R_OUTPUT_BUFFER_TOO_SMALL);
        return 0;
    }

    if (!ctx->hw->cipher(ctx, out, in, inl)) {
        ERR_raise(ERR_LIB_PROV, PROV_R_CIPHER_OPERATION_FAILED);
        return 0;
    }

    *outl = inl;
    return 1;
}

int cipher_generic_get_ctx_params(void *vctx, OSSL_PARAM params[])
{
    PROV_CIPHER_CTX *ctx = (PROV_CIPHER_CTX *)vctx;
    OSSL_PARAM *p;

    p = OSSL_PARAM_locate(params, OSSL_CIPHER_PARAM_IVLEN);
    if (p != NULL && !OSSL_PARAM_set_size_t(p, ctx->ivlen)) {
        ERR_raise(ERR_LIB_PROV, PROV_R_FAILED_TO_SET_PARAMETER);
        return 0;
    }
    p = OSSL_PARAM_locate(params, OSSL_CIPHER_PARAM_PADDING);
    if (p != NULL && !OSSL_PARAM_set_uint(p, ctx->pad)) {
        ERR_raise(ERR_LIB_PROV, PROV_R_FAILED_TO_SET_PARAMETER);
        return 0;
    }
    p = OSSL_PARAM_locate(params, OSSL_CIPHER_PARAM_IV);
    if (p != NULL
        && !OSSL_PARAM_set_octet_ptr(p, &ctx->oiv, ctx->ivlen)
        && !OSSL_PARAM_set_octet_string(p, &ctx->oiv, ctx->ivlen)) {
        ERR_raise(ERR_LIB_PROV, PROV_R_FAILED_TO_SET_PARAMETER);
        return 0;
    }
    p = OSSL_PARAM_locate(params, OSSL_CIPHER_PARAM_IV_STATE);
    if (p != NULL
        && !OSSL_PARAM_set_octet_ptr(p, &ctx->iv, ctx->ivlen)
        && !OSSL_PARAM_set_octet_string(p, &ctx->iv, ctx->ivlen)) {
        ERR_raise(ERR_LIB_PROV, PROV_R_FAILED_TO_SET_PARAMETER);
        return 0;
    }
    p = OSSL_PARAM_locate(params, OSSL_CIPHER_PARAM_NUM);
    if (p != NULL && !OSSL_PARAM_set_uint(p, ctx->num)) {
        ERR_raise(ERR_LIB_PROV, PROV_R_FAILED_TO_SET_PARAMETER);
        return 0;
    }
    p = OSSL_PARAM_locate(params, OSSL_CIPHER_PARAM_KEYLEN);
    if (p != NULL && !OSSL_PARAM_set_size_t(p, ctx->keylen)) {
        ERR_raise(ERR_LIB_PROV, PROV_R_FAILED_TO_SET_PARAMETER);
        return 0;
    }
    p = OSSL_PARAM_locate(params, OSSL_CIPHER_PARAM_TLS_MAC);
    if (p != NULL
        && !OSSL_PARAM_set_octet_ptr(p, ctx->tlsmac, ctx->tlsmacsize)) {
        ERR_raise(ERR_LIB_PROV, PROV_R_FAILED_TO_SET_PARAMETER);
        return 0;
    }
    return 1;
}

int cipher_generic_set_ctx_params(void *vctx, const OSSL_PARAM params[])
{
    PROV_CIPHER_CTX *ctx = (PROV_CIPHER_CTX *)vctx;
    const OSSL_PARAM *p;

    p = OSSL_PARAM_locate_const(params, OSSL_CIPHER_PARAM_PADDING);
    if (p != NULL) {
        unsigned int pad;

        if (!OSSL_PARAM_get_uint(p, &pad)) {
            ERR_raise(ERR_LIB_PROV, PROV_R_FAILED_TO_GET_PARAMETER);
            return 0;
        }
        ctx->pad = pad ? 1 : 0;
    }
    p = OSSL_PARAM_locate_const(params, OSSL_CIPHER_PARAM_TLS_VERSION);
    if (p != NULL) {
        if (!OSSL_PARAM_get_uint(p, &ctx->tlsversion)) {
            ERR_raise(ERR_LIB_PROV, PROV_R_FAILED_TO_GET_PARAMETER);
            return 0;
        }
    }
    p = OSSL_PARAM_locate_const(params, OSSL_CIPHER_PARAM_TLS_MAC_SIZE);
    if (p != NULL) {
        if (!OSSL_PARAM_get_size_t(p, &ctx->tlsmacsize)) {
            ERR_raise(ERR_LIB_PROV, PROV_R_FAILED_TO_GET_PARAMETER);
            return 0;
        }
    }
    p = OSSL_PARAM_locate_const(params, OSSL_CIPHER_PARAM_NUM);
    if (p != NULL) {
        unsigned int num;

        if (!OSSL_PARAM_get_uint(p, &num)) {
            ERR_raise(ERR_LIB_PROV, PROV_R_FAILED_TO_GET_PARAMETER);
            return 0;
        }
        ctx->num = num;
    }
    return 1;
}

int cipher_generic_initiv(PROV_CIPHER_CTX *ctx, const unsigned char *iv,
                          size_t ivlen)
{
    if (ivlen != ctx->ivlen
        || ivlen > sizeof(ctx->iv)) {
        ERR_raise(ERR_LIB_PROV, PROV_R_INVALID_IVLEN);
        return 0;
    }
    ctx->iv_set = 1;
    memcpy(ctx->iv, iv, ivlen);
    memcpy(ctx->oiv, iv, ivlen);
    return 1;
}

void cipher_generic_initkey(void *vctx, size_t kbits, size_t blkbits,
                            size_t ivbits, unsigned int mode, uint64_t flags,
                            const PROV_CIPHER_HW *hw, void *provctx)
{
    PROV_CIPHER_CTX *ctx = (PROV_CIPHER_CTX *)vctx;

    ctx->flags = flags;
    ctx->pad = 1;
    ctx->keylen = ((kbits) / 8);
    ctx->ivlen = ((ivbits) / 8);
    ctx->hw = hw;
    ctx->mode = mode;
    ctx->blocksize = blkbits / 8;
    if (provctx != NULL)
        ctx->libctx = PROV_LIBRARY_CONTEXT_OF(provctx); /* used for rand */
}
