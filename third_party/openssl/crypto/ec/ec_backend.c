/*
 * Copyright 2020 The OpenSSL Project Authors. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://www.openssl.org/source/license.html
 */

#include <openssl/core_names.h>
#include <openssl/objects.h>
#include <openssl/params.h>
#include <openssl/err.h>
#include "crypto/bn.h"
#include "crypto/ec.h"
#include "ec_local.h"
#include "e_os.h"
#include "internal/param_build_set.h"

/* Mapping between a flag and a name */
static const OSSL_ITEM encoding_nameid_map[] = {
    { OPENSSL_EC_EXPLICIT_CURVE, OSSL_PKEY_EC_ENCODING_EXPLICIT },
    { OPENSSL_EC_NAMED_CURVE, OSSL_PKEY_EC_ENCODING_GROUP },
};

int ec_encoding_name2id(const char *name)
{
    size_t i, sz;

    /* Return the default value if there is no name */
    if (name == NULL)
        return OPENSSL_EC_NAMED_CURVE;

    for (i = 0, sz = OSSL_NELEM(encoding_nameid_map); i < sz; i++) {
        if (strcasecmp(name, encoding_nameid_map[i].ptr) == 0)
            return encoding_nameid_map[i].id;
    }
    return -1;
}

static char *ec_param_encoding_id2name(int id)
{
    size_t i, sz;

    for (i = 0, sz = OSSL_NELEM(encoding_nameid_map); i < sz; i++) {
        if (id == (int)encoding_nameid_map[i].id)
            return encoding_nameid_map[i].ptr;
    }
    return NULL;
}

int ec_group_todata(const EC_GROUP *group, OSSL_PARAM_BLD *tmpl,
                    OSSL_PARAM params[], OPENSSL_CTX *libctx, const char *propq,
                    BN_CTX *bnctx, unsigned char **genbuf)
{
    int ret = 0, curve_nid, encoding_flag;
    const char *field_type, *encoding_name;
    const BIGNUM *cofactor, *order;
    BIGNUM *p = NULL, *a = NULL, *b = NULL;
    point_conversion_form_t genform;
    const EC_POINT *genpt;
    unsigned char *seed = NULL;
    size_t genbuf_len, seed_len;

    if (group == NULL) {
        ECerr(0,EC_R_PASSED_NULL_PARAMETER);
        return 0;
    }

    encoding_flag = EC_GROUP_get_asn1_flag(group) & OPENSSL_EC_NAMED_CURVE;
    encoding_name = ec_param_encoding_id2name(encoding_flag);
    if (encoding_name == NULL
        || !ossl_param_build_set_utf8_string(tmpl, params,
                                             OSSL_PKEY_PARAM_EC_ENCODING,
                                             encoding_name)) {
        ECerr(0, EC_R_INVALID_ENCODING);
        return 0;
    }

    curve_nid = EC_GROUP_get_curve_name(group);
    if (curve_nid == NID_undef) {
        /* explicit curve */
        int fid = EC_GROUP_get_field_type(group);

        if (fid == NID_X9_62_prime_field) {
            field_type = SN_X9_62_prime_field;
        } else if (fid == NID_X9_62_characteristic_two_field) {
            field_type = SN_X9_62_characteristic_two_field;
        } else {
            ECerr(0, EC_R_INVALID_FIELD);
            return 0;
        }

        p = BN_CTX_get(bnctx);
        a = BN_CTX_get(bnctx);
        b = BN_CTX_get(bnctx);
        if (b == NULL) {
            ECerr(0, ERR_R_MALLOC_FAILURE);
            goto err;
        }

        if (!EC_GROUP_get_curve(group, p, a, b, bnctx)) {
            ECerr(0, EC_R_INVALID_CURVE);
            goto err;
        }

        order = EC_GROUP_get0_order(group);
        if (order == NULL) {
            ECerr(0, EC_R_INVALID_GROUP_ORDER);
            goto err;
        }
        genpt = EC_GROUP_get0_generator(group);
        if (genpt == NULL) {
            ECerr(0, EC_R_INVALID_GENERATOR);
            goto err;
        }
        genform = EC_GROUP_get_point_conversion_form(group);
        genbuf_len = EC_POINT_point2buf(group, genpt, genform, genbuf, bnctx);
        if (genbuf_len == 0) {
            ECerr(0, EC_R_INVALID_GENERATOR);
            goto err;
        }

        if (!ossl_param_build_set_utf8_string(tmpl, params,
                                              OSSL_PKEY_PARAM_EC_FIELD_TYPE,
                                              field_type)
            || !ossl_param_build_set_bn(tmpl, params, OSSL_PKEY_PARAM_EC_P, p)
            || !ossl_param_build_set_bn(tmpl, params, OSSL_PKEY_PARAM_EC_A, a)
            || !ossl_param_build_set_bn(tmpl, params, OSSL_PKEY_PARAM_EC_B, b)
            || !ossl_param_build_set_bn(tmpl, params, OSSL_PKEY_PARAM_EC_ORDER,
                                        order)
            || !ossl_param_build_set_octet_string(tmpl, params,
                                                  OSSL_PKEY_PARAM_EC_GENERATOR,
                                                  *genbuf, genbuf_len)) {
            ECerr(0, ERR_R_MALLOC_FAILURE);
            goto err;
        }

        cofactor = EC_GROUP_get0_cofactor(group);
        if (cofactor != NULL
            && !ossl_param_build_set_bn(tmpl, params,
                                        OSSL_PKEY_PARAM_EC_COFACTOR, cofactor)) {
            ECerr(0, ERR_R_MALLOC_FAILURE);
            goto err;
        }

        seed = EC_GROUP_get0_seed(group);
        seed_len = EC_GROUP_get_seed_len(group);
        if (seed != NULL
            && seed_len > 0
            && !ossl_param_build_set_octet_string(tmpl, params,
                                                  OSSL_PKEY_PARAM_EC_SEED,
                                                  seed, seed_len)) {
            ECerr(0, ERR_R_MALLOC_FAILURE);
            goto err;
        }
#ifdef OPENSSL_NO_EC2M
        if (fid == NID_X9_62_characteristic_two_field) {
            ECerr(0, EC_R_GF2M_NOT_SUPPORTED);
            goto err;
        }
#endif
    } else {
        /* named curve */
        const char *curve_name = ec_curve_nid2name(curve_nid);

        if (curve_name == NULL
            || !ossl_param_build_set_utf8_string(tmpl, params,
                                                 OSSL_PKEY_PARAM_GROUP_NAME,
                                                 curve_name)) {
            ECerr(0, EC_R_INVALID_CURVE);
            goto err;
        }
    }
    ret = 1;
err:
    return ret;
}

/*
 * The intention with the "backend" source file is to offer backend support
 * for legacy backends (EVP_PKEY_ASN1_METHOD and EVP_PKEY_METHOD) and provider
 * implementations alike.
 */
int ec_set_ecdh_cofactor_mode(EC_KEY *ec, int mode)
{
    const EC_GROUP *ecg = EC_KEY_get0_group(ec);
    const BIGNUM *cofactor;
    /*
     * mode can be only 0 for disable, or 1 for enable here.
     *
     * This is in contrast with the same parameter on an ECDH EVP_PKEY_CTX that
     * also supports mode == -1 with the meaning of "reset to the default for
     * the associated key".
     */
    if (mode < 0 || mode > 1)
        return 0;

    if ((cofactor = EC_GROUP_get0_cofactor(ecg)) == NULL )
        return 0;

    /* ECDH cofactor mode has no effect if cofactor is 1 */
    if (BN_is_one(cofactor))
        return 1;

    if (mode == 1)
        EC_KEY_set_flags(ec, EC_FLAG_COFACTOR_ECDH);
    else if (mode == 0)
        EC_KEY_clear_flags(ec, EC_FLAG_COFACTOR_ECDH);

    return 1;
}

/*
 * Callers of ec_key_fromdata MUST make sure that ec_key_params_fromdata has
 * been called before!
 *
 * This function only gets the bare keypair, domain parameters and other
 * parameters are treated separately, and domain parameters are required to
 * define a keypair.
 */
int ec_key_fromdata(EC_KEY *ec, const OSSL_PARAM params[], int include_private)
{
    const OSSL_PARAM *param_priv_key = NULL, *param_pub_key = NULL;
    BN_CTX *ctx = NULL;
    BIGNUM *priv_key = NULL;
    unsigned char *pub_key = NULL;
    size_t pub_key_len;
    const EC_GROUP *ecg = NULL;
    EC_POINT *pub_point = NULL;
    int ok = 0;

    ecg = EC_KEY_get0_group(ec);
    if (ecg == NULL)
        return 0;

    param_pub_key =
        OSSL_PARAM_locate_const(params, OSSL_PKEY_PARAM_PUB_KEY);
    if (include_private)
        param_priv_key =
            OSSL_PARAM_locate_const(params, OSSL_PKEY_PARAM_PRIV_KEY);

    ctx = BN_CTX_new_ex(ec_key_get_libctx(ec));
    if (ctx == NULL)
        goto err;

    /* OpenSSL decree: If there's a private key, there must be a public key */
    if (param_priv_key != NULL && param_pub_key == NULL)
        goto err;

    if (param_pub_key != NULL)
        if (!OSSL_PARAM_get_octet_string(param_pub_key,
                                         (void **)&pub_key, 0, &pub_key_len)
            || (pub_point = EC_POINT_new(ecg)) == NULL
            || !EC_POINT_oct2point(ecg, pub_point, pub_key, pub_key_len, ctx))
        goto err;

    if (param_priv_key != NULL && include_private) {
        int fixed_words;
        const BIGNUM *order;

        /*
         * Key import/export should never leak the bit length of the secret
         * scalar in the key.
         *
         * For this reason, on export we use padded BIGNUMs with fixed length.
         *
         * When importing we also should make sure that, even if short lived,
         * the newly created BIGNUM is marked with the BN_FLG_CONSTTIME flag as
         * soon as possible, so that any processing of this BIGNUM might opt for
         * constant time implementations in the backend.
         *
         * Setting the BN_FLG_CONSTTIME flag alone is never enough, we also have
         * to preallocate the BIGNUM internal buffer to a fixed public size big
         * enough that operations performed during the processing never trigger
         * a realloc which would leak the size of the scalar through memory
         * accesses.
         *
         * Fixed Length
         * ------------
         *
         * The order of the large prime subgroup of the curve is our choice for
         * a fixed public size, as that is generally the upper bound for
         * generating a private key in EC cryptosystems and should fit all valid
         * secret scalars.
         *
         * For padding on export we just use the bit length of the order
         * converted to bytes (rounding up).
         *
         * For preallocating the BIGNUM storage we look at the number of "words"
         * required for the internal representation of the order, and we
         * preallocate 2 extra "words" in case any of the subsequent processing
         * might temporarily overflow the order length.
         */
        order = EC_GROUP_get0_order(ecg);
        if (order == NULL || BN_is_zero(order))
            goto err;

        fixed_words = bn_get_top(order) + 2;

        if ((priv_key = BN_secure_new()) == NULL)
            goto err;
        if (bn_wexpand(priv_key, fixed_words) == NULL)
            goto err;
        BN_set_flags(priv_key, BN_FLG_CONSTTIME);

        if (!OSSL_PARAM_get_BN(param_priv_key, &priv_key))
            goto err;
    }

    if (priv_key != NULL
        && !EC_KEY_set_private_key(ec, priv_key))
        goto err;

    if (pub_point != NULL
        && !EC_KEY_set_public_key(ec, pub_point))
        goto err;

    ok = 1;

 err:
    BN_CTX_free(ctx);
    BN_clear_free(priv_key);
    OPENSSL_free(pub_key);
    EC_POINT_free(pub_point);
    return ok;
}

int ec_group_fromdata(EC_KEY *ec, const OSSL_PARAM params[])
{
    int ok = 0;
    EC_GROUP *group = NULL;

    if (ec == NULL)
        return 0;

     group = EC_GROUP_new_from_params(params, ec_key_get_libctx(ec),
                                      ec_key_get0_propq(ec));

    if (!EC_KEY_set_group(ec, group))
        goto err;

    /*
     * TODO(3.0): if the group has changed, should we invalidate the private and
     * public key?
     */
    ok = 1;
err:
    EC_GROUP_free(group);
    return ok;
}

int ec_key_otherparams_fromdata(EC_KEY *ec, const OSSL_PARAM params[])
{
    const OSSL_PARAM *p;

    if (ec == NULL)
        return 0;

    p = OSSL_PARAM_locate_const(params, OSSL_PKEY_PARAM_USE_COFACTOR_ECDH);
    if (p != NULL) {
        int mode;

        if (!OSSL_PARAM_get_int(p, &mode)
            || !ec_set_ecdh_cofactor_mode(ec, mode))
            return 0;
    }

    return 1;
}
