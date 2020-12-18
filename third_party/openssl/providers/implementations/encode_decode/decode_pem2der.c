/*
 * Copyright 2020 The OpenSSL Project Authors. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://www.openssl.org/source/license.html
 */

/*
 * RSA low level APIs are deprecated for public use, but still ok for
 * internal use.
 */
#include "internal/deprecated.h"

#include <string.h>

#include <openssl/core_dispatch.h>
#include <openssl/core_names.h>
#include <openssl/crypto.h>
#include <openssl/err.h>
#include <openssl/params.h>
#include <openssl/pem.h>
#include "prov/bio.h"
#include "prov/implementations.h"
#include "prov/providercommonerr.h"
#include "endecoder_local.h"

static int read_pem(PROV_CTX *provctx, OSSL_CORE_BIO *cin,
                    char **pem_name, char **pem_header,
                    unsigned char **data, long *len)
{
    BIO *in = bio_new_from_core_bio(provctx, cin);
    int ok = (PEM_read_bio(in, pem_name, pem_header, data, len) > 0);

    BIO_free(in);
    return ok;
}

static OSSL_FUNC_decoder_newctx_fn pem2der_newctx;
static OSSL_FUNC_decoder_freectx_fn pem2der_freectx;
static OSSL_FUNC_decoder_gettable_params_fn pem2der_gettable_params;
static OSSL_FUNC_decoder_get_params_fn pem2der_get_params;
static OSSL_FUNC_decoder_decode_fn pem2der_decode;

/*
 * Context used for PEM to DER decoding.
 */
struct pem2der_ctx_st {
    PROV_CTX *provctx;
};

static void *pem2der_newctx(void *provctx)
{
    struct pem2der_ctx_st *ctx = OPENSSL_zalloc(sizeof(*ctx));

    if (ctx != NULL)
        ctx->provctx = provctx;
    return ctx;
}

static void pem2der_freectx(void *vctx)
{
    struct pem2der_ctx_st *ctx = vctx;

    OPENSSL_free(ctx);
}

static const OSSL_PARAM *pem2der_gettable_params(void *provctx)
{
    static const OSSL_PARAM gettables[] = {
        { OSSL_DECODER_PARAM_INPUT_TYPE, OSSL_PARAM_UTF8_PTR, NULL, 0, 0 },
        OSSL_PARAM_END,
    };

    return gettables;
}

static int pem2der_get_params(OSSL_PARAM params[])
{
    OSSL_PARAM *p;

    p = OSSL_PARAM_locate(params, OSSL_DECODER_PARAM_INPUT_TYPE);
    if (p != NULL && !OSSL_PARAM_set_utf8_ptr(p, "PEM"))
        return 0;

    return 1;
}

/* pem_password_cb compatible function */
struct pem2der_pass_data_st {
    OSSL_PASSPHRASE_CALLBACK *cb;
    void *cbarg;
};

static int pem2der_pass_helper(char *buf, int num, int w, void *data)
{
    struct pem2der_pass_data_st *pass_data = data;
    size_t plen;

    if (pass_data == NULL
        || pass_data->cb == NULL
        || !pass_data->cb(buf, num, &plen, NULL, pass_data->cbarg))
        return -1;
    return (int)plen;
}

static int pem2der_decode(void *vctx, OSSL_CORE_BIO *cin,
                          OSSL_CALLBACK *data_cb, void *data_cbarg,
                          OSSL_PASSPHRASE_CALLBACK *pw_cb, void *pw_cbarg)
{
    struct pem2der_ctx_st *ctx = vctx;
    char *pem_name = NULL, *pem_header = NULL;
    unsigned char *der = NULL;
    long der_len = 0;
    int ok = 0;

    if (read_pem(ctx->provctx, cin, &pem_name, &pem_header,
                 &der, &der_len) <= 0)
        return 0;

    /*
     * 10 is the number of characters in "Proc-Type:", which
     * PEM_get_EVP_CIPHER_INFO() requires to be present.
     * If the PEM header has less characters than that, it's
     * not worth spending cycles on it.
     */
    if (strlen(pem_header) > 10) {
        EVP_CIPHER_INFO cipher;
        struct pem2der_pass_data_st pass_data;

        pass_data.cb = pw_cb;
        pass_data.cbarg = pw_cbarg;
        if (!PEM_get_EVP_CIPHER_INFO(pem_header, &cipher)
            || !PEM_do_header(&cipher, der, &der_len,
                              pem2der_pass_helper, &pass_data))
            goto end;
    }

    {
        OSSL_PARAM params[3];

        params[0] =
            OSSL_PARAM_construct_utf8_string(OSSL_OBJECT_PARAM_DATA_TYPE,
                                             pem_name, 0);
        params[1] =
            OSSL_PARAM_construct_octet_string(OSSL_OBJECT_PARAM_DATA,
                                              der, der_len);
        params[2] = OSSL_PARAM_construct_end();

        ok = data_cb(params, data_cbarg);
    }

 end:
    OPENSSL_free(pem_name);
    OPENSSL_free(pem_header);
    OPENSSL_free(der);
    return ok;
}

const OSSL_DISPATCH pem_to_der_decoder_functions[] = {
    { OSSL_FUNC_DECODER_NEWCTX, (void (*)(void))pem2der_newctx },
    { OSSL_FUNC_DECODER_FREECTX, (void (*)(void))pem2der_freectx },
    { OSSL_FUNC_DECODER_GETTABLE_PARAMS,
      (void (*)(void))pem2der_gettable_params },
    { OSSL_FUNC_DECODER_GET_PARAMS,
      (void (*)(void))pem2der_get_params },
    { OSSL_FUNC_DECODER_DECODE, (void (*)(void))pem2der_decode },
    { 0, NULL }
};
