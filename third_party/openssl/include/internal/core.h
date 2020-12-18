/*
 * Copyright 2019-2020 The OpenSSL Project Authors. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://www.openssl.org/source/license.html
 */

#ifndef OSSL_INTERNAL_CORE_H
# define OSSL_INTERNAL_CORE_H

/*
 * namespaces:
 *
 * ossl_method_         Core Method API
 */

/*
 * construct an arbitrary method from a dispatch table found by looking
 * up a match for the < operation_id, name, property > combination.
 * constructor and destructor are the constructor and destructor for that
 * arbitrary object.
 *
 * These objects are normally cached, unless the provider says not to cache.
 * However, force_cache can be used to force caching whatever the provider
 * says (for example, because the application knows better).
 */
typedef struct ossl_method_construct_method_st {
    /* Create store */
    void *(*alloc_tmp_store)(OPENSSL_CTX *ctx);
    /* Remove a store */
    void (*dealloc_tmp_store)(void *store);
    /* Get an already existing method from a store */
    void *(*get)(OPENSSL_CTX *libctx, void *store, void *data);
    /* Store a method in a store */
    int (*put)(OPENSSL_CTX *libctx, void *store, void *method,
               const OSSL_PROVIDER *prov, int operation_id, const char *name,
               const char *propdef, void *data);
    /* Construct a new method */
    void *(*construct)(const OSSL_ALGORITHM *algodef, OSSL_PROVIDER *prov,
                       void *data);
    /* Destruct a method */
    void (*destruct)(void *method, void *data);
} OSSL_METHOD_CONSTRUCT_METHOD;

void *ossl_method_construct(OPENSSL_CTX *ctx, int operation_id,
                            int force_cache,
                            OSSL_METHOD_CONSTRUCT_METHOD *mcm, void *mcm_data);

void ossl_algorithm_do_all(OPENSSL_CTX *libctx, int operation_id,
                           OSSL_PROVIDER *provider,
                           int (*pre)(OSSL_PROVIDER *, int operation_id,
                                      void *data, int *result),
                           void (*fn)(OSSL_PROVIDER *provider,
                                      const OSSL_ALGORITHM *algo,
                                      int no_store, void *data),
                           int (*post)(OSSL_PROVIDER *, int operation_id,
                                       int no_store, void *data, int *result),
                           void *data);

#endif
