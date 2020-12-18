/*
 * Copyright 1995-2020 The OpenSSL Project Authors. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://www.openssl.org/source/license.html
 */

/* We need to use some engine deprecated APIs */
#define OPENSSL_SUPPRESS_DEPRECATED

#include <stdio.h>
#include <time.h>
#include "internal/cryptlib.h"
#include <openssl/opensslconf.h>
#include "crypto/rand.h"
#include "crypto/cryptlib.h"
#include <openssl/engine.h>
#include <openssl/core_names.h>
#include "internal/thread_once.h"
#include "rand_local.h"
#include "e_os.h"

#ifndef FIPS_MODULE
# include "prov/rand_pool.h"
# include "prov/seeding.h"

# ifndef OPENSSL_NO_ENGINE
/* non-NULL if default_RAND_meth is ENGINE-provided */
static ENGINE *funct_ref;
static CRYPTO_RWLOCK *rand_engine_lock;
# endif
static CRYPTO_RWLOCK *rand_meth_lock;
static const RAND_METHOD *default_RAND_meth;
static CRYPTO_ONCE rand_init = CRYPTO_ONCE_STATIC_INIT;

static int rand_inited = 0;

DEFINE_RUN_ONCE_STATIC(do_rand_init)
{
# ifndef OPENSSL_NO_ENGINE
    rand_engine_lock = CRYPTO_THREAD_lock_new();
    if (rand_engine_lock == NULL)
        return 0;
# endif

    rand_meth_lock = CRYPTO_THREAD_lock_new();
    if (rand_meth_lock == NULL)
        goto err;

    if (!rand_pool_init())
        goto err;

    rand_inited = 1;
    return 1;

 err:
    CRYPTO_THREAD_lock_free(rand_meth_lock);
    rand_meth_lock = NULL;
# ifndef OPENSSL_NO_ENGINE
    CRYPTO_THREAD_lock_free(rand_engine_lock);
    rand_engine_lock = NULL;
# endif
    return 0;
}

void rand_cleanup_int(void)
{
    const RAND_METHOD *meth = default_RAND_meth;

    if (!rand_inited)
        return;

    if (meth != NULL && meth->cleanup != NULL)
        meth->cleanup();
    RAND_set_rand_method(NULL);
    rand_pool_cleanup();
# ifndef OPENSSL_NO_ENGINE
    CRYPTO_THREAD_lock_free(rand_engine_lock);
    rand_engine_lock = NULL;
# endif
    CRYPTO_THREAD_lock_free(rand_meth_lock);
    rand_meth_lock = NULL;
    rand_inited = 0;
}

/*
 * RAND_close_seed_files() ensures that any seed file descriptors are
 * closed after use.  This only applies to libcrypto/default provider,
 * it does not apply to other providers.
 */
void RAND_keep_random_devices_open(int keep)
{
    if (RUN_ONCE(&rand_init, do_rand_init))
        rand_pool_keep_random_devices_open(keep);
}

/*
 * RAND_poll() reseeds the default RNG using random input
 *
 * The random input is obtained from polling various entropy
 * sources which depend on the operating system and are
 * configurable via the --with-rand-seed configure option.
 */
int RAND_poll(void)
{
    const RAND_METHOD *meth = RAND_get_rand_method();
    int ret = meth == RAND_OpenSSL();

    if (meth == NULL)
        return 0;

#ifndef OPENSSL_NO_DEPRECATED_3_0
    if (!ret) {
        /* fill random pool and seed the current legacy RNG */
        RAND_POOL *pool = rand_pool_new(RAND_DRBG_STRENGTH, 1,
                                        (RAND_DRBG_STRENGTH + 7) / 8,
                                        RAND_POOL_MAX_LENGTH);

        if (pool == NULL)
            return 0;

        if (prov_pool_acquire_entropy(pool) == 0)
            goto err;

        if (meth->add == NULL
            || meth->add(rand_pool_buffer(pool),
                         rand_pool_length(pool),
                         (rand_pool_entropy(pool) / 8.0)) == 0)
            goto err;

        ret = 1;
     err:
        rand_pool_free(pool);
    }
#endif
    return ret;
}

int RAND_set_rand_method(const RAND_METHOD *meth)
{
    if (!RUN_ONCE(&rand_init, do_rand_init))
        return 0;

    CRYPTO_THREAD_write_lock(rand_meth_lock);
# ifndef OPENSSL_NO_ENGINE
    ENGINE_finish(funct_ref);
    funct_ref = NULL;
# endif
    default_RAND_meth = meth;
    CRYPTO_THREAD_unlock(rand_meth_lock);
    return 1;
}

const RAND_METHOD *RAND_get_rand_method(void)
{
    const RAND_METHOD *tmp_meth = NULL;

    if (!RUN_ONCE(&rand_init, do_rand_init))
        return NULL;

    CRYPTO_THREAD_write_lock(rand_meth_lock);
    if (default_RAND_meth == NULL) {
# ifndef OPENSSL_NO_ENGINE
        ENGINE *e;

        /* If we have an engine that can do RAND, use it. */
        if ((e = ENGINE_get_default_RAND()) != NULL
                && (tmp_meth = ENGINE_get_RAND(e)) != NULL) {
            funct_ref = e;
            default_RAND_meth = tmp_meth;
        } else {
            ENGINE_finish(e);
            default_RAND_meth = &rand_meth;
        }
# else
        default_RAND_meth = &rand_meth;
# endif
    }
    tmp_meth = default_RAND_meth;
    CRYPTO_THREAD_unlock(rand_meth_lock);
    return tmp_meth;
}

# if !defined(OPENSSL_NO_ENGINE)
int RAND_set_rand_engine(ENGINE *engine)
{
    const RAND_METHOD *tmp_meth = NULL;

    if (!RUN_ONCE(&rand_init, do_rand_init))
        return 0;

    if (engine != NULL) {
        if (!ENGINE_init(engine))
            return 0;
        tmp_meth = ENGINE_get_RAND(engine);
        if (tmp_meth == NULL) {
            ENGINE_finish(engine);
            return 0;
        }
    }
    CRYPTO_THREAD_write_lock(rand_engine_lock);
    /* This function releases any prior ENGINE so call it first */
    RAND_set_rand_method(tmp_meth);
    funct_ref = engine;
    CRYPTO_THREAD_unlock(rand_engine_lock);
    return 1;
}
# endif

void RAND_seed(const void *buf, int num)
{
    const RAND_METHOD *meth = RAND_get_rand_method();

    if (meth != NULL && meth->seed != NULL)
        meth->seed(buf, num);
}

void RAND_add(const void *buf, int num, double randomness)
{
    const RAND_METHOD *meth = RAND_get_rand_method();

    if (meth != NULL && meth->add != NULL)
        meth->add(buf, num, randomness);
}

# if !defined(OPENSSL_NO_DEPRECATED_1_1_0)
int RAND_pseudo_bytes(unsigned char *buf, int num)
{
    const RAND_METHOD *meth = RAND_get_rand_method();

    if (meth != NULL && meth->pseudorand != NULL)
        return meth->pseudorand(buf, num);
    RANDerr(RAND_F_RAND_PSEUDO_BYTES, RAND_R_FUNC_NOT_IMPLEMENTED);
    return -1;
}
# endif

int RAND_status(void)
{
    EVP_RAND_CTX *rand;
    const RAND_METHOD *meth = RAND_get_rand_method();

    if (meth != NULL && meth != RAND_OpenSSL())
        return meth->status != NULL ? meth->status() : 0;

    if ((rand = RAND_get0_primary(NULL)) == NULL)
        return 0;
    return EVP_RAND_state(rand) == EVP_RAND_STATE_READY;
}
#else  /* !FIPS_MODULE */

const RAND_METHOD *RAND_get_rand_method(void)
{
    return NULL;
}
#endif /* !FIPS_MODULE */

/*
 * This function is not part of RAND_METHOD, so if we're not using
 * the default method, then just call RAND_bytes().  Otherwise make
 * sure we're instantiated and use the private DRBG.
 */
int RAND_priv_bytes_ex(OPENSSL_CTX *ctx, unsigned char *buf, int num)
{
    EVP_RAND_CTX *rand;
    const RAND_METHOD *meth = RAND_get_rand_method();

    if (meth != NULL && meth != RAND_OpenSSL()) {
        if (meth->bytes != NULL)
            return meth->bytes(buf, num);
        RANDerr(RAND_F_RAND_PRIV_BYTES_EX, RAND_R_FUNC_NOT_IMPLEMENTED);
        return -1;
    }

    rand = RAND_get0_private(ctx);
    if (rand != NULL)
        return EVP_RAND_generate(rand, buf, num, 0, 0, NULL, 0);

    return 0;
}

int RAND_priv_bytes(unsigned char *buf, int num)
{
    return RAND_priv_bytes_ex(NULL, buf, num);
}

int RAND_bytes_ex(OPENSSL_CTX *ctx, unsigned char *buf, int num)
{
    EVP_RAND_CTX *rand;
    const RAND_METHOD *meth = RAND_get_rand_method();

    if (meth != NULL && meth != RAND_OpenSSL()) {
        if (meth->bytes != NULL)
            return meth->bytes(buf, num);
        RANDerr(RAND_F_RAND_BYTES_EX, RAND_R_FUNC_NOT_IMPLEMENTED);
        return -1;
    }

    rand = RAND_get0_public(ctx);
    if (rand != NULL)
        return EVP_RAND_generate(rand, buf, num, 0, 0, NULL, 0);

    return 0;
}

int RAND_bytes(unsigned char *buf, int num)
{
    return RAND_bytes_ex(NULL, buf, num);
}

typedef struct rand_global_st {
    /*
     * The three shared DRBG instances
     *
     * There are three shared DRBG instances: <primary>, <public>, and
     * <private>.  The <public> and <private> DRBGs are secondary ones.
     * These are used for non-secret (e.g. nonces) and secret
     * (e.g. private keys) data respectively.
     */
    CRYPTO_RWLOCK *lock;

    /*
     * The <primary> DRBG
     *
     * Not used directly by the application, only for reseeding the two other
     * DRBGs. It reseeds itself by pulling either randomness from os entropy
     * sources or by consuming randomness which was added by RAND_add().
     *
     * The <primary> DRBG is a global instance which is accessed concurrently by
     * all threads. The necessary locking is managed automatically by its child
     * DRBG instances during reseeding.
     */
    EVP_RAND_CTX *primary;

    /*
     * The <public> DRBG
     *
     * Used by default for generating random bytes using RAND_bytes().
     *
     * The <public> secondary DRBG is thread-local, i.e., there is one instance
     * per thread.
     */
    CRYPTO_THREAD_LOCAL public;

    /*
     * The <private> DRBG
     *
     * Used by default for generating private keys using RAND_priv_bytes()
     *
     * The <private> secondary DRBG is thread-local, i.e., there is one
     * instance per thread.
     */
    CRYPTO_THREAD_LOCAL private;
} RAND_GLOBAL;

/*
 * Initialize the OPENSSL_CTX global DRBGs on first use.
 * Returns the allocated global data on success or NULL on failure.
 */
static void *rand_ossl_ctx_new(OPENSSL_CTX *libctx)
{
    RAND_GLOBAL *dgbl = OPENSSL_zalloc(sizeof(*dgbl));

    if (dgbl == NULL)
        return NULL;

#ifndef FIPS_MODULE
    /*
     * We need to ensure that base libcrypto thread handling has been
     * initialised.
     */
     OPENSSL_init_crypto(0, NULL);
#endif

    dgbl->lock = CRYPTO_THREAD_lock_new();
    if (dgbl->lock == NULL)
        goto err1;

    if (!CRYPTO_THREAD_init_local(&dgbl->private, NULL))
        goto err1;

    if (!CRYPTO_THREAD_init_local(&dgbl->public, NULL))
        goto err2;

    return dgbl;

 err2:
    CRYPTO_THREAD_cleanup_local(&dgbl->private);
 err1:
    CRYPTO_THREAD_lock_free(dgbl->lock);
    OPENSSL_free(dgbl);
    return NULL;
}

static void rand_ossl_ctx_free(void *vdgbl)
{
    RAND_GLOBAL *dgbl = vdgbl;

    if (dgbl == NULL)
        return;

    CRYPTO_THREAD_lock_free(dgbl->lock);
    EVP_RAND_CTX_free(dgbl->primary);
    CRYPTO_THREAD_cleanup_local(&dgbl->private);
    CRYPTO_THREAD_cleanup_local(&dgbl->public);

    OPENSSL_free(dgbl);
}

static const OPENSSL_CTX_METHOD rand_drbg_ossl_ctx_method = {
    rand_ossl_ctx_new,
    rand_ossl_ctx_free,
};

static RAND_GLOBAL *rand_get_global(OPENSSL_CTX *libctx)
{
    return openssl_ctx_get_data(libctx, OPENSSL_CTX_DRBG_INDEX,
                                &rand_drbg_ossl_ctx_method);
}

static void rand_delete_thread_state(void *arg)
{
    OPENSSL_CTX *ctx = arg;
    RAND_GLOBAL *dgbl = rand_get_global(ctx);
    EVP_RAND_CTX *rand;

    if (dgbl == NULL)
        return;

    rand = CRYPTO_THREAD_get_local(&dgbl->public);
    CRYPTO_THREAD_set_local(&dgbl->public, NULL);
    EVP_RAND_CTX_free(rand);

    rand = CRYPTO_THREAD_get_local(&dgbl->private);
    CRYPTO_THREAD_set_local(&dgbl->private, NULL);
    EVP_RAND_CTX_free(rand);
}

static EVP_RAND_CTX *rand_new_drbg(OPENSSL_CTX *libctx, EVP_RAND_CTX *parent,
                                   unsigned int reseed_interval,
                                   time_t reseed_time_interval)
{
    EVP_RAND *rand = EVP_RAND_fetch(libctx, "CTR-DRBG", NULL);
    EVP_RAND_CTX *ctx;
    OSSL_PARAM params[4], *p = params;
    
    if (rand == NULL) {
        RANDerr(0, RAND_R_UNABLE_TO_FETCH_DRBG);
        return NULL;
    }
    ctx = EVP_RAND_CTX_new(rand, parent);
    EVP_RAND_free(rand);
    if (ctx == NULL) {
        RANDerr(0, RAND_R_UNABLE_TO_CREATE_DRBG);
        return NULL;
    }

    *p++ = OSSL_PARAM_construct_utf8_string(OSSL_DRBG_PARAM_CIPHER,
                                            "AES-256-CTR", 0);
    *p++ = OSSL_PARAM_construct_uint(OSSL_DRBG_PARAM_RESEED_REQUESTS,
                                     &reseed_interval);
    *p++ = OSSL_PARAM_construct_time_t(OSSL_DRBG_PARAM_RESEED_TIME_INTERVAL,
                                       &reseed_time_interval);
    *p = OSSL_PARAM_construct_end();
    if (!EVP_RAND_set_ctx_params(ctx, params)) {
        RANDerr(0, RAND_R_ERROR_INITIALISING_DRBG);
        EVP_RAND_CTX_free(ctx);
        return NULL;
    }
    if (!EVP_RAND_instantiate(ctx, 0, 0, NULL, 0)) {
        RANDerr(0, RAND_R_ERROR_INSTANTIATING_DRBG);
        EVP_RAND_CTX_free(ctx);
        return NULL;
    }
    return ctx;
}

/*
 * Get the primary random generator.
 * Returns pointer to its EVP_RAND_CTX on success, NULL on failure.
 *
 */
EVP_RAND_CTX *RAND_get0_primary(OPENSSL_CTX *ctx)
{
    RAND_GLOBAL *dgbl = rand_get_global(ctx);

    if (dgbl == NULL)
        return NULL;

    if (dgbl->primary == NULL) {
        if (!CRYPTO_THREAD_write_lock(dgbl->lock))
            return NULL;
        if (dgbl->primary == NULL)
            dgbl->primary = rand_new_drbg(ctx, NULL, PRIMARY_RESEED_INTERVAL,
                                          PRIMARY_RESEED_TIME_INTERVAL);
        CRYPTO_THREAD_unlock(dgbl->lock);
    }
    return dgbl->primary;
}

/*
 * Get the public random generator.
 * Returns pointer to its EVP_RAND_CTX on success, NULL on failure.
 */
EVP_RAND_CTX *RAND_get0_public(OPENSSL_CTX *ctx)
{
    RAND_GLOBAL *dgbl = rand_get_global(ctx);
    EVP_RAND_CTX *rand, *primary;

    if (dgbl == NULL)
        return NULL;

    rand = CRYPTO_THREAD_get_local(&dgbl->public);
    if (rand == NULL) {
        primary = RAND_get0_primary(ctx);
        if (primary == NULL)
            return NULL;

        ctx = openssl_ctx_get_concrete(ctx);
        /*
         * If the private is also NULL then this is the first time we've
         * used this thread.
         */
        if (CRYPTO_THREAD_get_local(&dgbl->private) == NULL
                && !ossl_init_thread_start(NULL, ctx, rand_delete_thread_state))
            return NULL;
        rand = rand_new_drbg(ctx, primary, SECONDARY_RESEED_INTERVAL,
                             SECONDARY_RESEED_TIME_INTERVAL);
        CRYPTO_THREAD_set_local(&dgbl->public, rand);
    }
    return rand;
}

/*
 * Get the private random generator.
 * Returns pointer to its EVP_RAND_CTX on success, NULL on failure.
 */
EVP_RAND_CTX *RAND_get0_private(OPENSSL_CTX *ctx)
{
    RAND_GLOBAL *dgbl = rand_get_global(ctx);
    EVP_RAND_CTX *rand, *primary;

    if (dgbl == NULL)
        return NULL;

    rand = CRYPTO_THREAD_get_local(&dgbl->private);
    if (rand == NULL) {
        primary = RAND_get0_primary(ctx);
        if (primary == NULL)
            return NULL;

        ctx = openssl_ctx_get_concrete(ctx);
        /*
         * If the public is also NULL then this is the first time we've
         * used this thread.
         */
        if (CRYPTO_THREAD_get_local(&dgbl->public) == NULL
                && !ossl_init_thread_start(NULL, ctx, rand_delete_thread_state))
            return NULL;
        rand = rand_new_drbg(ctx, primary, SECONDARY_RESEED_INTERVAL,
                             SECONDARY_RESEED_TIME_INTERVAL);
        CRYPTO_THREAD_set_local(&dgbl->private, rand);
    }
    return rand;
}
