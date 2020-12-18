/*
 * Copyright 2006-2020 The OpenSSL Project Authors. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://www.openssl.org/source/license.html
 */

#include <stdio.h>
#include <stdlib.h>
#include <openssl/core.h>
#include <openssl/core_names.h>
#include "internal/cryptlib.h"
#include "internal/core.h"
#include <openssl/objects.h>
#include <openssl/evp.h>
#include "crypto/bn.h"
#include "crypto/asn1.h"
#include "crypto/evp.h"
#include "evp_local.h"

#if !defined(FIPS_MODULE) && !defined(OPENSSL_NO_EC)
# define TMP_SM2_HACK
#endif

/* TODO(3.0) remove when provider SM2 key generation is implemented */
#ifdef TMP_SM2_HACK
# include <openssl/ec.h>
# include "internal/sizes.h"
#endif

static int gen_init(EVP_PKEY_CTX *ctx, int operation)
{
    int ret = 0;

    if (ctx == NULL)
        goto not_supported;

    evp_pkey_ctx_free_old_ops(ctx);
    ctx->operation = operation;

    if (ctx->keymgmt == NULL || ctx->keymgmt->gen_init == NULL)
        goto legacy;

/* TODO remove when provider SM2 key generation is implemented */
#ifdef TMP_SM2_HACK
    if (ctx->pmeth != NULL && ctx->pmeth->pkey_id == EVP_PKEY_SM2)
        goto legacy;
#endif

    switch (operation) {
    case EVP_PKEY_OP_PARAMGEN:
        ctx->op.keymgmt.genctx =
            evp_keymgmt_gen_init(ctx->keymgmt,
                                 OSSL_KEYMGMT_SELECT_ALL_PARAMETERS);
        break;
    case EVP_PKEY_OP_KEYGEN:
        ctx->op.keymgmt.genctx =
            evp_keymgmt_gen_init(ctx->keymgmt, OSSL_KEYMGMT_SELECT_KEYPAIR);
        break;
    }

    if (ctx->op.keymgmt.genctx == NULL)
        ERR_raise(ERR_LIB_EVP, EVP_R_INITIALIZATION_ERROR);
    else
        ret = 1;
    goto end;

 legacy:
#ifdef FIPS_MODULE
    goto not_supported;
#else
    if (ctx->pmeth == NULL
        || (operation == EVP_PKEY_OP_PARAMGEN
            && ctx->pmeth->paramgen == NULL)
        || (operation == EVP_PKEY_OP_KEYGEN
            && ctx->pmeth->keygen == NULL))
        goto not_supported;

    ret = 1;
    switch (operation) {
    case EVP_PKEY_OP_PARAMGEN:
        if (ctx->pmeth->paramgen_init != NULL)
            ret = ctx->pmeth->paramgen_init(ctx);
        break;
    case EVP_PKEY_OP_KEYGEN:
        if (ctx->pmeth->keygen_init != NULL)
            ret = ctx->pmeth->keygen_init(ctx);
        break;
    }
#endif

 end:
    if (ret <= 0 && ctx != NULL) {
        evp_pkey_ctx_free_old_ops(ctx);
        ctx->operation = EVP_PKEY_OP_UNDEFINED;
    }
    return ret;

 not_supported:
    ERR_raise(ERR_LIB_EVP, EVP_R_OPERATION_NOT_SUPPORTED_FOR_THIS_KEYTYPE);
    ret = -2;
    goto end;
}

int EVP_PKEY_paramgen_init(EVP_PKEY_CTX *ctx)
{
    return gen_init(ctx, EVP_PKEY_OP_PARAMGEN);
}

int EVP_PKEY_keygen_init(EVP_PKEY_CTX *ctx)
{
    return gen_init(ctx, EVP_PKEY_OP_KEYGEN);
}

static int ossl_callback_to_pkey_gencb(const OSSL_PARAM params[], void *arg)
{
    EVP_PKEY_CTX *ctx = arg;
    const OSSL_PARAM *param = NULL;
    int p = -1, n = -1;

    if (ctx->pkey_gencb == NULL)
        return 1;                /* No callback?  That's fine */

    if ((param = OSSL_PARAM_locate_const(params, OSSL_GEN_PARAM_POTENTIAL))
        == NULL
        || !OSSL_PARAM_get_int(param, &p))
        return 0;
    if ((param = OSSL_PARAM_locate_const(params, OSSL_GEN_PARAM_ITERATION))
        == NULL
        || !OSSL_PARAM_get_int(param, &n))
        return 0;

    ctx->keygen_info[0] = p;
    ctx->keygen_info[1] = n;

    return ctx->pkey_gencb(ctx);
}

int EVP_PKEY_gen(EVP_PKEY_CTX *ctx, EVP_PKEY **ppkey)
{
    int ret = 0;
    OSSL_CALLBACK cb;
    EVP_PKEY *allocated_pkey = NULL;
    /* Legacy compatible keygen callback info, only used with provider impls */
    int gentmp[2];

    if (ppkey == NULL)
        return -1;

    if (ctx == NULL)
        goto not_supported;

    if ((ctx->operation & EVP_PKEY_OP_TYPE_GEN) == 0)
        goto not_initialized;

    if (*ppkey == NULL)
        *ppkey = allocated_pkey = EVP_PKEY_new();

    if (*ppkey == NULL) {
        ERR_raise(ERR_LIB_EVP, ERR_R_MALLOC_FAILURE);
        return -1;
    }

    if (ctx->op.keymgmt.genctx == NULL)
        goto legacy;

    /*
     * Asssigning gentmp to ctx->keygen_info is something our legacy
     * implementations do.  Because the provider implementations aren't
     * allowed to reach into our EVP_PKEY_CTX, we need to provide similar
     * space for backward compatibility.  It's ok that we attach a local
     * variable, as it should only be useful in the calls down from here.
     * This is cleared as soon as it isn't useful any more, i.e. directly
     * after the evp_keymgmt_util_gen() call.
     */
    ctx->keygen_info = gentmp;
    ctx->keygen_info_count = 2;

    ret = 1;
    if (ctx->pkey != NULL) {
        EVP_KEYMGMT *tmp_keymgmt = ctx->keymgmt;
        void *keydata =
            evp_pkey_export_to_provider(ctx->pkey, ctx->libctx,
                                        &tmp_keymgmt, ctx->propquery);

        if (tmp_keymgmt == NULL)
            goto not_supported;
        /*
         * It's ok if keydata is NULL here.  The backend is expected to deal
         * with that as it sees fit.
         */
        ret = evp_keymgmt_gen_set_template(ctx->keymgmt,
                                           ctx->op.keymgmt.genctx, keydata);
    }

    /*
     * the returned value from evp_keymgmt_util_gen() is cached in *ppkey,
     * so we so not need to save it, just check it.
     */
    ret = ret
        && (evp_keymgmt_util_gen(*ppkey, ctx->keymgmt, ctx->op.keymgmt.genctx,
                                 ossl_callback_to_pkey_gencb, ctx)
            != NULL);

    ctx->keygen_info = NULL;

#ifndef FIPS_MODULE
    /* In case |*ppkey| was originally a legacy key */
    if (ret)
        evp_pkey_free_legacy(*ppkey);
#endif

    /*
     * Because we still have legacy keys, and evp_pkey_downgrade()
     * TODO remove this #legacy internal keys are gone
     */
    (*ppkey)->type = ctx->legacy_keytype;

/* TODO remove when SM2 key have been cleanly separated from EC keys */
#ifdef TMP_SM2_HACK
    /*
     * Legacy SM2 keys are implemented as EC_KEY with a twist.  The legacy
     * key generation detects the SM2 curve and "magically" changes the pkey
     * id accordingly.
     * Since we don't have SM2 in the provider implementation, we need to
     * downgrade the generated provider side key to a legacy one under the
     * same conditions.
     *
     * THIS IS AN UGLY BUT TEMPORARY HACK
     */
    {
        char curve_name[OSSL_MAX_NAME_SIZE] = "";

        if (!EVP_PKEY_get_utf8_string_param(*ppkey, OSSL_PKEY_PARAM_GROUP_NAME,
                                            curve_name, sizeof(curve_name),
                                            NULL)
            || strcmp(curve_name, "SM2") != 0)
            goto end;
    }

    if (!evp_pkey_downgrade(*ppkey)
        || !EVP_PKEY_set_alias_type(*ppkey, EVP_PKEY_SM2))
        ret = 0;
#endif
    goto end;

 legacy:
#ifdef FIPS_MODULE
    goto not_supported;
#else
    if (ctx->pkey && !evp_pkey_downgrade(ctx->pkey))
        goto not_accessible;
    switch (ctx->operation) {
    case EVP_PKEY_OP_PARAMGEN:
        ret = ctx->pmeth->paramgen(ctx, *ppkey);
        break;
    case EVP_PKEY_OP_KEYGEN:
        ret = ctx->pmeth->keygen(ctx, *ppkey);
        break;
    default:
        goto not_supported;
    }
#endif

 end:
    if (ret <= 0) {
        if (allocated_pkey != NULL)
            *ppkey = NULL;
        EVP_PKEY_free(allocated_pkey);
    }
    return ret;

 not_supported:
    ERR_raise(ERR_LIB_EVP, EVP_R_OPERATION_NOT_SUPPORTED_FOR_THIS_KEYTYPE);
    ret = -2;
    goto end;
 not_initialized:
    ERR_raise(ERR_LIB_EVP, EVP_R_OPERATON_NOT_INITIALIZED);
    ret = -1;
    goto end;
#ifndef FIPS_MODULE
 not_accessible:
    ERR_raise(ERR_LIB_EVP, EVP_R_INACCESSIBLE_DOMAIN_PARAMETERS);
    ret = -1;
    goto end;
#endif
}

int EVP_PKEY_paramgen(EVP_PKEY_CTX *ctx, EVP_PKEY **ppkey)
{
    if (ctx->operation != EVP_PKEY_OP_PARAMGEN) {
        ERR_raise(ERR_LIB_EVP, EVP_R_OPERATON_NOT_INITIALIZED);
        return -1;
    }
    return EVP_PKEY_gen(ctx, ppkey);
}

int EVP_PKEY_keygen(EVP_PKEY_CTX *ctx, EVP_PKEY **ppkey)
{
    if (ctx->operation != EVP_PKEY_OP_KEYGEN) {
        ERR_raise(ERR_LIB_EVP, EVP_R_OPERATON_NOT_INITIALIZED);
        return -1;
    }
    return EVP_PKEY_gen(ctx, ppkey);
}

void EVP_PKEY_CTX_set_cb(EVP_PKEY_CTX *ctx, EVP_PKEY_gen_cb *cb)
{
    ctx->pkey_gencb = cb;
}

EVP_PKEY_gen_cb *EVP_PKEY_CTX_get_cb(EVP_PKEY_CTX *ctx)
{
    return ctx->pkey_gencb;
}

/*
 * "translation callback" to call EVP_PKEY_CTX callbacks using BN_GENCB style
 * callbacks.
 */

static int trans_cb(int a, int b, BN_GENCB *gcb)
{
    EVP_PKEY_CTX *ctx = BN_GENCB_get_arg(gcb);
    ctx->keygen_info[0] = a;
    ctx->keygen_info[1] = b;
    return ctx->pkey_gencb(ctx);
}

void evp_pkey_set_cb_translate(BN_GENCB *cb, EVP_PKEY_CTX *ctx)
{
    BN_GENCB_set(cb, trans_cb, ctx);
}

int EVP_PKEY_CTX_get_keygen_info(EVP_PKEY_CTX *ctx, int idx)
{
    if (idx == -1)
        return ctx->keygen_info_count;
    if (idx < 0 || idx > ctx->keygen_info_count)
        return 0;
    return ctx->keygen_info[idx];
}

#ifndef FIPS_MODULE

EVP_PKEY *EVP_PKEY_new_mac_key(int type, ENGINE *e,
                               const unsigned char *key, int keylen)
{
    EVP_PKEY_CTX *mac_ctx = NULL;
    EVP_PKEY *mac_key = NULL;
    mac_ctx = EVP_PKEY_CTX_new_id(type, e);
    if (!mac_ctx)
        return NULL;
    if (EVP_PKEY_keygen_init(mac_ctx) <= 0)
        goto merr;
    if (EVP_PKEY_CTX_set_mac_key(mac_ctx, key, keylen) <= 0)
        goto merr;
    if (EVP_PKEY_keygen(mac_ctx, &mac_key) <= 0)
        goto merr;
 merr:
    EVP_PKEY_CTX_free(mac_ctx);
    return mac_key;
}

#endif /* FIPS_MODULE */

/*- All methods below can also be used in FIPS_MODULE */

static int fromdata_init(EVP_PKEY_CTX *ctx, int operation)
{
    if (ctx == NULL || ctx->keytype == NULL)
        goto not_supported;

    evp_pkey_ctx_free_old_ops(ctx);
    if (ctx->keymgmt == NULL)
        goto not_supported;

    ctx->operation = operation;
    return 1;

 not_supported:
    if (ctx != NULL)
        ctx->operation = EVP_PKEY_OP_UNDEFINED;
    ERR_raise(ERR_LIB_EVP, EVP_R_OPERATION_NOT_SUPPORTED_FOR_THIS_KEYTYPE);
    return -2;
}

int EVP_PKEY_param_fromdata_init(EVP_PKEY_CTX *ctx)
{
    return fromdata_init(ctx, EVP_PKEY_OP_PARAMFROMDATA);
}

int EVP_PKEY_key_fromdata_init(EVP_PKEY_CTX *ctx)
{
    return fromdata_init(ctx, EVP_PKEY_OP_KEYFROMDATA);
}

int EVP_PKEY_fromdata(EVP_PKEY_CTX *ctx, EVP_PKEY **ppkey, OSSL_PARAM params[])
{
    void *keydata = NULL;
    int selection;

    if (ctx == NULL || (ctx->operation & EVP_PKEY_OP_TYPE_FROMDATA) == 0) {
        ERR_raise(ERR_LIB_EVP, EVP_R_OPERATION_NOT_SUPPORTED_FOR_THIS_KEYTYPE);
        return -2;
    }

    if (ppkey == NULL)
        return -1;

    if (*ppkey == NULL)
        *ppkey = EVP_PKEY_new();

    if (*ppkey == NULL) {
        ERR_raise(ERR_LIB_EVP, ERR_R_MALLOC_FAILURE);
        return -1;
    }

    if (ctx->operation == EVP_PKEY_OP_PARAMFROMDATA)
        selection = OSSL_KEYMGMT_SELECT_ALL_PARAMETERS;
    else
        selection = OSSL_KEYMGMT_SELECT_ALL;
    keydata = evp_keymgmt_util_fromdata(*ppkey, ctx->keymgmt, selection,
                                        params);

    if (keydata == NULL)
        return 0;
    /* keydata is cached in *ppkey, so we need not bother with it further */
    return 1;
}

/*
 * TODO(3.0) Re-evaluate the names, it's possible that we find these to be
 * better:
 *
 * EVP_PKEY_param_settable()
 * EVP_PKEY_param_gettable()
 */
const OSSL_PARAM *EVP_PKEY_param_fromdata_settable(EVP_PKEY_CTX *ctx)
{
    /* We call fromdata_init to get ctx->keymgmt populated */
    if (fromdata_init(ctx, EVP_PKEY_OP_UNDEFINED))
        return evp_keymgmt_import_types(ctx->keymgmt,
                                        OSSL_KEYMGMT_SELECT_ALL_PARAMETERS);
    return NULL;
}

const OSSL_PARAM *EVP_PKEY_key_fromdata_settable(EVP_PKEY_CTX *ctx)
{
    /* We call fromdata_init to get ctx->keymgmt populated */
    if (fromdata_init(ctx, EVP_PKEY_OP_UNDEFINED))
        return evp_keymgmt_import_types(ctx->keymgmt,
                                        OSSL_KEYMGMT_SELECT_ALL);
    return NULL;
}
