/*
 * Copyright 2019-2020 The OpenSSL Project Authors. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://www.openssl.org/source/license.html
 */

/* Dispatch functions for RC4_HMAC_MD5 cipher */

/*
 * MD5 and RC4 low level APIs are deprecated for public use, but still ok for
 * internal use.
 */
#include "internal/deprecated.h"

#include "cipher_rc4_hmac_md5.h"
#include "prov/implementations.h"
#include "prov/providercommon.h"
#include "prov/providercommonerr.h"

/* TODO(3.0) Figure out what flags are required */
#define RC4_HMAC_MD5_FLAGS (EVP_CIPH_STREAM_CIPHER | EVP_CIPH_VARIABLE_LENGTH  \
                            | EVP_CIPH_FLAG_AEAD_CIPHER)

#define RC4_HMAC_MD5_KEY_BITS (16 * 8)
#define RC4_HMAC_MD5_BLOCK_BITS (1 * 8)
#define RC4_HMAC_MD5_IV_BITS 0
#define RC4_HMAC_MD5_MODE 0

#define GET_HW(ctx) ((PROV_CIPHER_HW_RC4_HMAC_MD5 *)ctx->base.hw)

static OSSL_FUNC_cipher_newctx_fn rc4_hmac_md5_newctx;
static OSSL_FUNC_cipher_freectx_fn rc4_hmac_md5_freectx;
static OSSL_FUNC_cipher_get_ctx_params_fn rc4_hmac_md5_get_ctx_params;
static OSSL_FUNC_cipher_gettable_ctx_params_fn rc4_hmac_md5_gettable_ctx_params;
static OSSL_FUNC_cipher_set_ctx_params_fn rc4_hmac_md5_set_ctx_params;
static OSSL_FUNC_cipher_settable_ctx_params_fn rc4_hmac_md5_settable_ctx_params;
static OSSL_FUNC_cipher_get_params_fn rc4_hmac_md5_get_params;
#define rc4_hmac_md5_gettable_params cipher_generic_gettable_params
#define rc4_hmac_md5_einit cipher_generic_einit
#define rc4_hmac_md5_dinit cipher_generic_dinit
#define rc4_hmac_md5_update cipher_generic_stream_update
#define rc4_hmac_md5_final cipher_generic_stream_final
#define rc4_hmac_md5_cipher cipher_generic_cipher

static void *rc4_hmac_md5_newctx(void *provctx)
{
    PROV_RC4_HMAC_MD5_CTX *ctx;

    if (!ossl_prov_is_running())
        return NULL;

    ctx = OPENSSL_zalloc(sizeof(*ctx));
    if (ctx != NULL)
        cipher_generic_initkey(ctx, RC4_HMAC_MD5_KEY_BITS,
                               RC4_HMAC_MD5_BLOCK_BITS,
                               RC4_HMAC_MD5_IV_BITS,
                               RC4_HMAC_MD5_MODE, RC4_HMAC_MD5_FLAGS,
                               PROV_CIPHER_HW_rc4_hmac_md5(RC4_HMAC_MD5_KEY_BITS),
                               NULL);
     return ctx;
}

static void rc4_hmac_md5_freectx(void *vctx)
{
    PROV_RC4_HMAC_MD5_CTX *ctx = (PROV_RC4_HMAC_MD5_CTX *)vctx;

    cipher_generic_reset_ctx((PROV_CIPHER_CTX *)vctx);
    OPENSSL_clear_free(ctx,  sizeof(*ctx));
}

static const OSSL_PARAM rc4_hmac_md5_known_gettable_ctx_params[] = {
    OSSL_PARAM_size_t(OSSL_CIPHER_PARAM_KEYLEN, NULL),
    OSSL_PARAM_size_t(OSSL_CIPHER_PARAM_IVLEN, NULL),
    OSSL_PARAM_size_t(OSSL_CIPHER_PARAM_AEAD_TLS1_AAD_PAD, NULL),
    OSSL_PARAM_END
};
const OSSL_PARAM *rc4_hmac_md5_gettable_ctx_params(ossl_unused void *provctx)
{
    return rc4_hmac_md5_known_gettable_ctx_params;
}

static int rc4_hmac_md5_get_ctx_params(void *vctx, OSSL_PARAM params[])
{
    PROV_RC4_HMAC_MD5_CTX *ctx = (PROV_RC4_HMAC_MD5_CTX *)vctx;
    OSSL_PARAM *p;

    p = OSSL_PARAM_locate(params, OSSL_CIPHER_PARAM_KEYLEN);
    if (p != NULL && !OSSL_PARAM_set_size_t(p, ctx->base.keylen)) {
        ERR_raise(ERR_LIB_PROV, PROV_R_FAILED_TO_SET_PARAMETER);
        return 0;
    }

    p = OSSL_PARAM_locate(params, OSSL_CIPHER_PARAM_IVLEN);
    if (p != NULL && !OSSL_PARAM_set_size_t(p, ctx->base.ivlen)) {
        ERR_raise(ERR_LIB_PROV, PROV_R_FAILED_TO_SET_PARAMETER);
        return 0;
    }
    p = OSSL_PARAM_locate(params, OSSL_CIPHER_PARAM_AEAD_TLS1_AAD_PAD);
    if (p != NULL && !OSSL_PARAM_set_size_t(p, ctx->tls_aad_pad_sz)) {
        ERR_raise(ERR_LIB_PROV, PROV_R_FAILED_TO_SET_PARAMETER);
        return 0;
    }
    return 1;
}

static const OSSL_PARAM rc4_hmac_md5_known_settable_ctx_params[] = {
    OSSL_PARAM_size_t(OSSL_CIPHER_PARAM_KEYLEN, NULL),
    OSSL_PARAM_size_t(OSSL_CIPHER_PARAM_IVLEN, NULL),
    OSSL_PARAM_octet_string(OSSL_CIPHER_PARAM_AEAD_TLS1_AAD, NULL, 0),
    OSSL_PARAM_END
};
const OSSL_PARAM *rc4_hmac_md5_settable_ctx_params(ossl_unused void *provctx)
{
    return rc4_hmac_md5_known_settable_ctx_params;
}

static int rc4_hmac_md5_set_ctx_params(void *vctx, const OSSL_PARAM params[])
{
    PROV_RC4_HMAC_MD5_CTX *ctx = (PROV_RC4_HMAC_MD5_CTX *)vctx;
    const OSSL_PARAM *p;
    size_t sz;

    p = OSSL_PARAM_locate_const(params, OSSL_CIPHER_PARAM_KEYLEN);
    if (p != NULL) {
        if (!OSSL_PARAM_get_size_t(p, &sz)) {
            ERR_raise(ERR_LIB_PROV, PROV_R_FAILED_TO_GET_PARAMETER);
            return 0;
        }
        if (ctx->base.keylen != sz) {
            ERR_raise(ERR_LIB_PROV, PROV_R_INVALID_KEY_LENGTH);
            return 0;
        }
    }

    p = OSSL_PARAM_locate_const(params, OSSL_CIPHER_PARAM_IVLEN);
    if (p != NULL) {
        if (!OSSL_PARAM_get_size_t(p, &sz)) {
            ERR_raise(ERR_LIB_PROV, PROV_R_FAILED_TO_GET_PARAMETER);
            return 0;
        }
        if (ctx->base.ivlen != sz) {
            ERR_raise(ERR_LIB_PROV, PROV_R_INVALID_IV_LENGTH);
            return 0;
        }
    }

    p = OSSL_PARAM_locate_const(params, OSSL_CIPHER_PARAM_AEAD_TLS1_AAD);
    if (p != NULL) {
        if (p->data_type != OSSL_PARAM_OCTET_STRING) {
            ERR_raise(ERR_LIB_PROV, PROV_R_FAILED_TO_GET_PARAMETER);
            return 0;
        }
        sz = GET_HW(ctx)->tls_init(&ctx->base, p->data, p->data_size);
        if (sz == 0) {
            ERR_raise(ERR_LIB_PROV, PROV_R_INVALID_DATA);
            return 0;
        }
        ctx->tls_aad_pad_sz = sz;
    }
    p = OSSL_PARAM_locate_const(params, OSSL_CIPHER_PARAM_AEAD_TLS1_AAD);
    if (p != NULL) {
        if (p->data_type != OSSL_PARAM_OCTET_STRING) {
            ERR_raise(ERR_LIB_PROV, PROV_R_FAILED_TO_GET_PARAMETER);
            return 0;
        }
        GET_HW(ctx)->init_mackey(&ctx->base, p->data, p->data_size);
    }
    return 1;
}

static int rc4_hmac_md5_get_params(OSSL_PARAM params[])
{
    return cipher_generic_get_params(params, RC4_HMAC_MD5_MODE,
                                     RC4_HMAC_MD5_FLAGS,
                                     RC4_HMAC_MD5_KEY_BITS,
                                     RC4_HMAC_MD5_BLOCK_BITS,
                                     RC4_HMAC_MD5_IV_BITS);
}

const OSSL_DISPATCH rc4_hmac_md5_functions[] = {
    { OSSL_FUNC_CIPHER_NEWCTX, (void (*)(void))rc4_hmac_md5_newctx },
    { OSSL_FUNC_CIPHER_FREECTX, (void (*)(void))rc4_hmac_md5_freectx },
    { OSSL_FUNC_CIPHER_ENCRYPT_INIT, (void (*)(void))rc4_hmac_md5_einit },
    { OSSL_FUNC_CIPHER_DECRYPT_INIT, (void (*)(void))rc4_hmac_md5_dinit },
    { OSSL_FUNC_CIPHER_UPDATE, (void (*)(void))rc4_hmac_md5_update },
    { OSSL_FUNC_CIPHER_FINAL, (void (*)(void))rc4_hmac_md5_final },
    { OSSL_FUNC_CIPHER_CIPHER, (void (*)(void))rc4_hmac_md5_cipher },
    { OSSL_FUNC_CIPHER_GET_PARAMS, (void (*)(void))rc4_hmac_md5_get_params },
    { OSSL_FUNC_CIPHER_GETTABLE_PARAMS,
        (void (*)(void))rc4_hmac_md5_gettable_params },
    { OSSL_FUNC_CIPHER_GET_CTX_PARAMS,
        (void (*)(void))rc4_hmac_md5_get_ctx_params },
    { OSSL_FUNC_CIPHER_GETTABLE_CTX_PARAMS,
        (void (*)(void))rc4_hmac_md5_gettable_ctx_params },
    { OSSL_FUNC_CIPHER_SET_CTX_PARAMS,
        (void (*)(void))rc4_hmac_md5_set_ctx_params },
    { OSSL_FUNC_CIPHER_SETTABLE_CTX_PARAMS,
        (void (*)(void))rc4_hmac_md5_settable_ctx_params },
    { 0, NULL }
};
