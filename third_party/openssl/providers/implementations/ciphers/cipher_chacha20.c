/*
 * Copyright 2019-2020 The OpenSSL Project Authors. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://www.openssl.org/source/license.html
 */

/* Dispatch functions for chacha20 cipher */

#include "cipher_chacha20.h"
#include "prov/implementations.h"
#include "prov/providercommon.h"
#include "prov/providercommonerr.h"

#define CHACHA20_KEYLEN (CHACHA_KEY_SIZE)
#define CHACHA20_BLKLEN (1)
#define CHACHA20_IVLEN (CHACHA_CTR_SIZE)
/* TODO(3.0) Figure out what flags are required */
#define CHACHA20_FLAGS (EVP_CIPH_CUSTOM_IV | EVP_CIPH_ALWAYS_CALL_INIT)

static OSSL_FUNC_cipher_newctx_fn chacha20_newctx;
static OSSL_FUNC_cipher_freectx_fn chacha20_freectx;
static OSSL_FUNC_cipher_get_params_fn chacha20_get_params;
static OSSL_FUNC_cipher_get_ctx_params_fn chacha20_get_ctx_params;
static OSSL_FUNC_cipher_set_ctx_params_fn chacha20_set_ctx_params;
static OSSL_FUNC_cipher_gettable_ctx_params_fn chacha20_gettable_ctx_params;
static OSSL_FUNC_cipher_settable_ctx_params_fn chacha20_settable_ctx_params;
#define chacha20_cipher cipher_generic_cipher
#define chacha20_update cipher_generic_stream_update
#define chacha20_final cipher_generic_stream_final
#define chacha20_gettable_params cipher_generic_gettable_params

void chacha20_initctx(PROV_CHACHA20_CTX *ctx)
{
    cipher_generic_initkey(ctx, CHACHA20_KEYLEN * 8,
                           CHACHA20_BLKLEN * 8,
                           CHACHA20_IVLEN * 8,
                           0, CHACHA20_FLAGS,
                           PROV_CIPHER_HW_chacha20(CHACHA20_KEYLEN * 8),
                           NULL);
}

static void *chacha20_newctx(void *provctx)
{
    PROV_CHACHA20_CTX *ctx;

    if (!ossl_prov_is_running())
        return NULL;

    ctx = OPENSSL_zalloc(sizeof(*ctx));
    if (ctx != NULL)
        chacha20_initctx(ctx);
    return ctx;
}

static void chacha20_freectx(void *vctx)
{
    PROV_CHACHA20_CTX *ctx = (PROV_CHACHA20_CTX *)vctx;

    if (ctx != NULL) {
        cipher_generic_reset_ctx((PROV_CIPHER_CTX *)vctx);
        OPENSSL_clear_free(ctx, sizeof(*ctx));
    }
}

static int chacha20_get_params(OSSL_PARAM params[])
{
    return cipher_generic_get_params(params, 0, CHACHA20_FLAGS,
                                     CHACHA20_KEYLEN * 8,
                                     CHACHA20_BLKLEN * 8,
                                     CHACHA20_IVLEN * 8);
}

static int chacha20_get_ctx_params(void *vctx, OSSL_PARAM params[])
{
    OSSL_PARAM *p;

    p = OSSL_PARAM_locate(params, OSSL_CIPHER_PARAM_IVLEN);
    if (p != NULL && !OSSL_PARAM_set_size_t(p, CHACHA20_IVLEN)) {
        ERR_raise(ERR_LIB_PROV, PROV_R_FAILED_TO_SET_PARAMETER);
        return 0;
    }
    p = OSSL_PARAM_locate(params, OSSL_CIPHER_PARAM_KEYLEN);
    if (p != NULL && !OSSL_PARAM_set_size_t(p, CHACHA20_KEYLEN)) {
        ERR_raise(ERR_LIB_PROV, PROV_R_FAILED_TO_SET_PARAMETER);
        return 0;
    }

    return 1;
}

static const OSSL_PARAM chacha20_known_gettable_ctx_params[] = {
    OSSL_PARAM_size_t(OSSL_CIPHER_PARAM_KEYLEN, NULL),
    OSSL_PARAM_size_t(OSSL_CIPHER_PARAM_IVLEN, NULL),
    OSSL_PARAM_END
};
const OSSL_PARAM *chacha20_gettable_ctx_params(ossl_unused void *provctx)
{
    return chacha20_known_gettable_ctx_params;
}

static int chacha20_set_ctx_params(void *vctx, const OSSL_PARAM params[])
{
    const OSSL_PARAM *p;
    size_t len;

    p = OSSL_PARAM_locate_const(params, OSSL_CIPHER_PARAM_KEYLEN);
    if (p != NULL) {
        if (!OSSL_PARAM_get_size_t(p, &len)) {
            ERR_raise(ERR_LIB_PROV, PROV_R_FAILED_TO_GET_PARAMETER);
            return 0;
        }
        if (len != CHACHA20_KEYLEN) {
            ERR_raise(ERR_LIB_PROV, PROV_R_INVALID_KEY_LENGTH);
            return 0;
        }
    }
    p = OSSL_PARAM_locate_const(params, OSSL_CIPHER_PARAM_IVLEN);
    if (p != NULL) {
        if (!OSSL_PARAM_get_size_t(p, &len)) {
            ERR_raise(ERR_LIB_PROV, PROV_R_FAILED_TO_GET_PARAMETER);
            return 0;
        }
        if (len != CHACHA20_IVLEN) {
            ERR_raise(ERR_LIB_PROV, PROV_R_INVALID_IV_LENGTH);
            return 0;
        }
    }
    return 1;
}

static const OSSL_PARAM chacha20_known_settable_ctx_params[] = {
    OSSL_PARAM_size_t(OSSL_CIPHER_PARAM_KEYLEN, NULL),
    OSSL_PARAM_size_t(OSSL_CIPHER_PARAM_IVLEN, NULL),
    OSSL_PARAM_END
};
const OSSL_PARAM *chacha20_settable_ctx_params(ossl_unused void *provctx)
{
    return chacha20_known_settable_ctx_params;
}

int chacha20_einit(void *vctx, const unsigned char *key, size_t keylen,
                   const unsigned char *iv, size_t ivlen)
{
    int ret;

    /* The generic function checks for ossl_prov_is_running() */
    ret= cipher_generic_einit(vctx, key, keylen, iv, ivlen);
    if (ret && iv != NULL) {
        PROV_CIPHER_CTX *ctx = (PROV_CIPHER_CTX *)vctx;
        PROV_CIPHER_HW_CHACHA20 *hw = (PROV_CIPHER_HW_CHACHA20 *)ctx->hw;

        hw->initiv(ctx);
    }
    return ret;
}

int chacha20_dinit(void *vctx, const unsigned char *key, size_t keylen,
                   const unsigned char *iv, size_t ivlen)
{
    int ret;

    /* The generic function checks for ossl_prov_is_running() */
    ret= cipher_generic_dinit(vctx, key, keylen, iv, ivlen);
    if (ret && iv != NULL) {
        PROV_CIPHER_CTX *ctx = (PROV_CIPHER_CTX *)vctx;
        PROV_CIPHER_HW_CHACHA20 *hw = (PROV_CIPHER_HW_CHACHA20 *)ctx->hw;

        hw->initiv(ctx);
    }
    return ret;
}

/* chacha20_functions */
const OSSL_DISPATCH chacha20_functions[] = {
    { OSSL_FUNC_CIPHER_NEWCTX, (void (*)(void))chacha20_newctx },
    { OSSL_FUNC_CIPHER_FREECTX, (void (*)(void))chacha20_freectx },
    { OSSL_FUNC_CIPHER_ENCRYPT_INIT, (void (*)(void))chacha20_einit },
    { OSSL_FUNC_CIPHER_DECRYPT_INIT, (void (*)(void))chacha20_dinit },
    { OSSL_FUNC_CIPHER_UPDATE, (void (*)(void))chacha20_update },
    { OSSL_FUNC_CIPHER_FINAL, (void (*)(void))chacha20_final },
    { OSSL_FUNC_CIPHER_CIPHER, (void (*)(void))chacha20_cipher},
    { OSSL_FUNC_CIPHER_GET_PARAMS, (void (*)(void))chacha20_get_params },
    { OSSL_FUNC_CIPHER_GETTABLE_PARAMS,(void (*)(void))chacha20_gettable_params },
    { OSSL_FUNC_CIPHER_GET_CTX_PARAMS, (void (*)(void))chacha20_get_ctx_params },
    { OSSL_FUNC_CIPHER_GETTABLE_CTX_PARAMS,
        (void (*)(void))chacha20_gettable_ctx_params },
    { OSSL_FUNC_CIPHER_SET_CTX_PARAMS, (void (*)(void))chacha20_set_ctx_params },
    { OSSL_FUNC_CIPHER_SETTABLE_CTX_PARAMS,
        (void (*)(void))chacha20_settable_ctx_params },
    { 0, NULL }
};

