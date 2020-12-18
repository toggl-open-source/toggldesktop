/*
 * Copyright 1995-2020 The OpenSSL Project Authors. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://www.openssl.org/source/license.html
 */

#include <stdio.h>
#include "internal/cryptlib.h"
#include <openssl/bn.h>
#include "dsa_local.h"
#include "crypto/dsa.h"

int dsa_check_params(const DSA *dsa, int *ret)
{
    /*
     * (2b) FFC domain params conform to FIPS-186-4 explicit domain param
     * validity tests.
     */
    return ffc_params_FIPS186_4_validate(dsa->libctx, &dsa->params,
                                         FFC_PARAM_TYPE_DSA, ret, NULL);
}

/*
 * See SP800-56Ar3 Section 5.6.2.3.1 : FFC Full public key validation.
 */
int dsa_check_pub_key(const DSA *dsa, const BIGNUM *pub_key, int *ret)
{
    return ffc_validate_public_key(&dsa->params, pub_key, ret);
}

/*
 * See SP800-56Ar3 Section 5.6.2.3.1 : FFC Partial public key validation.
 * To only be used with ephemeral FFC public keys generated using the approved
 * safe-prime groups.
 */
int dsa_check_pub_key_partial(const DSA *dsa, const BIGNUM *pub_key, int *ret)
{
    return ffc_validate_public_key_partial(&dsa->params, pub_key, ret);
}

int dsa_check_priv_key(const DSA *dsa, const BIGNUM *priv_key, int *ret)
{
    *ret = 0;

    return (dsa->params.q != NULL
            && ffc_validate_private_key(dsa->params.q, priv_key, ret));
}

/*
 * FFC pairwise check from SP800-56A R3.
 *    Section 5.6.2.1.4 Owner Assurance of Pair-wise Consistency
 */
int dsa_check_pairwise(const DSA *dsa)
{
    int ret = 0;
    BN_CTX *ctx = NULL;
    BIGNUM *pub_key = NULL;

    if (dsa->params.p == NULL
        || dsa->params.g == NULL
        || dsa->priv_key == NULL
        || dsa->pub_key == NULL)
        return 0;

    ctx = BN_CTX_new_ex(dsa->libctx);
    if (ctx == NULL)
        goto err;
    pub_key = BN_new();
    if (pub_key == NULL)
        goto err;

    /* recalculate the public key = (g ^ priv) mod p */
    if (!dsa_generate_public_key(ctx, dsa, dsa->priv_key, pub_key))
        goto err;
    /* check it matches the existing pubic_key */
    ret = BN_cmp(pub_key, dsa->pub_key) == 0;
err:
    BN_free(pub_key);
    BN_CTX_free(ctx);
    return ret;
}
