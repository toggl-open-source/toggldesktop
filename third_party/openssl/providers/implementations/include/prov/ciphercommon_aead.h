/*
 * Copyright 2019-2020 The OpenSSL Project Authors. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://www.openssl.org/source/license.html
 */

#define UNINITIALISED_SIZET ((size_t)-1)

/* TODO(3.0) Figure out what flags are really needed */
#define AEAD_FLAGS (EVP_CIPH_FLAG_AEAD_CIPHER           \
                    | EVP_CIPH_CUSTOM_IV                \
                    | EVP_CIPH_ALWAYS_CALL_INIT         \
                    | EVP_CIPH_CTRL_INIT                \
                    | EVP_CIPH_CUSTOM_COPY)

#define IMPLEMENT_aead_cipher(alg, lc, UCMODE, flags, kbits, blkbits, ivbits)  \
static OSSL_FUNC_cipher_get_params_fn alg##_##kbits##_##lc##_get_params;         \
static int alg##_##kbits##_##lc##_get_params(OSSL_PARAM params[])              \
{                                                                              \
    return cipher_generic_get_params(params, EVP_CIPH_##UCMODE##_MODE,         \
                                     flags, kbits, blkbits, ivbits);           \
}                                                                              \
static OSSL_FUNC_cipher_newctx_fn alg##kbits##lc##_newctx;                       \
static void * alg##kbits##lc##_newctx(void *provctx)                           \
{                                                                              \
    return alg##_##lc##_newctx(provctx, kbits);                                \
}                                                                              \
const OSSL_DISPATCH alg##kbits##lc##_functions[] = {                           \
    { OSSL_FUNC_CIPHER_NEWCTX, (void (*)(void))alg##kbits##lc##_newctx },      \
    { OSSL_FUNC_CIPHER_FREECTX, (void (*)(void))alg##_##lc##_freectx },        \
    { OSSL_FUNC_CIPHER_ENCRYPT_INIT, (void (*)(void)) lc##_einit },            \
    { OSSL_FUNC_CIPHER_DECRYPT_INIT, (void (*)(void)) lc##_dinit },            \
    { OSSL_FUNC_CIPHER_UPDATE, (void (*)(void)) lc##_stream_update },          \
    { OSSL_FUNC_CIPHER_FINAL, (void (*)(void)) lc##_stream_final },            \
    { OSSL_FUNC_CIPHER_CIPHER, (void (*)(void)) lc##_cipher },                 \
    { OSSL_FUNC_CIPHER_GET_PARAMS,                                             \
      (void (*)(void)) alg##_##kbits##_##lc##_get_params },                    \
    { OSSL_FUNC_CIPHER_GET_CTX_PARAMS,                                         \
      (void (*)(void)) lc##_get_ctx_params },                                  \
    { OSSL_FUNC_CIPHER_SET_CTX_PARAMS,                                         \
      (void (*)(void)) lc##_set_ctx_params },                                  \
    { OSSL_FUNC_CIPHER_GETTABLE_PARAMS,                                        \
      (void (*)(void))cipher_generic_gettable_params },                        \
    { OSSL_FUNC_CIPHER_GETTABLE_CTX_PARAMS,                                    \
      (void (*)(void))cipher_aead_gettable_ctx_params },                       \
    { OSSL_FUNC_CIPHER_SETTABLE_CTX_PARAMS,                                    \
      (void (*)(void))cipher_aead_settable_ctx_params },                       \
    { 0, NULL }                                                                \
}
