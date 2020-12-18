/*
 * Copyright 1995-2020 The OpenSSL Project Authors. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://www.openssl.org/source/license.html
 */

/*
 * DSA low level APIs are deprecated for public use, but still ok for
 * internal use.
 */
#include "internal/deprecated.h"

#include <stdio.h>
#include "internal/cryptlib.h"
#include "internal/refcount.h"
#include <openssl/bn.h>
#include <openssl/asn1.h>
#include <openssl/engine.h>
#include <openssl/core_names.h>
#include "dsa_local.h"
#include "crypto/evp.h"
#include "crypto/dsa.h"
#include "crypto/dh.h" /* required by DSA_dup_DH() */

static DSA *dsa_new_intern(ENGINE *engine, OPENSSL_CTX *libctx);

#ifndef FIPS_MODULE

int DSA_set_ex_data(DSA *d, int idx, void *arg)
{
    return CRYPTO_set_ex_data(&d->ex_data, idx, arg);
}

void *DSA_get_ex_data(const DSA *d, int idx)
{
    return CRYPTO_get_ex_data(&d->ex_data, idx);
}

# ifndef OPENSSL_NO_DH
DH *DSA_dup_DH(const DSA *r)
{
    /*
     * DSA has p, q, g, optional pub_key, optional priv_key.
     * DH has p, optional length, g, optional pub_key,
     * optional priv_key, optional q.
     */
    DH *ret = NULL;
    BIGNUM *pub_key = NULL, *priv_key = NULL;

    if (r == NULL)
        goto err;
    ret = DH_new();
    if (ret == NULL)
        goto err;

    if (!ffc_params_copy(dh_get0_params(ret), &r->params))
        goto err;

    if (r->pub_key != NULL) {
        pub_key = BN_dup(r->pub_key);
        if (pub_key == NULL)
            goto err;
        if (r->priv_key != NULL) {
            priv_key = BN_dup(r->priv_key);
            if (priv_key == NULL)
                goto err;
        }
        if (!DH_set0_key(ret, pub_key, priv_key))
            goto err;
    } else if (r->priv_key != NULL) {
        /* Shouldn't happen */
        goto err;
    }

    return ret;

 err:
    BN_free(pub_key);
    BN_free(priv_key);
    DH_free(ret);
    return NULL;
}
# endif /*  OPENSSL_NO_DH */

void DSA_clear_flags(DSA *d, int flags)
{
    d->flags &= ~flags;
}

int DSA_test_flags(const DSA *d, int flags)
{
    return d->flags & flags;
}

void DSA_set_flags(DSA *d, int flags)
{
    d->flags |= flags;
}

ENGINE *DSA_get0_engine(DSA *d)
{
    return d->engine;
}

int DSA_set_method(DSA *dsa, const DSA_METHOD *meth)
{
    /*
     * NB: The caller is specifically setting a method, so it's not up to us
     * to deal with which ENGINE it comes from.
     */
    const DSA_METHOD *mtmp;
    mtmp = dsa->meth;
    if (mtmp->finish)
        mtmp->finish(dsa);
#ifndef OPENSSL_NO_ENGINE
    ENGINE_finish(dsa->engine);
    dsa->engine = NULL;
#endif
    dsa->meth = meth;
    if (meth->init)
        meth->init(dsa);
    return 1;
}
#endif /* FIPS_MODULE */


const DSA_METHOD *DSA_get_method(DSA *d)
{
    return d->meth;
}

static DSA *dsa_new_intern(ENGINE *engine, OPENSSL_CTX *libctx)
{
    DSA *ret = OPENSSL_zalloc(sizeof(*ret));

    if (ret == NULL) {
        DSAerr(0, ERR_R_MALLOC_FAILURE);
        return NULL;
    }

    ret->references = 1;
    ret->lock = CRYPTO_THREAD_lock_new();
    if (ret->lock == NULL) {
        DSAerr(0, ERR_R_MALLOC_FAILURE);
        OPENSSL_free(ret);
        return NULL;
    }

    ret->libctx = libctx;
    ret->meth = DSA_get_default_method();
#if !defined(FIPS_MODULE) && !defined(OPENSSL_NO_ENGINE)
    ret->flags = ret->meth->flags & ~DSA_FLAG_NON_FIPS_ALLOW; /* early default init */
    if (engine) {
        if (!ENGINE_init(engine)) {
            DSAerr(0, ERR_R_ENGINE_LIB);
            goto err;
        }
        ret->engine = engine;
    } else
        ret->engine = ENGINE_get_default_DSA();
    if (ret->engine) {
        ret->meth = ENGINE_get_DSA(ret->engine);
        if (ret->meth == NULL) {
            DSAerr(0, ERR_R_ENGINE_LIB);
            goto err;
        }
    }
#endif

    ret->flags = ret->meth->flags & ~DSA_FLAG_NON_FIPS_ALLOW;

#ifndef FIPS_MODULE
    if (!crypto_new_ex_data_ex(libctx, CRYPTO_EX_INDEX_DSA, ret, &ret->ex_data))
        goto err;
#endif

    if ((ret->meth->init != NULL) && !ret->meth->init(ret)) {
        DSAerr(0, ERR_R_INIT_FAIL);
        goto err;
    }

    return ret;

 err:
    DSA_free(ret);
    return NULL;
}

DSA *DSA_new_method(ENGINE *engine)
{
    return dsa_new_intern(engine, NULL);
}

DSA *dsa_new_with_ctx(OPENSSL_CTX *libctx)
{
    return dsa_new_intern(NULL, libctx);
}

#ifndef FIPS_MODULE
DSA *DSA_new(void)
{
    return dsa_new_intern(NULL, NULL);
}
#endif

void DSA_free(DSA *r)
{
    int i;

    if (r == NULL)
        return;

    CRYPTO_DOWN_REF(&r->references, &i, r->lock);
    REF_PRINT_COUNT("DSA", r);
    if (i > 0)
        return;
    REF_ASSERT_ISNT(i < 0);

    if (r->meth != NULL && r->meth->finish != NULL)
        r->meth->finish(r);
#if !defined(FIPS_MODULE) && !defined(OPENSSL_NO_ENGINE)
    ENGINE_finish(r->engine);
#endif

#ifndef FIPS_MODULE
    CRYPTO_free_ex_data(CRYPTO_EX_INDEX_DSA, r, &r->ex_data);
#endif

    CRYPTO_THREAD_lock_free(r->lock);

    ffc_params_cleanup(&r->params);
    BN_clear_free(r->pub_key);
    BN_clear_free(r->priv_key);
    OPENSSL_free(r);
}

int DSA_up_ref(DSA *r)
{
    int i;

    if (CRYPTO_UP_REF(&r->references, &i, r->lock) <= 0)
        return 0;

    REF_PRINT_COUNT("DSA", r);
    REF_ASSERT_ISNT(i < 2);
    return ((i > 1) ? 1 : 0);
}

void DSA_get0_pqg(const DSA *d,
                  const BIGNUM **p, const BIGNUM **q, const BIGNUM **g)
{
    ffc_params_get0_pqg(&d->params, p, q, g);
}

int DSA_set0_pqg(DSA *d, BIGNUM *p, BIGNUM *q, BIGNUM *g)
{
    /* If the fields p, q and g in d are NULL, the corresponding input
     * parameters MUST be non-NULL.
     */
    if ((d->params.p == NULL && p == NULL)
        || (d->params.q == NULL && q == NULL)
        || (d->params.g == NULL && g == NULL))
        return 0;

    ffc_params_set0_pqg(&d->params, p, q, g);
    d->dirty_cnt++;

    return 1;
}

const BIGNUM *DSA_get0_p(const DSA *d)
{
    return d->params.p;
}

const BIGNUM *DSA_get0_q(const DSA *d)
{
    return d->params.q;
}

const BIGNUM *DSA_get0_g(const DSA *d)
{
    return d->params.g;
}

const BIGNUM *DSA_get0_pub_key(const DSA *d)
{
    return d->pub_key;
}

const BIGNUM *DSA_get0_priv_key(const DSA *d)
{
    return d->priv_key;
}

void DSA_get0_key(const DSA *d,
                  const BIGNUM **pub_key, const BIGNUM **priv_key)
{
    if (pub_key != NULL)
        *pub_key = d->pub_key;
    if (priv_key != NULL)
        *priv_key = d->priv_key;
}

int DSA_set0_key(DSA *d, BIGNUM *pub_key, BIGNUM *priv_key)
{
    /* If the field pub_key in d is NULL, the corresponding input
     * parameters MUST be non-NULL.  The priv_key field may
     * be left NULL.
     */
    if (d->pub_key == NULL && pub_key == NULL)
        return 0;

    if (pub_key != NULL) {
        BN_free(d->pub_key);
        d->pub_key = pub_key;
    }
    if (priv_key != NULL) {
        BN_free(d->priv_key);
        d->priv_key = priv_key;
    }
    d->dirty_cnt++;

    return 1;
}

int DSA_security_bits(const DSA *d)
{
    if (d->params.p != NULL && d->params.q != NULL)
        return BN_security_bits(BN_num_bits(d->params.p),
                                BN_num_bits(d->params.q));
    return -1;
}

int DSA_bits(const DSA *dsa)
{
    return BN_num_bits(dsa->params.p);
}

FFC_PARAMS *dsa_get0_params(DSA *dsa)
{
    return &dsa->params;
}

int dsa_ffc_params_fromdata(DSA *dsa, const OSSL_PARAM params[])
{
    int ret;
    FFC_PARAMS *ffc;

    if (dsa == NULL)
        return 0;
    ffc = dsa_get0_params(dsa);
    if (ffc == NULL)
        return 0;

    ret = ffc_params_fromdata(ffc, params);
    if (ret)
        dsa->dirty_cnt++;
    return ret;
}

static int dsa_paramgen_check(EVP_PKEY_CTX *ctx)
{
    if (ctx == NULL || !EVP_PKEY_CTX_IS_GEN_OP(ctx)) {
        ERR_raise(ERR_LIB_EVP, EVP_R_COMMAND_NOT_SUPPORTED);
        /* Uses the same return values as EVP_PKEY_CTX_ctrl */
        return -2;
    }
    /* If key type not DSA return error */
    if (ctx->pmeth != NULL && ctx->pmeth->pkey_id != EVP_PKEY_DSA)
        return -1;
    return 1;
}

int EVP_PKEY_CTX_set_dsa_paramgen_type(EVP_PKEY_CTX *ctx, const char *name)
{
    int ret;
    OSSL_PARAM params[2], *p = params;

    if ((ret = dsa_paramgen_check(ctx)) <= 0)
        return ret;

    *p++ = OSSL_PARAM_construct_utf8_string(OSSL_PKEY_PARAM_FFC_TYPE,
                                            (char *)name, 0);
    *p++ = OSSL_PARAM_construct_end();

    return EVP_PKEY_CTX_set_params(ctx, params);
}

int EVP_PKEY_CTX_set_dsa_paramgen_gindex(EVP_PKEY_CTX *ctx, int gindex)
{
    int ret;
    OSSL_PARAM params[2], *p = params;

    if ((ret = dsa_paramgen_check(ctx)) <= 0)
        return ret;

    *p++ = OSSL_PARAM_construct_int(OSSL_PKEY_PARAM_FFC_GINDEX, &gindex);
    *p++ = OSSL_PARAM_construct_end();

    return EVP_PKEY_CTX_set_params(ctx, params);
}

int EVP_PKEY_CTX_set_dsa_paramgen_seed(EVP_PKEY_CTX *ctx,
                                       const unsigned char *seed,
                                       size_t seedlen)
{
    int ret;
    OSSL_PARAM params[2], *p = params;

    if ((ret = dsa_paramgen_check(ctx)) <= 0)
        return ret;

    *p++ = OSSL_PARAM_construct_octet_string(OSSL_PKEY_PARAM_FFC_SEED,
                                             (void *)seed, seedlen);
    *p++ = OSSL_PARAM_construct_end();

    return EVP_PKEY_CTX_set_params(ctx, params);
}

int EVP_PKEY_CTX_set_dsa_paramgen_bits(EVP_PKEY_CTX *ctx, int nbits)
{
    int ret;
    OSSL_PARAM params[2], *p = params;
    size_t bits = nbits;

    if ((ret = dsa_paramgen_check(ctx)) <= 0)
        return ret;

#if !defined(FIPS_MODULE)
    /* TODO(3.0): Remove this eventually when no more legacy */
    if (ctx->op.keymgmt.genctx == NULL)
        return EVP_PKEY_CTX_ctrl(ctx, EVP_PKEY_DSA,  EVP_PKEY_OP_PARAMGEN,
                                 EVP_PKEY_CTRL_DSA_PARAMGEN_BITS, nbits, NULL);
#endif

    *p++ = OSSL_PARAM_construct_size_t(OSSL_PKEY_PARAM_FFC_PBITS, &bits);
    *p++ = OSSL_PARAM_construct_end();

    return EVP_PKEY_CTX_set_params(ctx, params);
}

int EVP_PKEY_CTX_set_dsa_paramgen_q_bits(EVP_PKEY_CTX *ctx, int qbits)
{
    int ret;
    OSSL_PARAM params[2], *p = params;
    size_t bits2 = qbits;

    if ((ret = dsa_paramgen_check(ctx)) <= 0)
        return ret;

#if !defined(FIPS_MODULE)
    /* TODO(3.0): Remove this eventually when no more legacy */
    if (ctx->op.keymgmt.genctx == NULL)
        return EVP_PKEY_CTX_ctrl(ctx, EVP_PKEY_DSA,  EVP_PKEY_OP_PARAMGEN,
                                 EVP_PKEY_CTRL_DSA_PARAMGEN_Q_BITS, qbits, NULL);
#endif

    *p++ = OSSL_PARAM_construct_size_t(OSSL_PKEY_PARAM_FFC_QBITS, &bits2);
    *p++ = OSSL_PARAM_construct_end();

    return EVP_PKEY_CTX_set_params(ctx, params);
}

int EVP_PKEY_CTX_set_dsa_paramgen_md_props(EVP_PKEY_CTX *ctx,
                                           const char *md_name,
                                           const char *md_properties)
{
    int ret;
    OSSL_PARAM params[3], *p = params;

    if ((ret = dsa_paramgen_check(ctx)) <= 0)
        return ret;

#if !defined(FIPS_MODULE)
    /* TODO(3.0): Remove this eventually when no more legacy */
    if (ctx->op.keymgmt.genctx == NULL) {
        const EVP_MD *md = EVP_get_digestbyname(md_name);

         EVP_PKEY_CTX_ctrl(ctx, EVP_PKEY_DSA, EVP_PKEY_OP_PARAMGEN,
                           EVP_PKEY_CTRL_DSA_PARAMGEN_MD, 0, (void *)(md));
    }
#endif

    *p++ = OSSL_PARAM_construct_utf8_string(OSSL_PKEY_PARAM_FFC_DIGEST,
                                            (char *)md_name, 0);
    if (md_properties != NULL)
        *p++ = OSSL_PARAM_construct_utf8_string(OSSL_PKEY_PARAM_FFC_DIGEST_PROPS,
                                                (char *)md_properties, 0);
    *p++ = OSSL_PARAM_construct_end();

    return EVP_PKEY_CTX_set_params(ctx, params);
}

#if !defined(FIPS_MODULE)
int EVP_PKEY_CTX_set_dsa_paramgen_md(EVP_PKEY_CTX *ctx, const EVP_MD *md)
{
    const char *md_name = (md == NULL) ? "" : EVP_MD_name(md);

    return EVP_PKEY_CTX_set_dsa_paramgen_md_props(ctx, md_name, NULL);
}
#endif
