/*
 * Copyright 2019-2020 The OpenSSL Project Authors. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://www.openssl.org/source/license.html
 */

#include <openssl/err.h>
#include <openssl/cryptoerr.h>
#include <openssl/provider.h>
#include <openssl/core_names.h>
#include "internal/provider.h"

OSSL_PROVIDER *OSSL_PROVIDER_try_load(OPENSSL_CTX *libctx, const char *name)
{
    OSSL_PROVIDER *prov = NULL;

    /* Find it or create it */
    if ((prov = ossl_provider_find(libctx, name, 0)) == NULL
        && (prov = ossl_provider_new(libctx, name, NULL, 0)) == NULL)
        return NULL;

    if (!ossl_provider_activate(prov)) {
        ossl_provider_free(prov);
        return NULL;
    }

    return prov;
}

OSSL_PROVIDER *OSSL_PROVIDER_load(OPENSSL_CTX *libctx, const char *name)
{
    /* Any attempt to load a provider disables auto-loading of defaults */
    if (ossl_provider_disable_fallback_loading(libctx))
        return OSSL_PROVIDER_try_load(libctx, name);
    return NULL;
}

int OSSL_PROVIDER_unload(OSSL_PROVIDER *prov)
{
    ossl_provider_free(prov);
    return 1;
}

int OSSL_PROVIDER_available(OPENSSL_CTX *libctx, const char *name)
{
    OSSL_PROVIDER *prov = NULL;
    int available = 0;

    /* Find it or create it */
    prov = ossl_provider_find(libctx, name, 0);
    available = ossl_provider_available(prov);
    ossl_provider_free(prov);
    return available;
}

const OSSL_PARAM *OSSL_PROVIDER_gettable_params(const OSSL_PROVIDER *prov)
{
    return ossl_provider_gettable_params(prov);
}

int OSSL_PROVIDER_get_params(const OSSL_PROVIDER *prov, OSSL_PARAM params[])
{
    return ossl_provider_get_params(prov, params);
}

const OSSL_ALGORITHM *OSSL_PROVIDER_query_operation(const OSSL_PROVIDER *prov,
                                                    int operation_id,
                                                    int *no_cache)
{
    return ossl_provider_query_operation(prov, operation_id, no_cache);
}

void *OSSL_PROVIDER_get0_provider_ctx(const OSSL_PROVIDER *prov)
{
    return ossl_provider_prov_ctx(prov);
}

int OSSL_PROVIDER_self_test(const OSSL_PROVIDER *prov)
{
    return ossl_provider_self_test(prov);
}

int OSSL_PROVIDER_get_capabilities(const OSSL_PROVIDER *prov,
                                   const char *capability,
                                   OSSL_CALLBACK *cb,
                                   void *arg)
{
    return ossl_provider_get_capabilities(prov, capability, cb, arg);
}

int OSSL_PROVIDER_add_builtin(OPENSSL_CTX *libctx, const char *name,
                              OSSL_provider_init_fn *init_fn)
{
    OSSL_PROVIDER *prov = NULL;

    if (name == NULL || init_fn == NULL) {
        CRYPTOerr(CRYPTO_F_OSSL_PROVIDER_ADD_BUILTIN,
                  ERR_R_PASSED_NULL_PARAMETER);
        return 0;
    }

    /* Create it */
    if ((prov = ossl_provider_new(libctx, name, init_fn, 0)) == NULL)
        return 0;

    /*
     * It's safely stored in the internal store at this point,
     * free the returned extra reference
     */
    ossl_provider_free(prov);

    return 1;
}

const char *OSSL_PROVIDER_name(const OSSL_PROVIDER *prov)
{
    return ossl_provider_name(prov);
}

int OSSL_PROVIDER_do_all(OPENSSL_CTX *ctx,
                         int (*cb)(OSSL_PROVIDER *provider,
                                   void *cbdata),
                         void *cbdata)
{
    return ossl_provider_forall_loaded(ctx, cb, cbdata);
}
