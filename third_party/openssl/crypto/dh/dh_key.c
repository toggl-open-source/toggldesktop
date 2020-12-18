/*
 * Copyright 1995-2020 The OpenSSL Project Authors. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://www.openssl.org/source/license.html
 */

/*
 * DH low level APIs are deprecated for public use, but still ok for
 * internal use.
 */
#include "internal/deprecated.h"

#include <stdio.h>
#include "internal/cryptlib.h"
#include "dh_local.h"
#include "crypto/bn.h"
#include "crypto/dh.h"
#include "crypto/security_bits.h"

#ifdef FIPS_MODULE
# define MIN_STRENGTH 112
#else
# define MIN_STRENGTH 80
#endif

static int generate_key(DH *dh);
static int dh_bn_mod_exp(const DH *dh, BIGNUM *r,
                         const BIGNUM *a, const BIGNUM *p,
                         const BIGNUM *m, BN_CTX *ctx, BN_MONT_CTX *m_ctx);
static int dh_init(DH *dh);
static int dh_finish(DH *dh);

static int compute_key(unsigned char *key, const BIGNUM *pub_key, DH *dh)
{
    BN_CTX *ctx = NULL;
    BN_MONT_CTX *mont = NULL;
    BIGNUM *tmp;
    int ret = -1;
#ifndef FIPS_MODULE
    int check_result;
#endif

    if (BN_num_bits(dh->params.p) > OPENSSL_DH_MAX_MODULUS_BITS) {
        DHerr(0, DH_R_MODULUS_TOO_LARGE);
        goto err;
    }

    if (BN_num_bits(dh->params.p) < DH_MIN_MODULUS_BITS) {
        DHerr(0, DH_R_MODULUS_TOO_SMALL);
        return 0;
    }

    ctx = BN_CTX_new_ex(dh->libctx);
    if (ctx == NULL)
        goto err;
    BN_CTX_start(ctx);
    tmp = BN_CTX_get(ctx);
    if (tmp == NULL)
        goto err;

    if (dh->priv_key == NULL) {
        DHerr(0, DH_R_NO_PRIVATE_VALUE);
        goto err;
    }

    if (dh->flags & DH_FLAG_CACHE_MONT_P) {
        mont = BN_MONT_CTX_set_locked(&dh->method_mont_p,
                                      dh->lock, dh->params.p, ctx);
        BN_set_flags(dh->priv_key, BN_FLG_CONSTTIME);
        if (!mont)
            goto err;
    }
/* TODO(3.0) : Solve in a PR related to Key validation for DH */
#ifndef FIPS_MODULE
    if (!DH_check_pub_key(dh, pub_key, &check_result) || check_result) {
        DHerr(0, DH_R_INVALID_PUBKEY);
        goto err;
    }
#endif
    if (!dh->meth->bn_mod_exp(dh, tmp, pub_key, dh->priv_key, dh->params.p, ctx,
                              mont)) {
        DHerr(0, ERR_R_BN_LIB);
        goto err;
    }

    ret = BN_bn2bin(tmp, key);
 err:
    BN_CTX_end(ctx);
    BN_CTX_free(ctx);
    return ret;
}

int DH_compute_key(unsigned char *key, const BIGNUM *pub_key, DH *dh)
{
#ifdef FIPS_MODULE
    return compute_key(key, pub_key, dh);
#else
    return dh->meth->compute_key(key, pub_key, dh);
#endif
}

int DH_compute_key_padded(unsigned char *key, const BIGNUM *pub_key, DH *dh)
{
    int rv, pad;

#ifdef FIPS_MODULE
    rv = compute_key(key, pub_key, dh);
#else
    rv = dh->meth->compute_key(key, pub_key, dh);
#endif
    if (rv <= 0)
        return rv;
    pad = BN_num_bytes(dh->params.p) - rv;
    if (pad > 0) {
        memmove(key + pad, key, rv);
        memset(key, 0, pad);
    }
    return rv + pad;
}

static DH_METHOD dh_ossl = {
    "OpenSSL DH Method",
    generate_key,
    compute_key,
    dh_bn_mod_exp,
    dh_init,
    dh_finish,
    DH_FLAG_FIPS_METHOD,
    NULL,
    NULL
};

static const DH_METHOD *default_DH_method = &dh_ossl;

const DH_METHOD *DH_OpenSSL(void)
{
    return &dh_ossl;
}

const DH_METHOD *DH_get_default_method(void)
{
    return default_DH_method;
}

static int dh_bn_mod_exp(const DH *dh, BIGNUM *r,
                         const BIGNUM *a, const BIGNUM *p,
                         const BIGNUM *m, BN_CTX *ctx, BN_MONT_CTX *m_ctx)
{
    return BN_mod_exp_mont(r, a, p, m, ctx, m_ctx);
}

static int dh_init(DH *dh)
{
    dh->flags |= DH_FLAG_CACHE_MONT_P;
    ffc_params_init(&dh->params);
    dh->dirty_cnt++;
    return 1;
}

static int dh_finish(DH *dh)
{
    BN_MONT_CTX_free(dh->method_mont_p);
    return 1;
}

#ifndef FIPS_MODULE
void DH_set_default_method(const DH_METHOD *meth)
{
    default_DH_method = meth;
}
#endif /* FIPS_MODULE */

int DH_generate_key(DH *dh)
{
#ifdef FIPS_MODULE
    return generate_key(dh);
#else
    return dh->meth->generate_key(dh);
#endif
}

int dh_generate_public_key(BN_CTX *ctx, DH *dh, const BIGNUM *priv_key,
                           BIGNUM *pub_key)
{
    int ret = 0;
    BIGNUM *prk = BN_new();
    BN_MONT_CTX *mont = NULL;

    if (prk == NULL)
        return 0;

    if (dh->flags & DH_FLAG_CACHE_MONT_P) {
        mont = BN_MONT_CTX_set_locked(&dh->method_mont_p,
                                      dh->lock, dh->params.p, ctx);
        if (mont == NULL)
            goto err;
    }
    BN_with_flags(prk, priv_key, BN_FLG_CONSTTIME);

    /* pub_key = g^priv_key mod p */
    if (!dh->meth->bn_mod_exp(dh, pub_key, dh->params.g, prk, dh->params.p,
                              ctx, mont))
        goto err;
    ret = 1;
err:
    BN_clear_free(prk);
    return ret;
}

static int generate_key(DH *dh)
{
    int ok = 0;
    int generate_new_key = 0;
#ifndef FIPS_MODULE
    unsigned l;
#endif
    BN_CTX *ctx = NULL;
    BIGNUM *pub_key = NULL, *priv_key = NULL;

    if (BN_num_bits(dh->params.p) > OPENSSL_DH_MAX_MODULUS_BITS) {
        DHerr(0, DH_R_MODULUS_TOO_LARGE);
        return 0;
    }

    if (BN_num_bits(dh->params.p) < DH_MIN_MODULUS_BITS) {
        DHerr(0, DH_R_MODULUS_TOO_SMALL);
        return 0;
    }

    ctx = BN_CTX_new_ex(dh->libctx);
    if (ctx == NULL)
        goto err;

    if (dh->priv_key == NULL) {
        priv_key = BN_secure_new();
        if (priv_key == NULL)
            goto err;
        generate_new_key = 1;
    } else {
        priv_key = dh->priv_key;
    }

    if (dh->pub_key == NULL) {
        pub_key = BN_new();
        if (pub_key == NULL)
            goto err;
    } else {
        pub_key = dh->pub_key;
    }
    if (generate_new_key) {
        /* Is it an approved safe prime ?*/
        if (DH_get_nid(dh) != NID_undef) {
            int max_strength =
                    ifc_ffc_compute_security_bits(BN_num_bits(dh->params.p));

            if (dh->params.q == NULL
                || dh->length > BN_num_bits(dh->params.q))
                goto err;
            /* dh->length = maximum bit length of generated private key */
            if (!ffc_generate_private_key(ctx, &dh->params, dh->length,
                                          max_strength, priv_key))
                goto err;
        } else {
#ifdef FIPS_MODULE
            if (dh->params.q == NULL)
                goto err;
#else
            if (dh->params.q == NULL) {
                /* secret exponent length */
                l = dh->length ? dh->length : BN_num_bits(dh->params.p) - 1;
                if (!BN_priv_rand_ex(priv_key, l, BN_RAND_TOP_ONE,
                                     BN_RAND_BOTTOM_ANY, ctx))
                    goto err;
                /*
                 * We handle just one known case where g is a quadratic non-residue:
                 * for g = 2: p % 8 == 3
                 */
                if (BN_is_word(dh->params.g, DH_GENERATOR_2)
                    && !BN_is_bit_set(dh->params.p, 2)) {
                    /* clear bit 0, since it won't be a secret anyway */
                    if (!BN_clear_bit(priv_key, 0))
                        goto err;
                }
            } else
#endif
            {
                /* Do a partial check for invalid p, q, g */
                if (!ffc_params_simple_validate(dh->libctx, &dh->params,
                                                FFC_PARAM_TYPE_DH))
                    goto err;
                /*
                 * For FFC FIPS 186-4 keygen
                 * security strength s = 112,
                 * Max Private key size N = len(q)
                 */
                if (!ffc_generate_private_key(ctx, &dh->params,
                                              BN_num_bits(dh->params.q),
                                              MIN_STRENGTH,
                                              priv_key))
                    goto err;
            }
        }
    }

    if (!dh_generate_public_key(ctx, dh, priv_key, pub_key))
        goto err;

    dh->pub_key = pub_key;
    dh->priv_key = priv_key;
    dh->dirty_cnt++;
    ok = 1;
 err:
    if (ok != 1)
        DHerr(0, ERR_R_BN_LIB);

    if (pub_key != dh->pub_key)
        BN_free(pub_key);
    if (priv_key != dh->priv_key)
        BN_free(priv_key);
    BN_CTX_free(ctx);
    return ok;
}

int dh_buf2key(DH *dh, const unsigned char *buf, size_t len)
{
    int err_reason = DH_R_BN_ERROR;
    BIGNUM *pubkey = NULL;
    const BIGNUM *p;
    size_t p_size;

    if ((pubkey = BN_bin2bn(buf, len, NULL)) == NULL)
        goto err;
    DH_get0_pqg(dh, &p, NULL, NULL);
    if (p == NULL || (p_size = BN_num_bytes(p)) == 0) {
        err_reason = DH_R_NO_PARAMETERS_SET;
        goto err;
    }
    /*
     * As per Section 4.2.8.1 of RFC 8446 fail if DHE's
     * public key is of size not equal to size of p
     */
    if (BN_is_zero(pubkey) || p_size != len) {
        err_reason = DH_R_INVALID_PUBKEY;
        goto err;
    }
    if (DH_set0_key(dh, pubkey, NULL) != 1)
        goto err;
    return 1;
err:
    DHerr(DH_F_DH_BUF2KEY, err_reason);
    BN_free(pubkey);
    return 0;
}

size_t dh_key2buf(const DH *dh, unsigned char **pbuf_out, size_t size, int alloc)
{
    const BIGNUM *pubkey;
    unsigned char *pbuf = NULL;
    const BIGNUM *p;
    int p_size;

    DH_get0_pqg(dh, &p, NULL, NULL);
    DH_get0_key(dh, &pubkey, NULL);
    if (p == NULL || pubkey == NULL
            || (p_size = BN_num_bytes(p)) == 0
            || BN_num_bytes(pubkey) == 0) {
        DHerr(DH_F_DH_KEY2BUF, DH_R_INVALID_PUBKEY);
        return 0;
    }
    if (pbuf_out != NULL && (alloc || *pbuf_out != NULL)) {
        if (!alloc) {
            if (size >= (size_t)p_size)
                pbuf = *pbuf_out;
        } else {
            pbuf = OPENSSL_malloc(p_size);
        }

        if (pbuf == NULL) {
            DHerr(DH_F_DH_KEY2BUF, ERR_R_MALLOC_FAILURE);
            return 0;
        }
        /*
         * As per Section 4.2.8.1 of RFC 8446 left pad public
         * key with zeros to the size of p
         */
        if (BN_bn2binpad(pubkey, pbuf, p_size) < 0) {
            if (alloc)
                OPENSSL_free(pbuf);
            DHerr(DH_F_DH_KEY2BUF, DH_R_BN_ERROR);
            return 0;
        }
        *pbuf_out = pbuf;
    }
    return p_size;
}
