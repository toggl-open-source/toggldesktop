/*
 * Copyright 2019-2020 The OpenSSL Project Authors. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://www.openssl.org/source/license.html
 */

#include <openssl/params.h>
#include <openssl/core_dispatch.h>
#include <openssl/core_names.h>
#include <openssl/evp.h>
#include "internal/cryptlib.h"
#include "crypto/modes.h"

#define MAXCHUNK    ((size_t)1 << (sizeof(long) * 8 - 2))
#define MAXBITCHUNK ((size_t)1 << (sizeof(size_t) * 8 - 4))

#define GENERIC_BLOCK_SIZE 16
#define IV_STATE_UNINITIALISED 0  /* initial state is not initialized */
#define IV_STATE_BUFFERED      1  /* iv has been copied to the iv buffer */
#define IV_STATE_COPIED        2  /* iv has been copied from the iv buffer */
#define IV_STATE_FINISHED      3  /* the iv has been used - so don't reuse it */

#define PROV_CIPHER_FUNC(type, name, args) typedef type (* OSSL_##name##_fn)args

typedef struct prov_cipher_hw_st PROV_CIPHER_HW;
typedef struct prov_cipher_ctx_st PROV_CIPHER_CTX;

typedef int (PROV_CIPHER_HW_FN)(PROV_CIPHER_CTX *dat, unsigned char *out,
                                const unsigned char *in, size_t len);

/* TODO(3.0): VERIFY ME */
#define MAX_TLS_MAC_SIZE    48

struct prov_cipher_ctx_st {
    block128_f block;
    union {
        cbc128_f cbc;
        ctr128_f ctr;
        ecb128_f ecb;
    } stream;

    unsigned int mode;
    size_t keylen;           /* key size (in bytes) */
    size_t ivlen;
    size_t blocksize;
    size_t bufsz;            /* Number of bytes in buf */
    unsigned int cts_mode;   /* Use to set the type for CTS modes */
    unsigned int pad : 1;    /* Whether padding should be used or not */
    unsigned int enc : 1;    /* Set to 1 for encrypt, or 0 otherwise */
    unsigned int iv_set : 1; /* Set when the iv is copied to the iv/oiv buffers */
    unsigned int updated : 1; /* Set to 1 during update for one shot ciphers */


    unsigned int tlsversion; /* If TLS padding is in use the TLS version number */
    unsigned char *tlsmac;   /* tls MAC extracted from the last record */
    int alloced;             /*
                              * Whether the tlsmac data has been allocated or
                              * points into the user buffer.
                              */
    size_t tlsmacsize;       /* Size of the TLS MAC */
    size_t removetlspad;     /*
                              * Length of the fixed size data to remove when
                              * removing TLS padding (equals mac size plus
                              * IV size if applicable)
                              */

    /*
     * num contains the number of bytes of |iv| which are valid for modes that
     * manage partial blocks themselves.
     */
    unsigned int num;
    uint64_t flags;

    /* The original value of the iv */
    unsigned char oiv[GENERIC_BLOCK_SIZE];
    /* Buffer of partial blocks processed via update calls */
    unsigned char buf[GENERIC_BLOCK_SIZE];
    unsigned char iv[GENERIC_BLOCK_SIZE];
    const PROV_CIPHER_HW *hw; /* hardware specific functions */
    const void *ks; /* Pointer to algorithm specific key data */
    OPENSSL_CTX *libctx;
};

struct prov_cipher_hw_st {
    int (*init)(PROV_CIPHER_CTX *dat, const uint8_t *key, size_t keylen);
    PROV_CIPHER_HW_FN *cipher;
    void (*copyctx)(PROV_CIPHER_CTX *dst, const PROV_CIPHER_CTX *src);
};

void cipher_generic_reset_ctx(PROV_CIPHER_CTX *ctx);
OSSL_FUNC_cipher_encrypt_init_fn cipher_generic_einit;
OSSL_FUNC_cipher_decrypt_init_fn cipher_generic_dinit;
OSSL_FUNC_cipher_update_fn cipher_generic_block_update;
OSSL_FUNC_cipher_final_fn cipher_generic_block_final;
OSSL_FUNC_cipher_update_fn cipher_generic_stream_update;
OSSL_FUNC_cipher_final_fn cipher_generic_stream_final;
OSSL_FUNC_cipher_cipher_fn cipher_generic_cipher;
OSSL_FUNC_cipher_get_ctx_params_fn cipher_generic_get_ctx_params;
OSSL_FUNC_cipher_set_ctx_params_fn cipher_generic_set_ctx_params;
OSSL_FUNC_cipher_gettable_params_fn     cipher_generic_gettable_params;
OSSL_FUNC_cipher_gettable_ctx_params_fn cipher_generic_gettable_ctx_params;
OSSL_FUNC_cipher_settable_ctx_params_fn cipher_generic_settable_ctx_params;
OSSL_FUNC_cipher_set_ctx_params_fn cipher_var_keylen_set_ctx_params;
OSSL_FUNC_cipher_settable_ctx_params_fn cipher_var_keylen_settable_ctx_params;
OSSL_FUNC_cipher_gettable_ctx_params_fn cipher_aead_gettable_ctx_params;
OSSL_FUNC_cipher_settable_ctx_params_fn cipher_aead_settable_ctx_params;

int cipher_generic_get_params(OSSL_PARAM params[], unsigned int md,
                              unsigned long flags,
                              size_t kbits, size_t blkbits, size_t ivbits);
void cipher_generic_initkey(void *vctx, size_t kbits, size_t blkbits,
                            size_t ivbits, unsigned int mode, uint64_t flags,
                            const PROV_CIPHER_HW *hw, void *provctx);

#define IMPLEMENT_generic_cipher_func(alg, UCALG, lcmode, UCMODE, flags, kbits,\
                                      blkbits, ivbits, typ)                    \
const OSSL_DISPATCH alg##kbits##lcmode##_functions[] = {                       \
    { OSSL_FUNC_CIPHER_NEWCTX,                                                 \
      (void (*)(void)) alg##_##kbits##_##lcmode##_newctx },                    \
    { OSSL_FUNC_CIPHER_FREECTX, (void (*)(void)) alg##_freectx },              \
    { OSSL_FUNC_CIPHER_DUPCTX, (void (*)(void)) alg##_dupctx },                \
    { OSSL_FUNC_CIPHER_ENCRYPT_INIT, (void (*)(void))cipher_generic_einit },   \
    { OSSL_FUNC_CIPHER_DECRYPT_INIT, (void (*)(void))cipher_generic_dinit },   \
    { OSSL_FUNC_CIPHER_UPDATE, (void (*)(void))cipher_generic_##typ##_update },\
    { OSSL_FUNC_CIPHER_FINAL, (void (*)(void))cipher_generic_##typ##_final },  \
    { OSSL_FUNC_CIPHER_CIPHER, (void (*)(void))cipher_generic_cipher },        \
    { OSSL_FUNC_CIPHER_GET_PARAMS,                                             \
      (void (*)(void)) alg##_##kbits##_##lcmode##_get_params },                \
    { OSSL_FUNC_CIPHER_GET_CTX_PARAMS,                                         \
      (void (*)(void))cipher_generic_get_ctx_params },                         \
    { OSSL_FUNC_CIPHER_SET_CTX_PARAMS,                                         \
      (void (*)(void))cipher_generic_set_ctx_params },                         \
    { OSSL_FUNC_CIPHER_GETTABLE_PARAMS,                                        \
      (void (*)(void))cipher_generic_gettable_params },                        \
    { OSSL_FUNC_CIPHER_GETTABLE_CTX_PARAMS,                                    \
      (void (*)(void))cipher_generic_gettable_ctx_params },                    \
    { OSSL_FUNC_CIPHER_SETTABLE_CTX_PARAMS,                                    \
     (void (*)(void))cipher_generic_settable_ctx_params },                     \
    { 0, NULL }                                                                \
};

#define IMPLEMENT_var_keylen_cipher_func(alg, UCALG, lcmode, UCMODE, flags,    \
                                         kbits, blkbits, ivbits, typ)          \
const OSSL_DISPATCH alg##kbits##lcmode##_functions[] = {                       \
    { OSSL_FUNC_CIPHER_NEWCTX,                                                 \
      (void (*)(void)) alg##_##kbits##_##lcmode##_newctx },                    \
    { OSSL_FUNC_CIPHER_FREECTX, (void (*)(void)) alg##_freectx },              \
    { OSSL_FUNC_CIPHER_DUPCTX, (void (*)(void)) alg##_dupctx },                \
    { OSSL_FUNC_CIPHER_ENCRYPT_INIT, (void (*)(void))cipher_generic_einit },   \
    { OSSL_FUNC_CIPHER_DECRYPT_INIT, (void (*)(void))cipher_generic_dinit },   \
    { OSSL_FUNC_CIPHER_UPDATE, (void (*)(void))cipher_generic_##typ##_update },\
    { OSSL_FUNC_CIPHER_FINAL, (void (*)(void))cipher_generic_##typ##_final },  \
    { OSSL_FUNC_CIPHER_CIPHER, (void (*)(void))cipher_generic_cipher },        \
    { OSSL_FUNC_CIPHER_GET_PARAMS,                                             \
      (void (*)(void)) alg##_##kbits##_##lcmode##_get_params },                \
    { OSSL_FUNC_CIPHER_GET_CTX_PARAMS,                                         \
      (void (*)(void))cipher_generic_get_ctx_params },                         \
    { OSSL_FUNC_CIPHER_SET_CTX_PARAMS,                                         \
      (void (*)(void))cipher_var_keylen_set_ctx_params },                      \
    { OSSL_FUNC_CIPHER_GETTABLE_PARAMS,                                        \
      (void (*)(void))cipher_generic_gettable_params },                        \
    { OSSL_FUNC_CIPHER_GETTABLE_CTX_PARAMS,                                    \
      (void (*)(void))cipher_generic_gettable_ctx_params },                    \
    { OSSL_FUNC_CIPHER_SETTABLE_CTX_PARAMS,                                    \
     (void (*)(void))cipher_var_keylen_settable_ctx_params },                  \
    { 0, NULL }                                                                \
};


#define IMPLEMENT_generic_cipher_genfn(alg, UCALG, lcmode, UCMODE, flags,      \
                                       kbits, blkbits, ivbits, typ)            \
static OSSL_FUNC_cipher_get_params_fn alg##_##kbits##_##lcmode##_get_params;     \
static int alg##_##kbits##_##lcmode##_get_params(OSSL_PARAM params[])          \
{                                                                              \
    return cipher_generic_get_params(params, EVP_CIPH_##UCMODE##_MODE, flags,  \
                                     kbits, blkbits, ivbits);                  \
}                                                                              \
static OSSL_FUNC_cipher_newctx_fn alg##_##kbits##_##lcmode##_newctx;             \
static void * alg##_##kbits##_##lcmode##_newctx(void *provctx)                 \
{                                                                              \
     PROV_##UCALG##_CTX *ctx = ossl_prov_is_running() ? OPENSSL_zalloc(sizeof(*ctx))\
                                                     : NULL;                   \
     if (ctx != NULL) {                                                        \
         cipher_generic_initkey(ctx, kbits, blkbits, ivbits,                   \
                                EVP_CIPH_##UCMODE##_MODE, flags,               \
                                PROV_CIPHER_HW_##alg##_##lcmode(kbits),        \
                                provctx);                                      \
     }                                                                         \
     return ctx;                                                               \
}                                                                              \

#define IMPLEMENT_generic_cipher(alg, UCALG, lcmode, UCMODE, flags, kbits,     \
                                 blkbits, ivbits, typ)                         \
IMPLEMENT_generic_cipher_genfn(alg, UCALG, lcmode, UCMODE, flags, kbits,       \
                               blkbits, ivbits, typ)                           \
IMPLEMENT_generic_cipher_func(alg, UCALG, lcmode, UCMODE, flags, kbits,        \
                              blkbits, ivbits, typ)

#define IMPLEMENT_var_keylen_cipher(alg, UCALG, lcmode, UCMODE, flags, kbits,  \
                                    blkbits, ivbits, typ)                      \
IMPLEMENT_generic_cipher_genfn(alg, UCALG, lcmode, UCMODE, flags, kbits,       \
                               blkbits, ivbits, typ)                           \
IMPLEMENT_var_keylen_cipher_func(alg, UCALG, lcmode, UCMODE, flags, kbits,     \
                                 blkbits, ivbits, typ)

PROV_CIPHER_HW_FN cipher_hw_generic_cbc;
PROV_CIPHER_HW_FN cipher_hw_generic_ecb;
PROV_CIPHER_HW_FN cipher_hw_generic_ofb128;
PROV_CIPHER_HW_FN cipher_hw_generic_cfb128;
PROV_CIPHER_HW_FN cipher_hw_generic_cfb8;
PROV_CIPHER_HW_FN cipher_hw_generic_cfb1;
PROV_CIPHER_HW_FN cipher_hw_generic_ctr;
PROV_CIPHER_HW_FN cipher_hw_chunked_cbc;
PROV_CIPHER_HW_FN cipher_hw_chunked_cfb8;
PROV_CIPHER_HW_FN cipher_hw_chunked_cfb128;
PROV_CIPHER_HW_FN cipher_hw_chunked_ofb128;
#define cipher_hw_chunked_ecb  cipher_hw_generic_ecb
#define cipher_hw_chunked_ctr  cipher_hw_generic_ctr
#define cipher_hw_chunked_cfb1 cipher_hw_generic_cfb1

#define IMPLEMENT_CIPHER_HW_OFB(MODE, NAME, CTX_NAME, KEY_NAME, FUNC_PREFIX)   \
static int cipher_hw_##NAME##_##MODE##_cipher(PROV_CIPHER_CTX *ctx,            \
                                         unsigned char *out,                   \
                                         const unsigned char *in, size_t len)  \
{                                                                              \
    int num = ctx->num;                                                        \
    KEY_NAME *key = &(((CTX_NAME *)ctx)->ks.ks);                               \
                                                                               \
    while (len >= MAXCHUNK) {                                                  \
        FUNC_PREFIX##_encrypt(in, out, MAXCHUNK, key, ctx->iv, &num);          \
        len -= MAXCHUNK;                                                       \
        in += MAXCHUNK;                                                        \
        out += MAXCHUNK;                                                       \
    }                                                                          \
    if (len > 0) {                                                             \
        FUNC_PREFIX##_encrypt(in, out, (long)len, key, ctx->iv, &num);         \
    }                                                                          \
    ctx->num = num;                                                            \
    return 1;                                                                  \
}

#define IMPLEMENT_CIPHER_HW_ECB(MODE, NAME, CTX_NAME, KEY_NAME, FUNC_PREFIX)   \
static int cipher_hw_##NAME##_##MODE##_cipher(PROV_CIPHER_CTX *ctx,            \
                                         unsigned char *out,                   \
                                         const unsigned char *in, size_t len)  \
{                                                                              \
    size_t i, bl = ctx->blocksize;                                             \
    KEY_NAME *key = &(((CTX_NAME *)ctx)->ks.ks);                               \
                                                                               \
    if (len < bl)                                                              \
        return 1;                                                              \
    for (i = 0, len -= bl; i <= len; i += bl)                                  \
        FUNC_PREFIX##_encrypt(in + i, out + i, key, ctx->enc);                 \
    return 1;                                                                  \
}

#define IMPLEMENT_CIPHER_HW_CBC(MODE, NAME, CTX_NAME, KEY_NAME, FUNC_PREFIX)   \
static int cipher_hw_##NAME##_##MODE##_cipher(PROV_CIPHER_CTX *ctx,            \
                                         unsigned char *out,                   \
                                         const unsigned char *in, size_t len)  \
{                                                                              \
    KEY_NAME *key = &(((CTX_NAME *)ctx)->ks.ks);                               \
                                                                               \
    while (len >= MAXCHUNK) {                                                  \
        FUNC_PREFIX##_encrypt(in, out, MAXCHUNK, key, ctx->iv, ctx->enc);      \
        len -= MAXCHUNK;                                                       \
        in += MAXCHUNK;                                                        \
        out += MAXCHUNK;                                                       \
    }                                                                          \
    if (len > 0)                                                               \
        FUNC_PREFIX##_encrypt(in, out, (long)len, key, ctx->iv, ctx->enc);     \
    return 1;                                                                  \
}

#define IMPLEMENT_CIPHER_HW_CFB(MODE, NAME, CTX_NAME, KEY_NAME, FUNC_PREFIX)   \
static int cipher_hw_##NAME##_##MODE##_cipher(PROV_CIPHER_CTX *ctx,            \
                                         unsigned char *out,                   \
                                         const unsigned char *in, size_t len)  \
{                                                                              \
    size_t chunk = MAXCHUNK;                                                   \
    KEY_NAME *key = &(((CTX_NAME *)ctx)->ks.ks);                               \
    int num = ctx->num;                                                        \
                                                                               \
    if (len < chunk)                                                           \
        chunk = len;                                                           \
    while (len > 0 && len >= chunk) {                                          \
        FUNC_PREFIX##_encrypt(in, out, (long)chunk, key, ctx->iv, &num,        \
                              ctx->enc);                                       \
        len -= chunk;                                                          \
        in += chunk;                                                           \
        out += chunk;                                                          \
        if (len < chunk)                                                       \
            chunk = len;                                                       \
    }                                                                          \
    ctx->num = num;                                                            \
    return 1;                                                                  \
}

#define IMPLEMENT_CIPHER_HW_COPYCTX(name, CTX_TYPE)                            \
static void name(PROV_CIPHER_CTX *dst, const PROV_CIPHER_CTX *src)             \
{                                                                              \
    CTX_TYPE *sctx = (CTX_TYPE *)src;                                          \
    CTX_TYPE *dctx = (CTX_TYPE *)dst;                                          \
                                                                               \
    *dctx = *sctx;                                                             \
    dst->ks = &dctx->ks.ks;                                                    \
}

#define CIPHER_DEFAULT_GETTABLE_CTX_PARAMS_START(name)                         \
static const OSSL_PARAM name##_known_gettable_ctx_params[] = {                 \
    OSSL_PARAM_size_t(OSSL_CIPHER_PARAM_KEYLEN, NULL),                         \
    OSSL_PARAM_size_t(OSSL_CIPHER_PARAM_IVLEN, NULL),                          \
    OSSL_PARAM_uint(OSSL_CIPHER_PARAM_PADDING, NULL),                          \
    OSSL_PARAM_uint(OSSL_CIPHER_PARAM_NUM, NULL),                              \
    OSSL_PARAM_octet_string(OSSL_CIPHER_PARAM_IV, NULL, 0),                    \
    OSSL_PARAM_octet_string(OSSL_CIPHER_PARAM_IV_STATE, NULL, 0),

#define CIPHER_DEFAULT_GETTABLE_CTX_PARAMS_END(name)                           \
    OSSL_PARAM_END                                                             \
};                                                                             \
const OSSL_PARAM * name##_gettable_ctx_params(void *provctx)                   \
{                                                                              \
    return name##_known_gettable_ctx_params;                                   \
}

#define CIPHER_DEFAULT_SETTABLE_CTX_PARAMS_START(name)                         \
static const OSSL_PARAM name##_known_settable_ctx_params[] = {                 \
    OSSL_PARAM_uint(OSSL_CIPHER_PARAM_PADDING, NULL),                          \
    OSSL_PARAM_uint(OSSL_CIPHER_PARAM_NUM, NULL),
#define CIPHER_DEFAULT_SETTABLE_CTX_PARAMS_END(name)                           \
    OSSL_PARAM_END                                                             \
};                                                                             \
const OSSL_PARAM * name##_settable_ctx_params(void *provctx)                   \
{                                                                              \
    return name##_known_settable_ctx_params;                                   \
}

int cipher_generic_initiv(PROV_CIPHER_CTX *ctx, const unsigned char *iv,
                          size_t ivlen);

size_t fillblock(unsigned char *buf, size_t *buflen, size_t blocksize,
                 const unsigned char **in, size_t *inlen);
int trailingdata(unsigned char *buf, size_t *buflen, size_t blocksize,
                 const unsigned char **in, size_t *inlen);

