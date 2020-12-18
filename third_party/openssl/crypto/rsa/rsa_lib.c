/*
 * Copyright 1995-2020 The OpenSSL Project Authors. All Rights Reserved.
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

#include <stdio.h>
#include <openssl/crypto.h>
#include <openssl/core_names.h>
#include <openssl/engine.h>
#include <openssl/evp.h>
#include "internal/cryptlib.h"
#include "internal/refcount.h"
#include "openssl/param_build.h"
#include "crypto/bn.h"
#include "crypto/evp.h"
#include "crypto/rsa.h"
#include "crypto/security_bits.h"
#include "rsa_local.h"

static RSA *rsa_new_intern(ENGINE *engine, OPENSSL_CTX *libctx);

#ifndef FIPS_MODULE
RSA *RSA_new(void)
{
    return rsa_new_intern(NULL, NULL);
}

const RSA_METHOD *RSA_get_method(const RSA *rsa)
{
    return rsa->meth;
}

int RSA_set_method(RSA *rsa, const RSA_METHOD *meth)
{
    /*
     * NB: The caller is specifically setting a method, so it's not up to us
     * to deal with which ENGINE it comes from.
     */
    const RSA_METHOD *mtmp;
    mtmp = rsa->meth;
    if (mtmp->finish)
        mtmp->finish(rsa);
#ifndef OPENSSL_NO_ENGINE
    ENGINE_finish(rsa->engine);
    rsa->engine = NULL;
#endif
    rsa->meth = meth;
    if (meth->init)
        meth->init(rsa);
    return 1;
}

RSA *RSA_new_method(ENGINE *engine)
{
    return rsa_new_intern(engine, NULL);
}
#endif

RSA *rsa_new_with_ctx(OPENSSL_CTX *libctx)
{
    return rsa_new_intern(NULL, libctx);
}

static RSA *rsa_new_intern(ENGINE *engine, OPENSSL_CTX *libctx)
{
    RSA *ret = OPENSSL_zalloc(sizeof(*ret));

    if (ret == NULL) {
        RSAerr(0, ERR_R_MALLOC_FAILURE);
        return NULL;
    }

    ret->references = 1;
    ret->lock = CRYPTO_THREAD_lock_new();
    if (ret->lock == NULL) {
        RSAerr(0, ERR_R_MALLOC_FAILURE);
        OPENSSL_free(ret);
        return NULL;
    }

    ret->libctx = libctx;
    ret->meth = RSA_get_default_method();
#if !defined(OPENSSL_NO_ENGINE) && !defined(FIPS_MODULE)
    ret->flags = ret->meth->flags & ~RSA_FLAG_NON_FIPS_ALLOW;
    if (engine) {
        if (!ENGINE_init(engine)) {
            RSAerr(0, ERR_R_ENGINE_LIB);
            goto err;
        }
        ret->engine = engine;
    } else {
        ret->engine = ENGINE_get_default_RSA();
    }
    if (ret->engine) {
        ret->meth = ENGINE_get_RSA(ret->engine);
        if (ret->meth == NULL) {
            RSAerr(0, ERR_R_ENGINE_LIB);
            goto err;
        }
    }
#endif

    ret->flags = ret->meth->flags & ~RSA_FLAG_NON_FIPS_ALLOW;
#ifndef FIPS_MODULE
    if (!CRYPTO_new_ex_data(CRYPTO_EX_INDEX_RSA, ret, &ret->ex_data)) {
        goto err;
    }
#endif

    if ((ret->meth->init != NULL) && !ret->meth->init(ret)) {
        RSAerr(0, ERR_R_INIT_FAIL);
        goto err;
    }

    return ret;

 err:
    RSA_free(ret);
    return NULL;
}

void RSA_free(RSA *r)
{
    int i;

    if (r == NULL)
        return;

    CRYPTO_DOWN_REF(&r->references, &i, r->lock);
    REF_PRINT_COUNT("RSA", r);
    if (i > 0)
        return;
    REF_ASSERT_ISNT(i < 0);

    if (r->meth != NULL && r->meth->finish != NULL)
        r->meth->finish(r);
#if !defined(OPENSSL_NO_ENGINE) && !defined(FIPS_MODULE)
    ENGINE_finish(r->engine);
#endif

#ifndef FIPS_MODULE
    CRYPTO_free_ex_data(CRYPTO_EX_INDEX_RSA, r, &r->ex_data);
#endif

    CRYPTO_THREAD_lock_free(r->lock);

    BN_free(r->n);
    BN_free(r->e);
    BN_clear_free(r->d);
    BN_clear_free(r->p);
    BN_clear_free(r->q);
    BN_clear_free(r->dmp1);
    BN_clear_free(r->dmq1);
    BN_clear_free(r->iqmp);

#if defined(FIPS_MODULE) && !defined(OPENSSL_NO_ACVP_TESTS)
    rsa_acvp_test_free(r->acvp_test);
#endif

#ifndef FIPS_MODULE
    RSA_PSS_PARAMS_free(r->pss);
    sk_RSA_PRIME_INFO_pop_free(r->prime_infos, rsa_multip_info_free);
#endif
    BN_BLINDING_free(r->blinding);
    BN_BLINDING_free(r->mt_blinding);
    OPENSSL_free(r->bignum_data);
    OPENSSL_free(r);
}

int RSA_up_ref(RSA *r)
{
    int i;

    if (CRYPTO_UP_REF(&r->references, &i, r->lock) <= 0)
        return 0;

    REF_PRINT_COUNT("RSA", r);
    REF_ASSERT_ISNT(i < 2);
    return i > 1 ? 1 : 0;
}

OPENSSL_CTX *rsa_get0_libctx(RSA *r)
{
    return r->libctx;
}

#ifndef FIPS_MODULE
int RSA_set_ex_data(RSA *r, int idx, void *arg)
{
    return CRYPTO_set_ex_data(&r->ex_data, idx, arg);
}

void *RSA_get_ex_data(const RSA *r, int idx)
{
    return CRYPTO_get_ex_data(&r->ex_data, idx);
}
#endif

/*
 * Define a scaling constant for our fixed point arithmetic.
 * This value must be a power of two because the base two logarithm code
 * makes this assumption.  The exponent must also be a multiple of three so
 * that the scale factor has an exact cube root.  Finally, the scale factor
 * should not be so large that a multiplication of two scaled numbers
 * overflows a 64 bit unsigned integer.
 */
static const unsigned int scale = 1 << 18;
static const unsigned int cbrt_scale = 1 << (2 * 18 / 3);

/* Define some constants, none exceed 32 bits */
static const unsigned int log_2  = 0x02c5c8;    /* scale * log(2) */
static const unsigned int log_e  = 0x05c551;    /* scale * log2(M_E) */
static const unsigned int c1_923 = 0x07b126;    /* scale * 1.923 */
static const unsigned int c4_690 = 0x12c28f;    /* scale * 4.690 */

/*
 * Multiply two scaled integers together and rescale the result.
 */
static ossl_inline uint64_t mul2(uint64_t a, uint64_t b)
{
    return a * b / scale;
}

/*
 * Calculate the cube root of a 64 bit scaled integer.
 * Although the cube root of a 64 bit number does fit into a 32 bit unsigned
 * integer, this is not guaranteed after scaling, so this function has a
 * 64 bit return.  This uses the shifting nth root algorithm with some
 * algebraic simplifications.
 */
static uint64_t icbrt64(uint64_t x)
{
    uint64_t r = 0;
    uint64_t b;
    int s;

    for (s = 63; s >= 0; s -= 3) {
        r <<= 1;
        b = 3 * r * (r + 1) + 1;
        if ((x >> s) >= b) {
            x -= b << s;
            r++;
        }
    }
    return r * cbrt_scale;
}

/*
 * Calculate the natural logarithm of a 64 bit scaled integer.
 * This is done by calculating a base two logarithm and scaling.
 * The maximum logarithm (base 2) is 64 and this reduces base e, so
 * a 32 bit result should not overflow.  The argument passed must be
 * greater than unity so we don't need to handle negative results.
 */
static uint32_t ilog_e(uint64_t v)
{
    uint32_t i, r = 0;

    /*
     * Scale down the value into the range 1 .. 2.
     *
     * If fractional numbers need to be processed, another loop needs
     * to go here that checks v < scale and if so multiplies it by 2 and
     * reduces r by scale.  This also means making r signed.
     */
    while (v >= 2 * scale) {
        v >>= 1;
        r += scale;
    }
    for (i = scale / 2; i != 0; i /= 2) {
        v = mul2(v, v);
        if (v >= 2 * scale) {
            v >>= 1;
            r += i;
        }
    }
    r = (r * (uint64_t)scale) / log_e;
    return r;
}

/*
 * NIST SP 800-56B rev 2 Appendix D: Maximum Security Strength Estimates for IFC
 * Modulus Lengths.
 *
 * Note that this formula is also referred to in SP800-56A rev3 Appendix D:
 * for FFC safe prime groups for modp and ffdhe.
 * After Table 25 and Table 26 it refers to
 * "The maximum security strength estimates were calculated using the formula in
 * Section 7.5 of the FIPS 140 IG and rounded to the nearest multiple of eight
 * bits".
 *
 * The formula is:
 *
 * E = \frac{1.923 \sqrt[3]{nBits \cdot log_e(2)}
 *           \cdot(log_e(nBits \cdot log_e(2))^{2/3} - 4.69}{log_e(2)}
 * The two cube roots are merged together here.
 */
uint16_t ifc_ffc_compute_security_bits(int n)
{
    uint64_t x;
    uint32_t lx;
    uint16_t y;

    /* Look for common values as listed in SP 800-56B rev 2 Appendix D */
    switch (n) {
    case 2048:
        return 112;
    case 3072:
        return 128;
    case 4096:
        return 152;
    case 6144:
        return 176;
    case 8192:
        return 200;
    }
    /*
     * The first incorrect result (i.e. not accurate or off by one low) occurs
     * for n = 699668.  The true value here is 1200.  Instead of using this n
     * as the check threshold, the smallest n such that the correct result is
     * 1200 is used instead.
     */
    if (n >= 687737)
        return 1200;
    if (n < 8)
        return 0;

    x = n * (uint64_t)log_2;
    lx = ilog_e(x);
    y = (uint16_t)((mul2(c1_923, icbrt64(mul2(mul2(x, lx), lx))) - c4_690)
                   / log_2);
    return (y + 4) & ~7;
}



int RSA_security_bits(const RSA *rsa)
{
    int bits = BN_num_bits(rsa->n);

#ifndef FIPS_MODULE
    if (rsa->version == RSA_ASN1_VERSION_MULTI) {
        /* This ought to mean that we have private key at hand. */
        int ex_primes = sk_RSA_PRIME_INFO_num(rsa->prime_infos);

        if (ex_primes <= 0 || (ex_primes + 2) > rsa_multip_cap(bits))
            return 0;
    }
#endif
    return ifc_ffc_compute_security_bits(bits);
}

int RSA_set0_key(RSA *r, BIGNUM *n, BIGNUM *e, BIGNUM *d)
{
    /* If the fields n and e in r are NULL, the corresponding input
     * parameters MUST be non-NULL for n and e.  d may be
     * left NULL (in case only the public key is used).
     */
    if ((r->n == NULL && n == NULL)
        || (r->e == NULL && e == NULL))
        return 0;

    if (n != NULL) {
        BN_free(r->n);
        r->n = n;
    }
    if (e != NULL) {
        BN_free(r->e);
        r->e = e;
    }
    if (d != NULL) {
        BN_clear_free(r->d);
        r->d = d;
        BN_set_flags(r->d, BN_FLG_CONSTTIME);
    }
    r->dirty_cnt++;

    return 1;
}

int RSA_set0_factors(RSA *r, BIGNUM *p, BIGNUM *q)
{
    /* If the fields p and q in r are NULL, the corresponding input
     * parameters MUST be non-NULL.
     */
    if ((r->p == NULL && p == NULL)
        || (r->q == NULL && q == NULL))
        return 0;

    if (p != NULL) {
        BN_clear_free(r->p);
        r->p = p;
        BN_set_flags(r->p, BN_FLG_CONSTTIME);
    }
    if (q != NULL) {
        BN_clear_free(r->q);
        r->q = q;
        BN_set_flags(r->q, BN_FLG_CONSTTIME);
    }
    r->dirty_cnt++;

    return 1;
}

int RSA_set0_crt_params(RSA *r, BIGNUM *dmp1, BIGNUM *dmq1, BIGNUM *iqmp)
{
    /* If the fields dmp1, dmq1 and iqmp in r are NULL, the corresponding input
     * parameters MUST be non-NULL.
     */
    if ((r->dmp1 == NULL && dmp1 == NULL)
        || (r->dmq1 == NULL && dmq1 == NULL)
        || (r->iqmp == NULL && iqmp == NULL))
        return 0;

    if (dmp1 != NULL) {
        BN_clear_free(r->dmp1);
        r->dmp1 = dmp1;
        BN_set_flags(r->dmp1, BN_FLG_CONSTTIME);
    }
    if (dmq1 != NULL) {
        BN_clear_free(r->dmq1);
        r->dmq1 = dmq1;
        BN_set_flags(r->dmq1, BN_FLG_CONSTTIME);
    }
    if (iqmp != NULL) {
        BN_clear_free(r->iqmp);
        r->iqmp = iqmp;
        BN_set_flags(r->iqmp, BN_FLG_CONSTTIME);
    }
    r->dirty_cnt++;

    return 1;
}

#ifndef FIPS_MODULE
/*
 * Is it better to export RSA_PRIME_INFO structure
 * and related functions to let user pass a triplet?
 */
int RSA_set0_multi_prime_params(RSA *r, BIGNUM *primes[], BIGNUM *exps[],
                                BIGNUM *coeffs[], int pnum)
{
    STACK_OF(RSA_PRIME_INFO) *prime_infos, *old = NULL;
    RSA_PRIME_INFO *pinfo;
    int i;

    if (primes == NULL || exps == NULL || coeffs == NULL || pnum == 0)
        return 0;

    prime_infos = sk_RSA_PRIME_INFO_new_reserve(NULL, pnum);
    if (prime_infos == NULL)
        return 0;

    if (r->prime_infos != NULL)
        old = r->prime_infos;

    for (i = 0; i < pnum; i++) {
        pinfo = rsa_multip_info_new();
        if (pinfo == NULL)
            goto err;
        if (primes[i] != NULL && exps[i] != NULL && coeffs[i] != NULL) {
            BN_clear_free(pinfo->r);
            BN_clear_free(pinfo->d);
            BN_clear_free(pinfo->t);
            pinfo->r = primes[i];
            pinfo->d = exps[i];
            pinfo->t = coeffs[i];
            BN_set_flags(pinfo->r, BN_FLG_CONSTTIME);
            BN_set_flags(pinfo->d, BN_FLG_CONSTTIME);
            BN_set_flags(pinfo->t, BN_FLG_CONSTTIME);
        } else {
            rsa_multip_info_free(pinfo);
            goto err;
        }
        (void)sk_RSA_PRIME_INFO_push(prime_infos, pinfo);
    }

    r->prime_infos = prime_infos;

    if (!rsa_multip_calc_product(r)) {
        r->prime_infos = old;
        goto err;
    }

    if (old != NULL) {
        /*
         * This is hard to deal with, since the old infos could
         * also be set by this function and r, d, t should not
         * be freed in that case. So currently, stay consistent
         * with other *set0* functions: just free it...
         */
        sk_RSA_PRIME_INFO_pop_free(old, rsa_multip_info_free);
    }

    r->version = RSA_ASN1_VERSION_MULTI;
    r->dirty_cnt++;

    return 1;
 err:
    /* r, d, t should not be freed */
    sk_RSA_PRIME_INFO_pop_free(prime_infos, rsa_multip_info_free_ex);
    return 0;
}
#endif

void RSA_get0_key(const RSA *r,
                  const BIGNUM **n, const BIGNUM **e, const BIGNUM **d)
{
    if (n != NULL)
        *n = r->n;
    if (e != NULL)
        *e = r->e;
    if (d != NULL)
        *d = r->d;
}

void RSA_get0_factors(const RSA *r, const BIGNUM **p, const BIGNUM **q)
{
    if (p != NULL)
        *p = r->p;
    if (q != NULL)
        *q = r->q;
}

#ifndef FIPS_MODULE
int RSA_get_multi_prime_extra_count(const RSA *r)
{
    int pnum;

    pnum = sk_RSA_PRIME_INFO_num(r->prime_infos);
    if (pnum <= 0)
        pnum = 0;
    return pnum;
}

int RSA_get0_multi_prime_factors(const RSA *r, const BIGNUM *primes[])
{
    int pnum, i;
    RSA_PRIME_INFO *pinfo;

    if ((pnum = RSA_get_multi_prime_extra_count(r)) == 0)
        return 0;

    /*
     * return other primes
     * it's caller's responsibility to allocate oth_primes[pnum]
     */
    for (i = 0; i < pnum; i++) {
        pinfo = sk_RSA_PRIME_INFO_value(r->prime_infos, i);
        primes[i] = pinfo->r;
    }

    return 1;
}
#endif

void RSA_get0_crt_params(const RSA *r,
                         const BIGNUM **dmp1, const BIGNUM **dmq1,
                         const BIGNUM **iqmp)
{
    if (dmp1 != NULL)
        *dmp1 = r->dmp1;
    if (dmq1 != NULL)
        *dmq1 = r->dmq1;
    if (iqmp != NULL)
        *iqmp = r->iqmp;
}

#ifndef FIPS_MODULE
int RSA_get0_multi_prime_crt_params(const RSA *r, const BIGNUM *exps[],
                                    const BIGNUM *coeffs[])
{
    int pnum;

    if ((pnum = RSA_get_multi_prime_extra_count(r)) == 0)
        return 0;

    /* return other primes */
    if (exps != NULL || coeffs != NULL) {
        RSA_PRIME_INFO *pinfo;
        int i;

        /* it's the user's job to guarantee the buffer length */
        for (i = 0; i < pnum; i++) {
            pinfo = sk_RSA_PRIME_INFO_value(r->prime_infos, i);
            if (exps != NULL)
                exps[i] = pinfo->d;
            if (coeffs != NULL)
                coeffs[i] = pinfo->t;
        }
    }

    return 1;
}
#endif

const BIGNUM *RSA_get0_n(const RSA *r)
{
    return r->n;
}

const BIGNUM *RSA_get0_e(const RSA *r)
{
    return r->e;
}

const BIGNUM *RSA_get0_d(const RSA *r)
{
    return r->d;
}

const BIGNUM *RSA_get0_p(const RSA *r)
{
    return r->p;
}

const BIGNUM *RSA_get0_q(const RSA *r)
{
    return r->q;
}

const BIGNUM *RSA_get0_dmp1(const RSA *r)
{
    return r->dmp1;
}

const BIGNUM *RSA_get0_dmq1(const RSA *r)
{
    return r->dmq1;
}

const BIGNUM *RSA_get0_iqmp(const RSA *r)
{
    return r->iqmp;
}

const RSA_PSS_PARAMS *RSA_get0_pss_params(const RSA *r)
{
#ifdef FIPS_MODULE
    return NULL;
#else
    return r->pss;
#endif
}

/* Internal */
RSA_PSS_PARAMS_30 *rsa_get0_pss_params_30(RSA *r)
{
    return &r->pss_params;
}

void RSA_clear_flags(RSA *r, int flags)
{
    r->flags &= ~flags;
}

int RSA_test_flags(const RSA *r, int flags)
{
    return r->flags & flags;
}

void RSA_set_flags(RSA *r, int flags)
{
    r->flags |= flags;
}

int RSA_get_version(RSA *r)
{
    /* { two-prime(0), multi(1) } */
    return r->version;
}

#ifndef FIPS_MODULE
ENGINE *RSA_get0_engine(const RSA *r)
{
    return r->engine;
}

int RSA_pkey_ctx_ctrl(EVP_PKEY_CTX *ctx, int optype, int cmd, int p1, void *p2)
{
    /* If key type not RSA or RSA-PSS return error */
    if (ctx != NULL && ctx->pmeth != NULL
        && ctx->pmeth->pkey_id != EVP_PKEY_RSA
        && ctx->pmeth->pkey_id != EVP_PKEY_RSA_PSS)
        return -1;
     return EVP_PKEY_CTX_ctrl(ctx, -1, optype, cmd, p1, p2);
}
#endif

DEFINE_STACK_OF(BIGNUM)

int rsa_set0_all_params(RSA *r, const STACK_OF(BIGNUM) *primes,
                        const STACK_OF(BIGNUM) *exps,
                        const STACK_OF(BIGNUM) *coeffs)
{
#ifndef FIPS_MODULE
    STACK_OF(RSA_PRIME_INFO) *prime_infos, *old_infos = NULL;
#endif
    int pnum;

    if (primes == NULL || exps == NULL || coeffs == NULL)
        return 0;

    pnum = sk_BIGNUM_num(primes);
    if (pnum < 2
        || pnum != sk_BIGNUM_num(exps)
        || pnum != sk_BIGNUM_num(coeffs) + 1)
        return 0;

    if (!RSA_set0_factors(r, sk_BIGNUM_value(primes, 0),
                          sk_BIGNUM_value(primes, 1))
        || !RSA_set0_crt_params(r, sk_BIGNUM_value(exps, 0),
                                sk_BIGNUM_value(exps, 1),
                                sk_BIGNUM_value(coeffs, 0)))
        return 0;

#ifndef FIPS_MODULE
    old_infos = r->prime_infos;
#endif

    if (pnum > 2) {
#ifndef FIPS_MODULE
        int i;

        prime_infos = sk_RSA_PRIME_INFO_new_reserve(NULL, pnum);
        if (prime_infos == NULL)
            return 0;

        for (i = 2; i < pnum; i++) {
            BIGNUM *prime = sk_BIGNUM_value(primes, i);
            BIGNUM *exp = sk_BIGNUM_value(exps, i);
            BIGNUM *coeff = sk_BIGNUM_value(coeffs, i - 1);
            RSA_PRIME_INFO *pinfo = NULL;

            if (!ossl_assert(prime != NULL && exp != NULL && coeff != NULL))
                goto err;

            /* Using rsa_multip_info_new() is wasteful, so allocate directly */
            if ((pinfo = OPENSSL_zalloc(sizeof(*pinfo))) == NULL) {
                ERR_raise(ERR_LIB_RSA, ERR_R_MALLOC_FAILURE);
                goto err;
            }

            pinfo->r = prime;
            pinfo->d = exp;
            pinfo->t = coeff;
            BN_set_flags(pinfo->r, BN_FLG_CONSTTIME);
            BN_set_flags(pinfo->d, BN_FLG_CONSTTIME);
            BN_set_flags(pinfo->t, BN_FLG_CONSTTIME);
            (void)sk_RSA_PRIME_INFO_push(prime_infos, pinfo);
        }

        r->prime_infos = prime_infos;

        if (!rsa_multip_calc_product(r)) {
            r->prime_infos = old_infos;
            goto err;
        }
#else
        return 0;
#endif
    }

#ifndef FIPS_MODULE
    if (old_infos != NULL) {
        /*
         * This is hard to deal with, since the old infos could
         * also be set by this function and r, d, t should not
         * be freed in that case. So currently, stay consistent
         * with other *set0* functions: just free it...
         */
        sk_RSA_PRIME_INFO_pop_free(old_infos, rsa_multip_info_free);
    }
#endif

    r->version = pnum > 2 ? RSA_ASN1_VERSION_MULTI : RSA_ASN1_VERSION_DEFAULT;
    r->dirty_cnt++;

    return 1;
#ifndef FIPS_MODULE
 err:
    /* r, d, t should not be freed */
    sk_RSA_PRIME_INFO_pop_free(prime_infos, rsa_multip_info_free_ex);
    return 0;
#endif
}

DEFINE_SPECIAL_STACK_OF_CONST(BIGNUM_const, BIGNUM)

int rsa_get0_all_params(RSA *r, STACK_OF(BIGNUM_const) *primes,
                        STACK_OF(BIGNUM_const) *exps,
                        STACK_OF(BIGNUM_const) *coeffs)
{
#ifndef FIPS_MODULE
    RSA_PRIME_INFO *pinfo;
    int i, pnum;
#endif

    if (r == NULL)
        return 0;

    /* If |p| is NULL, there are no CRT parameters */
    if (RSA_get0_p(r) == NULL)
        return 1;

    sk_BIGNUM_const_push(primes, RSA_get0_p(r));
    sk_BIGNUM_const_push(primes, RSA_get0_q(r));
    sk_BIGNUM_const_push(exps, RSA_get0_dmp1(r));
    sk_BIGNUM_const_push(exps, RSA_get0_dmq1(r));
    sk_BIGNUM_const_push(coeffs, RSA_get0_iqmp(r));

#ifndef FIPS_MODULE
    pnum = RSA_get_multi_prime_extra_count(r);
    for (i = 0; i < pnum; i++) {
        pinfo = sk_RSA_PRIME_INFO_value(r->prime_infos, i);
        sk_BIGNUM_const_push(primes, pinfo->r);
        sk_BIGNUM_const_push(exps, pinfo->d);
        sk_BIGNUM_const_push(coeffs, pinfo->t);
    }
#endif

    return 1;
}

#ifndef FIPS_MODULE
int EVP_PKEY_CTX_set_rsa_padding(EVP_PKEY_CTX *ctx, int pad_mode)
{
    OSSL_PARAM pad_params[2], *p = pad_params;

    if (ctx == NULL) {
        ERR_raise(ERR_LIB_EVP, EVP_R_COMMAND_NOT_SUPPORTED);
        /* Uses the same return values as EVP_PKEY_CTX_ctrl */
        return -2;
    }

    /* If key type not RSA or RSA-PSS return error */
    if (ctx->pmeth != NULL
            && ctx->pmeth->pkey_id != EVP_PKEY_RSA
            && ctx->pmeth->pkey_id != EVP_PKEY_RSA_PSS)
        return -1;

    /* TODO(3.0): Remove this eventually when no more legacy */
    if ((!EVP_PKEY_CTX_IS_ASYM_CIPHER_OP(ctx)
         || ctx->op.ciph.ciphprovctx == NULL)
        && (!EVP_PKEY_CTX_IS_SIGNATURE_OP(ctx)
            || ctx->op.sig.sigprovctx == NULL))
        return EVP_PKEY_CTX_ctrl(ctx, -1, -1, EVP_PKEY_CTRL_RSA_PADDING,
                                 pad_mode, NULL);

    *p++ = OSSL_PARAM_construct_int(OSSL_PKEY_PARAM_PAD_MODE, &pad_mode);
    *p++ = OSSL_PARAM_construct_end();

    return EVP_PKEY_CTX_set_params(ctx, pad_params);
}

int EVP_PKEY_CTX_get_rsa_padding(EVP_PKEY_CTX *ctx, int *pad_mode)
{
    OSSL_PARAM pad_params[2], *p = pad_params;

    if (ctx == NULL || pad_mode == NULL) {
        ERR_raise(ERR_LIB_EVP, EVP_R_COMMAND_NOT_SUPPORTED);
        /* Uses the same return values as EVP_PKEY_CTX_ctrl */
        return -2;
    }

    /* If key type not RSA or RSA-PSS return error */
    if (ctx->pmeth != NULL
            && ctx->pmeth->pkey_id != EVP_PKEY_RSA
            && ctx->pmeth->pkey_id != EVP_PKEY_RSA_PSS)
        return -1;

    /* TODO(3.0): Remove this eventually when no more legacy */
    if ((!EVP_PKEY_CTX_IS_ASYM_CIPHER_OP(ctx)
         || ctx->op.ciph.ciphprovctx == NULL)
        && (!EVP_PKEY_CTX_IS_SIGNATURE_OP(ctx)
            || ctx->op.sig.sigprovctx == NULL))
        return EVP_PKEY_CTX_ctrl(ctx, -1, -1, EVP_PKEY_CTRL_GET_RSA_PADDING, 0,
                                 pad_mode);

    *p++ = OSSL_PARAM_construct_int(OSSL_PKEY_PARAM_PAD_MODE, pad_mode);
    *p++ = OSSL_PARAM_construct_end();

    if (!EVP_PKEY_CTX_get_params(ctx, pad_params))
        return 0;

    return 1;

}

int EVP_PKEY_CTX_set_rsa_oaep_md(EVP_PKEY_CTX *ctx, const EVP_MD *md)
{
    const char *name;

    if (ctx == NULL || !EVP_PKEY_CTX_IS_ASYM_CIPHER_OP(ctx)) {
        ERR_raise(ERR_LIB_EVP, EVP_R_COMMAND_NOT_SUPPORTED);
        /* Uses the same return values as EVP_PKEY_CTX_ctrl */
        return -2;
    }

    /* If key type not RSA return error */
    if (ctx->pmeth != NULL && ctx->pmeth->pkey_id != EVP_PKEY_RSA)
        return -1;

    /* TODO(3.0): Remove this eventually when no more legacy */
    if (ctx->op.ciph.ciphprovctx == NULL)
        return EVP_PKEY_CTX_ctrl(ctx, EVP_PKEY_RSA, EVP_PKEY_OP_TYPE_CRYPT,
                                 EVP_PKEY_CTRL_RSA_OAEP_MD, 0, (void *)md);

    name = (md == NULL) ? "" : EVP_MD_name(md);

    return EVP_PKEY_CTX_set_rsa_oaep_md_name(ctx, name, NULL);
}

int EVP_PKEY_CTX_set_rsa_oaep_md_name(EVP_PKEY_CTX *ctx, const char *mdname,
                                      const char *mdprops)
{
    OSSL_PARAM rsa_params[3], *p = rsa_params;

    if (ctx == NULL || !EVP_PKEY_CTX_IS_ASYM_CIPHER_OP(ctx)) {
        ERR_raise(ERR_LIB_EVP, EVP_R_COMMAND_NOT_SUPPORTED);
        /* Uses the same return values as EVP_PKEY_CTX_ctrl */
        return -2;
    }

    /* If key type not RSA return error */
    if (ctx->pmeth != NULL && ctx->pmeth->pkey_id != EVP_PKEY_RSA)
        return -1;


    *p++ = OSSL_PARAM_construct_utf8_string(OSSL_ASYM_CIPHER_PARAM_OAEP_DIGEST,
                                            /*
                                             * Cast away the const. This is read
                                             * only so should be safe
                                             */
                                            (char *)mdname, 0);
    if (mdprops != NULL) {
        *p++ = OSSL_PARAM_construct_utf8_string(
                    OSSL_ASYM_CIPHER_PARAM_OAEP_DIGEST_PROPS,
                    /*
                     * Cast away the const. This is read
                     * only so should be safe
                     */
                    (char *)mdprops, 0);
    }
    *p++ = OSSL_PARAM_construct_end();

    return EVP_PKEY_CTX_set_params(ctx, rsa_params);
}

int EVP_PKEY_CTX_get_rsa_oaep_md_name(EVP_PKEY_CTX *ctx, char *name,
                                      size_t namelen)
{
    OSSL_PARAM rsa_params[2], *p = rsa_params;

    if (ctx == NULL || !EVP_PKEY_CTX_IS_ASYM_CIPHER_OP(ctx)) {
        ERR_raise(ERR_LIB_EVP, EVP_R_COMMAND_NOT_SUPPORTED);
        /* Uses the same return values as EVP_PKEY_CTX_ctrl */
        return -2;
    }

    /* If key type not RSA return error */
    if (ctx->pmeth != NULL && ctx->pmeth->pkey_id != EVP_PKEY_RSA)
        return -1;

    *p++ = OSSL_PARAM_construct_utf8_string(OSSL_ASYM_CIPHER_PARAM_OAEP_DIGEST,
                                            name, namelen);
    *p++ = OSSL_PARAM_construct_end();

    if (!EVP_PKEY_CTX_get_params(ctx, rsa_params))
        return -1;

    return 1;
}

int EVP_PKEY_CTX_get_rsa_oaep_md(EVP_PKEY_CTX *ctx, const EVP_MD **md)
{
    /* 80 should be big enough */
    char name[80] = "";

    if (ctx == NULL || md == NULL || !EVP_PKEY_CTX_IS_ASYM_CIPHER_OP(ctx)) {
        ERR_raise(ERR_LIB_EVP, EVP_R_COMMAND_NOT_SUPPORTED);
        /* Uses the same return values as EVP_PKEY_CTX_ctrl */
        return -2;
    }

    /* If key type not RSA return error */
    if (ctx->pmeth != NULL && ctx->pmeth->pkey_id != EVP_PKEY_RSA)
        return -1;

    /* TODO(3.0): Remove this eventually when no more legacy */
    if (ctx->op.ciph.ciphprovctx == NULL)
        return EVP_PKEY_CTX_ctrl(ctx, EVP_PKEY_RSA, EVP_PKEY_OP_TYPE_CRYPT,
                                 EVP_PKEY_CTRL_GET_RSA_OAEP_MD, 0, (void *)md);

    if (EVP_PKEY_CTX_get_rsa_oaep_md_name(ctx, name, sizeof(name)) <= 0)
        return -1;

    /* May be NULL meaning "unknown" */
    *md = evp_get_digestbyname_ex(ctx->libctx, name);

    return 1;
}

static int int_set_rsa_mgf1_md(EVP_PKEY_CTX *ctx,
                               /* For EVP_PKEY_CTX_ctrl() */
                               int keytype, int optype, int cmd,
                               const EVP_MD *md,
                               /* For EVP_PKEY_CTX_set_params() */
                               const char *mdname, const char *mdprops)
{
    OSSL_PARAM rsa_params[3], *p = rsa_params;

    if (ctx == NULL || (ctx->operation & optype) == 0) {
        ERR_raise(ERR_LIB_EVP, EVP_R_COMMAND_NOT_SUPPORTED);
        /* Uses the same return values as EVP_PKEY_CTX_ctrl */
        return -2;
    }

    /* If key type not RSA return error */
    if (ctx->pmeth != NULL
        && (keytype == -1
            ? (ctx->pmeth->pkey_id != EVP_PKEY_RSA
               && ctx->pmeth->pkey_id != EVP_PKEY_RSA_PSS)
            : ctx->pmeth->pkey_id != keytype))
        return -1;

    /* TODO(3.0): Remove this eventually when no more legacy */
    if (cmd != -1) {
        if ((EVP_PKEY_CTX_IS_ASYM_CIPHER_OP(ctx)
             && ctx->op.ciph.ciphprovctx == NULL)
            || (EVP_PKEY_CTX_IS_SIGNATURE_OP(ctx)
                && ctx->op.sig.sigprovctx == NULL)
            || (EVP_PKEY_CTX_IS_GEN_OP(ctx)
                && ctx->op.keymgmt.genctx == NULL))
            return EVP_PKEY_CTX_ctrl(ctx, keytype, optype, cmd, 0, (void *)md);

        mdname = (md == NULL) ? "" : EVP_MD_name(md);
    }


    *p++ = OSSL_PARAM_construct_utf8_string(OSSL_PKEY_PARAM_MGF1_DIGEST,
                                            /*
                                             * Cast away the const. This is
                                             * read only so should be safe
                                             */
                                            (char *)mdname, 0);
    if (mdprops != NULL) {
        *p++ =
            OSSL_PARAM_construct_utf8_string(OSSL_PKEY_PARAM_MGF1_PROPERTIES,
                                             /*
                                              * Cast away the const. This is
                                              * read only so should be safe
                                              */
                                             (char *)mdprops, 0);
    }
    *p++ = OSSL_PARAM_construct_end();

    return EVP_PKEY_CTX_set_params(ctx, rsa_params);
}

int EVP_PKEY_CTX_set_rsa_mgf1_md(EVP_PKEY_CTX *ctx, const EVP_MD *md)
{
    return int_set_rsa_mgf1_md(ctx, -1,
                               EVP_PKEY_OP_TYPE_CRYPT | EVP_PKEY_OP_TYPE_SIG,
                               EVP_PKEY_CTRL_RSA_MGF1_MD, md, NULL, NULL);
}

int EVP_PKEY_CTX_set_rsa_mgf1_md_name(EVP_PKEY_CTX *ctx, const char *mdname,
                                      const char *mdprops)
{
    return int_set_rsa_mgf1_md(ctx, -1,
                               EVP_PKEY_OP_TYPE_CRYPT | EVP_PKEY_OP_TYPE_SIG,
                               -1, NULL, mdname, mdprops);
}

int EVP_PKEY_CTX_set_rsa_pss_keygen_mgf1_md(EVP_PKEY_CTX *ctx, const EVP_MD *md)
{
    return int_set_rsa_mgf1_md(ctx, EVP_PKEY_RSA_PSS,
                               EVP_PKEY_OP_KEYGEN, EVP_PKEY_CTRL_RSA_MGF1_MD,
                               md, NULL, NULL);
}

int EVP_PKEY_CTX_set_rsa_pss_keygen_mgf1_md_name(EVP_PKEY_CTX *ctx,
                                                 const char *mdname)
{
    return int_set_rsa_mgf1_md(ctx, EVP_PKEY_RSA_PSS,
                               EVP_PKEY_OP_TYPE_CRYPT | EVP_PKEY_OP_TYPE_SIG,
                               -1, NULL, mdname, NULL);
}

int EVP_PKEY_CTX_get_rsa_mgf1_md_name(EVP_PKEY_CTX *ctx, char *name,
                                      size_t namelen)
{
    OSSL_PARAM rsa_params[2], *p = rsa_params;

    if (ctx == NULL
            || (!EVP_PKEY_CTX_IS_ASYM_CIPHER_OP(ctx)
                && !EVP_PKEY_CTX_IS_SIGNATURE_OP(ctx))) {
        ERR_raise(ERR_LIB_EVP, EVP_R_COMMAND_NOT_SUPPORTED);
        /* Uses the same return values as EVP_PKEY_CTX_ctrl */
        return -2;
    }

    /* If key type not RSA or RSA-PSS return error */
    if (ctx->pmeth != NULL
            && ctx->pmeth->pkey_id != EVP_PKEY_RSA
            && ctx->pmeth->pkey_id != EVP_PKEY_RSA_PSS)
        return -1;

    *p++ = OSSL_PARAM_construct_utf8_string(OSSL_PKEY_PARAM_MGF1_DIGEST,
                                            name, namelen);
    *p++ = OSSL_PARAM_construct_end();

    if (!EVP_PKEY_CTX_get_params(ctx, rsa_params))
        return -1;

    return 1;
}

int EVP_PKEY_CTX_get_rsa_mgf1_md(EVP_PKEY_CTX *ctx, const EVP_MD **md)
{
    /* 80 should be big enough */
    char name[80] = "";

    if (ctx == NULL
            || (!EVP_PKEY_CTX_IS_ASYM_CIPHER_OP(ctx)
                && !EVP_PKEY_CTX_IS_SIGNATURE_OP(ctx))) {
        ERR_raise(ERR_LIB_EVP, EVP_R_COMMAND_NOT_SUPPORTED);
        /* Uses the same return values as EVP_PKEY_CTX_ctrl */
        return -2;
    }

    /* If key type not RSA or RSA-PSS return error */
    if (ctx->pmeth != NULL
            && ctx->pmeth->pkey_id != EVP_PKEY_RSA
            && ctx->pmeth->pkey_id != EVP_PKEY_RSA_PSS)
        return -1;

    /* TODO(3.0): Remove this eventually when no more legacy */
    if ((EVP_PKEY_CTX_IS_ASYM_CIPHER_OP(ctx)
                && ctx->op.ciph.ciphprovctx == NULL)
            || (EVP_PKEY_CTX_IS_SIGNATURE_OP(ctx)
                && ctx->op.sig.sigprovctx == NULL))
        return EVP_PKEY_CTX_ctrl(ctx, -1,
                                 EVP_PKEY_OP_TYPE_SIG | EVP_PKEY_OP_TYPE_CRYPT,
                                 EVP_PKEY_CTRL_GET_RSA_MGF1_MD, 0, (void *)md);

    if (EVP_PKEY_CTX_get_rsa_mgf1_md_name(ctx, name, sizeof(name)) <= 0)
        return -1;

    /* May be NULL meaning "unknown" */
    *md = evp_get_digestbyname_ex(ctx->libctx, name);

    return 1;
}

int EVP_PKEY_CTX_set0_rsa_oaep_label(EVP_PKEY_CTX *ctx, void *label, int llen)
{
    OSSL_PARAM rsa_params[2], *p = rsa_params;

    if (ctx == NULL || !EVP_PKEY_CTX_IS_ASYM_CIPHER_OP(ctx)) {
        ERR_raise(ERR_LIB_EVP, EVP_R_COMMAND_NOT_SUPPORTED);
        /* Uses the same return values as EVP_PKEY_CTX_ctrl */
        return -2;
    }

    /* If key type not RSA return error */
    if (ctx->pmeth != NULL && ctx->pmeth->pkey_id != EVP_PKEY_RSA)
        return -1;

    /* TODO(3.0): Remove this eventually when no more legacy */
    if (ctx->op.ciph.ciphprovctx == NULL)
        return EVP_PKEY_CTX_ctrl(ctx, EVP_PKEY_RSA, EVP_PKEY_OP_TYPE_CRYPT,
                                 EVP_PKEY_CTRL_RSA_OAEP_LABEL, llen,
                                 (void *)label);

    *p++ = OSSL_PARAM_construct_octet_string(OSSL_ASYM_CIPHER_PARAM_OAEP_LABEL,
                                             /*
                                              * Cast away the const. This is
                                              * read only so should be safe
                                              */
                                             (void *)label,
                                             (size_t)llen);
    *p++ = OSSL_PARAM_construct_end();

    if (!EVP_PKEY_CTX_set_params(ctx, rsa_params))
        return 0;

    OPENSSL_free(label);
    return 1;
}

int EVP_PKEY_CTX_get0_rsa_oaep_label(EVP_PKEY_CTX *ctx, unsigned char **label)
{
    OSSL_PARAM rsa_params[3], *p = rsa_params;
    size_t labellen;

    if (ctx == NULL || !EVP_PKEY_CTX_IS_ASYM_CIPHER_OP(ctx)) {
        ERR_raise(ERR_LIB_EVP, EVP_R_COMMAND_NOT_SUPPORTED);
        /* Uses the same return values as EVP_PKEY_CTX_ctrl */
        return -2;
    }

    /* If key type not RSA return error */
    if (ctx->pmeth != NULL && ctx->pmeth->pkey_id != EVP_PKEY_RSA)
        return -1;

    /* TODO(3.0): Remove this eventually when no more legacy */
    if (ctx->op.ciph.ciphprovctx == NULL)
        return EVP_PKEY_CTX_ctrl(ctx, EVP_PKEY_RSA, EVP_PKEY_OP_TYPE_CRYPT,
                                 EVP_PKEY_CTRL_GET_RSA_OAEP_LABEL, 0,
                                 (void *)label);

    *p++ = OSSL_PARAM_construct_octet_ptr(OSSL_ASYM_CIPHER_PARAM_OAEP_LABEL,
                                          (void **)label, 0);
    *p++ = OSSL_PARAM_construct_size_t(OSSL_ASYM_CIPHER_PARAM_OAEP_LABEL_LEN,
                                       &labellen);
    *p++ = OSSL_PARAM_construct_end();

    if (!EVP_PKEY_CTX_get_params(ctx, rsa_params))
        return -1;

    if (labellen > INT_MAX)
        return -1;

    return (int)labellen;
}

static int int_set_rsa_pss_saltlen(EVP_PKEY_CTX *ctx, int saltlen,
                                   int keytype, int optype)
{
    OSSL_PARAM pad_params[2], *p = pad_params;

    if (ctx == NULL || (ctx->operation & optype) == 0) {
        ERR_raise(ERR_LIB_EVP, EVP_R_COMMAND_NOT_SUPPORTED);
        /* Uses the same return values as EVP_PKEY_CTX_ctrl */
        return -2;
    }

    /* If key type not RSA or RSA-PSS return error */
    if (ctx->pmeth != NULL
        && (keytype == -1
            ? (ctx->pmeth->pkey_id != EVP_PKEY_RSA
               && ctx->pmeth->pkey_id != EVP_PKEY_RSA_PSS)
            : ctx->pmeth->pkey_id != keytype))
        return -1;

    /* TODO(3.0): Remove this eventually when no more legacy */
    if ((EVP_PKEY_CTX_IS_SIGNATURE_OP(ctx)
         && ctx->op.sig.sigprovctx == NULL)
        || (EVP_PKEY_CTX_IS_GEN_OP(ctx)
            && ctx->op.keymgmt.genctx == NULL))
        return EVP_PKEY_CTX_ctrl(ctx, keytype, optype,
                                 EVP_PKEY_CTRL_RSA_PSS_SALTLEN,
                                 saltlen, NULL);

    *p++ =
        OSSL_PARAM_construct_int(OSSL_SIGNATURE_PARAM_PSS_SALTLEN, &saltlen);
    *p++ = OSSL_PARAM_construct_end();

    return EVP_PKEY_CTX_set_params(ctx, pad_params);
}

int EVP_PKEY_CTX_set_rsa_pss_saltlen(EVP_PKEY_CTX *ctx, int saltlen)
{
    return int_set_rsa_pss_saltlen(ctx, saltlen, -1, EVP_PKEY_OP_TYPE_SIG);
}

int EVP_PKEY_CTX_set_rsa_pss_keygen_saltlen(EVP_PKEY_CTX *ctx, int saltlen)
{
    return int_set_rsa_pss_saltlen(ctx, saltlen, EVP_PKEY_RSA_PSS,
                                   EVP_PKEY_OP_KEYGEN);
}

int EVP_PKEY_CTX_get_rsa_pss_saltlen(EVP_PKEY_CTX *ctx, int *saltlen)
{
    OSSL_PARAM pad_params[2], *p = pad_params;

    if (ctx == NULL || saltlen == NULL) {
        ERR_raise(ERR_LIB_EVP, EVP_R_COMMAND_NOT_SUPPORTED);
        /* Uses the same return values as EVP_PKEY_CTX_ctrl */
        return -2;
    }

    /* If key type not RSA or RSA-PSS return error */
    if (ctx->pmeth != NULL
            && ctx->pmeth->pkey_id != EVP_PKEY_RSA
            && ctx->pmeth->pkey_id != EVP_PKEY_RSA_PSS)
        return -1;

    /* TODO(3.0): Remove this eventually when no more legacy */
    if (!EVP_PKEY_CTX_IS_SIGNATURE_OP(ctx)
        || ctx->op.sig.sigprovctx == NULL)
        return EVP_PKEY_CTX_ctrl(ctx, -1, -1,
                                 EVP_PKEY_CTRL_GET_RSA_PSS_SALTLEN,
                                 0, saltlen);

    *p++ =
        OSSL_PARAM_construct_int(OSSL_SIGNATURE_PARAM_PSS_SALTLEN, saltlen);
    *p++ = OSSL_PARAM_construct_end();

    if (!EVP_PKEY_CTX_get_params(ctx, pad_params))
        return 0;

    return 1;

}

int EVP_PKEY_CTX_set_rsa_keygen_bits(EVP_PKEY_CTX *ctx, int bits)
{
    OSSL_PARAM params[2], *p = params;
    size_t bits2 = bits;

    if (ctx == NULL || !EVP_PKEY_CTX_IS_GEN_OP(ctx)) {
        ERR_raise(ERR_LIB_EVP, EVP_R_COMMAND_NOT_SUPPORTED);
        /* Uses the same return values as EVP_PKEY_CTX_ctrl */
        return -2;
    }

    /* If key type not RSA return error */
    if (ctx->pmeth != NULL && ctx->pmeth->pkey_id != EVP_PKEY_RSA)
        return -1;

    /* TODO(3.0): Remove this eventually when no more legacy */
    if (ctx->op.keymgmt.genctx == NULL)
        return EVP_PKEY_CTX_ctrl(ctx, EVP_PKEY_RSA, EVP_PKEY_OP_KEYGEN,
                                 EVP_PKEY_CTRL_RSA_KEYGEN_BITS, bits, NULL);

    *p++ = OSSL_PARAM_construct_size_t(OSSL_PKEY_PARAM_RSA_BITS, &bits2);
    *p++ = OSSL_PARAM_construct_end();

    if (!EVP_PKEY_CTX_set_params(ctx, params))
        return 0;

    return 1;
}

int EVP_PKEY_CTX_set_rsa_keygen_pubexp(EVP_PKEY_CTX *ctx, BIGNUM *pubexp)
{
    OSSL_PARAM_BLD *tmpl;
    OSSL_PARAM *params;
    int ret;

    if (ctx == NULL || !EVP_PKEY_CTX_IS_GEN_OP(ctx)) {
        ERR_raise(ERR_LIB_EVP, EVP_R_COMMAND_NOT_SUPPORTED);
        /* Uses the same return values as EVP_PKEY_CTX_ctrl */
        return -2;
    }

    /* If key type not RSA return error */
    if (ctx->pmeth != NULL && ctx->pmeth->pkey_id != EVP_PKEY_RSA)
        return -1;

    /* TODO(3.0): Remove this eventually when no more legacy */
    if (ctx->op.keymgmt.genctx == NULL)
        return EVP_PKEY_CTX_ctrl(ctx, EVP_PKEY_RSA, EVP_PKEY_OP_KEYGEN,
                                 EVP_PKEY_CTRL_RSA_KEYGEN_PUBEXP, 0, pubexp);

    if ((tmpl = OSSL_PARAM_BLD_new()) == NULL)
        return 0;
    if (!OSSL_PARAM_BLD_push_BN(tmpl, OSSL_PKEY_PARAM_RSA_E, pubexp)
        || (params = OSSL_PARAM_BLD_to_param(tmpl)) == NULL) {
        OSSL_PARAM_BLD_free(tmpl);
        return 0;
    }
    OSSL_PARAM_BLD_free(tmpl);

    ret = EVP_PKEY_CTX_set_params(ctx, params);
    OSSL_PARAM_BLD_free_params(params);
    return ret;
}

int EVP_PKEY_CTX_set_rsa_keygen_primes(EVP_PKEY_CTX *ctx, int primes)
{
    OSSL_PARAM params[2], *p = params;
    size_t primes2 = primes;

    if (ctx == NULL || !EVP_PKEY_CTX_IS_GEN_OP(ctx)) {
        ERR_raise(ERR_LIB_EVP, EVP_R_COMMAND_NOT_SUPPORTED);
        /* Uses the same return values as EVP_PKEY_CTX_ctrl */
        return -2;
    }

    /* If key type not RSA return error */
    if (ctx->pmeth != NULL && ctx->pmeth->pkey_id != EVP_PKEY_RSA)
        return -1;

    /* TODO(3.0): Remove this eventually when no more legacy */
    if (ctx->op.keymgmt.genctx == NULL)
        return EVP_PKEY_CTX_ctrl(ctx, EVP_PKEY_RSA, EVP_PKEY_OP_KEYGEN,
                                 EVP_PKEY_CTRL_RSA_KEYGEN_PRIMES, primes,
                                 NULL);

    *p++ = OSSL_PARAM_construct_size_t(OSSL_PKEY_PARAM_RSA_PRIMES, &primes2);
    *p++ = OSSL_PARAM_construct_end();

    if (!EVP_PKEY_CTX_set_params(ctx, params))
        return 0;

    return 1;
}
#endif
