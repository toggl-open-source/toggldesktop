/*
 * Copyright 2019-2020 The OpenSSL Project Authors. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://www.openssl.org/source/license.html
 */

#include <openssl/crypto.h>
#include <openssl/evp.h>
#include <openssl/err.h>
#include "internal/refcount.h"
#include "crypto/evp.h"
#include "internal/provider.h"
#include "internal/numbers.h"   /* includes SIZE_MAX */
#include "evp_local.h"

static EVP_KEYEXCH *evp_keyexch_new(OSSL_PROVIDER *prov)
{
    EVP_KEYEXCH *exchange = OPENSSL_zalloc(sizeof(EVP_KEYEXCH));

    if (exchange == NULL) {
        ERR_raise(ERR_LIB_EVP, ERR_R_MALLOC_FAILURE);
        return NULL;
    }

    exchange->lock = CRYPTO_THREAD_lock_new();
    if (exchange->lock == NULL) {
        ERR_raise(ERR_LIB_EVP, ERR_R_MALLOC_FAILURE);
        OPENSSL_free(exchange);
        return NULL;
    }
    exchange->prov = prov;
    ossl_provider_up_ref(prov);
    exchange->refcnt = 1;

    return exchange;
}

static void *evp_keyexch_from_dispatch(int name_id,
                                       const OSSL_DISPATCH *fns,
                                       OSSL_PROVIDER *prov)
{
    EVP_KEYEXCH *exchange = NULL;
    int fncnt = 0, sparamfncnt = 0, gparamfncnt = 0;

    if ((exchange = evp_keyexch_new(prov)) == NULL) {
        ERR_raise(ERR_LIB_EVP, ERR_R_MALLOC_FAILURE);
        goto err;
    }

    exchange->name_id = name_id;

    for (; fns->function_id != 0; fns++) {
        switch (fns->function_id) {
        case OSSL_FUNC_KEYEXCH_NEWCTX:
            if (exchange->newctx != NULL)
                break;
            exchange->newctx = OSSL_FUNC_keyexch_newctx(fns);
            fncnt++;
            break;
        case OSSL_FUNC_KEYEXCH_INIT:
            if (exchange->init != NULL)
                break;
            exchange->init = OSSL_FUNC_keyexch_init(fns);
            fncnt++;
            break;
        case OSSL_FUNC_KEYEXCH_SET_PEER:
            if (exchange->set_peer != NULL)
                break;
            exchange->set_peer = OSSL_FUNC_keyexch_set_peer(fns);
            break;
        case OSSL_FUNC_KEYEXCH_DERIVE:
            if (exchange->derive != NULL)
                break;
            exchange->derive = OSSL_FUNC_keyexch_derive(fns);
            fncnt++;
            break;
        case OSSL_FUNC_KEYEXCH_FREECTX:
            if (exchange->freectx != NULL)
                break;
            exchange->freectx = OSSL_FUNC_keyexch_freectx(fns);
            fncnt++;
            break;
        case OSSL_FUNC_KEYEXCH_DUPCTX:
            if (exchange->dupctx != NULL)
                break;
            exchange->dupctx = OSSL_FUNC_keyexch_dupctx(fns);
            break;
        case OSSL_FUNC_KEYEXCH_GET_CTX_PARAMS:
            if (exchange->get_ctx_params != NULL)
                break;
            exchange->get_ctx_params = OSSL_FUNC_keyexch_get_ctx_params(fns);
            gparamfncnt++;
            break;
        case OSSL_FUNC_KEYEXCH_GETTABLE_CTX_PARAMS:
            if (exchange->gettable_ctx_params != NULL)
                break;
            exchange->gettable_ctx_params
                = OSSL_FUNC_keyexch_gettable_ctx_params(fns);
            gparamfncnt++;
            break;
        case OSSL_FUNC_KEYEXCH_SET_CTX_PARAMS:
            if (exchange->set_ctx_params != NULL)
                break;
            exchange->set_ctx_params = OSSL_FUNC_keyexch_set_ctx_params(fns);
            sparamfncnt++;
            break;
        case OSSL_FUNC_KEYEXCH_SETTABLE_CTX_PARAMS:
            if (exchange->settable_ctx_params != NULL)
                break;
            exchange->settable_ctx_params
                = OSSL_FUNC_keyexch_settable_ctx_params(fns);
            sparamfncnt++;
            break;
        }
    }
    if (fncnt != 4
            || (gparamfncnt != 0 && gparamfncnt != 2)
            || (sparamfncnt != 0 && sparamfncnt != 2)) {
        /*
         * In order to be a consistent set of functions we must have at least
         * a complete set of "exchange" functions: init, derive, newctx,
         * and freectx. The set_ctx_params and settable_ctx_params functions are
         * optional, but if one of them is present then the other one must also
         * be present. Same goes for get_ctx_params and gettable_ctx_params.
         * The dupctx and set_peer functions are optional.
         */
        EVPerr(EVP_F_EVP_KEYEXCH_FROM_DISPATCH,
               EVP_R_INVALID_PROVIDER_FUNCTIONS);
        goto err;
    }

    return exchange;

 err:
    EVP_KEYEXCH_free(exchange);
    return NULL;
}

void EVP_KEYEXCH_free(EVP_KEYEXCH *exchange)
{
    if (exchange != NULL) {
        int i;

        CRYPTO_DOWN_REF(&exchange->refcnt, &i, exchange->lock);
        if (i > 0)
            return;
        ossl_provider_free(exchange->prov);
        CRYPTO_THREAD_lock_free(exchange->lock);
        OPENSSL_free(exchange);
    }
}

int EVP_KEYEXCH_up_ref(EVP_KEYEXCH *exchange)
{
    int ref = 0;

    CRYPTO_UP_REF(&exchange->refcnt, &ref, exchange->lock);
    return 1;
}

OSSL_PROVIDER *EVP_KEYEXCH_provider(const EVP_KEYEXCH *exchange)
{
    return exchange->prov;
}

EVP_KEYEXCH *EVP_KEYEXCH_fetch(OPENSSL_CTX *ctx, const char *algorithm,
                               const char *properties)
{
    return evp_generic_fetch(ctx, OSSL_OP_KEYEXCH, algorithm, properties,
                             evp_keyexch_from_dispatch,
                             (int (*)(void *))EVP_KEYEXCH_up_ref,
                             (void (*)(void *))EVP_KEYEXCH_free);
}

int EVP_PKEY_derive_init(EVP_PKEY_CTX *ctx)
{
    int ret;
    void *provkey = NULL;
    EVP_KEYEXCH *exchange = NULL;
    EVP_KEYMGMT *tmp_keymgmt = NULL;
    const char *supported_exch = NULL;

    if (ctx == NULL) {
        EVPerr(0, EVP_R_OPERATION_NOT_SUPPORTED_FOR_THIS_KEYTYPE);
        return -2;
    }

    evp_pkey_ctx_free_old_ops(ctx);
    ctx->operation = EVP_PKEY_OP_DERIVE;

    /*
     * TODO when we stop falling back to legacy, this and the ERR_pop_to_mark()
     * calls can be removed.
     */
    ERR_set_mark();

    if (ctx->keymgmt == NULL)
        goto legacy;

    /*
     * Ensure that the key is provided, either natively, or as a cached export.
     * If not, goto legacy
     */
    tmp_keymgmt = ctx->keymgmt;
    if (ctx->pkey == NULL) {
        /*
         * Some algorithms (e.g. legacy KDFs) don't have a pkey - so we create
         * a blank one.
         */
        EVP_PKEY *pkey = EVP_PKEY_new();

        if (pkey == NULL || !EVP_PKEY_set_type_by_keymgmt(pkey, tmp_keymgmt)) {
            ERR_clear_last_mark();
            EVP_PKEY_free(pkey);
            ERR_raise(ERR_LIB_EVP, EVP_R_INITIALIZATION_ERROR);
            goto err;
        }
        provkey = pkey->keydata = evp_keymgmt_newdata(tmp_keymgmt);
        if (provkey == NULL)
            EVP_PKEY_free(pkey);
        else
            ctx->pkey = pkey;
    } else {
        provkey = evp_pkey_export_to_provider(ctx->pkey, ctx->libctx,
                                            &tmp_keymgmt, ctx->propquery);
    }
    if (provkey == NULL)
        goto legacy;
    if (!EVP_KEYMGMT_up_ref(tmp_keymgmt)) {
        ERR_clear_last_mark();
        ERR_raise(ERR_LIB_EVP, EVP_R_INITIALIZATION_ERROR);
        goto err;
    }
    EVP_KEYMGMT_free(ctx->keymgmt);
    ctx->keymgmt = tmp_keymgmt;

    if (ctx->keymgmt->query_operation_name != NULL)
        supported_exch = ctx->keymgmt->query_operation_name(OSSL_OP_KEYEXCH);

    /*
     * If we didn't get a supported exch, assume there is one with the
     * same name as the key type.
     */
    if (supported_exch == NULL)
        supported_exch = ctx->keytype;

    /*
     * Because we cleared out old ops, we shouldn't need to worry about
     * checking if exchange is already there.
     */
    exchange = EVP_KEYEXCH_fetch(ctx->libctx, supported_exch, ctx->propquery);

    if (exchange == NULL
        || (EVP_KEYMGMT_provider(ctx->keymgmt)
            != EVP_KEYEXCH_provider(exchange))) {
        /*
         * We don't need to free ctx->keymgmt here, as it's not necessarily
         * tied to this operation.  It will be freed by EVP_PKEY_CTX_free().
         */
        EVP_KEYEXCH_free(exchange);
        goto legacy;
    }

    /*
     * TODO remove this when legacy is gone
     * If we don't have the full support we need with provided methods,
     * let's go see if legacy does.
     */
    ERR_pop_to_mark();

    /* No more legacy from here down to legacy: */

    ctx->op.kex.exchange = exchange;
    ctx->op.kex.exchprovctx = exchange->newctx(ossl_provider_ctx(exchange->prov));
    if (ctx->op.kex.exchprovctx == NULL) {
        /* The provider key can stay in the cache */
        EVPerr(0, EVP_R_INITIALIZATION_ERROR);
        goto err;
    }
    ret = exchange->init(ctx->op.kex.exchprovctx, provkey);

    return ret ? 1 : 0;
 err:
    evp_pkey_ctx_free_old_ops(ctx);
    ctx->operation = EVP_PKEY_OP_UNDEFINED;
    return 0;

 legacy:
    /*
     * TODO remove this when legacy is gone
     * If we don't have the full support we need with provided methods,
     * let's go see if legacy does.
     */
    ERR_pop_to_mark();

#ifdef FIPS_MODULE
    return 0;
#else
    if (ctx->pmeth == NULL || ctx->pmeth->derive == NULL) {
        EVPerr(0, EVP_R_OPERATION_NOT_SUPPORTED_FOR_THIS_KEYTYPE);
        return -2;
    }

    if (ctx->pmeth->derive_init == NULL)
        return 1;
    ret = ctx->pmeth->derive_init(ctx);
    if (ret <= 0)
        ctx->operation = EVP_PKEY_OP_UNDEFINED;
    return ret;
#endif
}

int EVP_PKEY_derive_set_peer(EVP_PKEY_CTX *ctx, EVP_PKEY *peer)
{
    int ret = 0;
    void *provkey = NULL;

    if (ctx == NULL) {
        EVPerr(EVP_F_EVP_PKEY_DERIVE_SET_PEER,
               EVP_R_OPERATION_NOT_SUPPORTED_FOR_THIS_KEYTYPE);
        return -2;
    }

    if (!EVP_PKEY_CTX_IS_DERIVE_OP(ctx) || ctx->op.kex.exchprovctx == NULL)
        goto legacy;

    if (ctx->op.kex.exchange->set_peer == NULL) {
        EVPerr(EVP_F_EVP_PKEY_DERIVE_SET_PEER,
               EVP_R_OPERATION_NOT_SUPPORTED_FOR_THIS_KEYTYPE);
        return -2;
    }

    provkey = evp_pkey_export_to_provider(peer, ctx->libctx, &ctx->keymgmt,
                                          ctx->propquery);
    /*
     * If making the key provided wasn't possible, legacy may be able to pick
     * it up
     */
    if (provkey == NULL)
        goto legacy;
    return ctx->op.kex.exchange->set_peer(ctx->op.kex.exchprovctx, provkey);

 legacy:
#ifdef FIPS_MODULE
    return ret;
#else
    /*
     * TODO(3.0) investigate the case where the operation is deemed legacy,
     * but the given peer key is provider only.
     */
    if (ctx->pmeth == NULL
        || !(ctx->pmeth->derive != NULL
             || ctx->pmeth->encrypt != NULL
             || ctx->pmeth->decrypt != NULL)
        || ctx->pmeth->ctrl == NULL) {
        EVPerr(EVP_F_EVP_PKEY_DERIVE_SET_PEER,
               EVP_R_OPERATION_NOT_SUPPORTED_FOR_THIS_KEYTYPE);
        return -2;
    }
    if (ctx->operation != EVP_PKEY_OP_DERIVE
        && ctx->operation != EVP_PKEY_OP_ENCRYPT
        && ctx->operation != EVP_PKEY_OP_DECRYPT) {
        EVPerr(EVP_F_EVP_PKEY_DERIVE_SET_PEER,
               EVP_R_OPERATON_NOT_INITIALIZED);
        return -1;
    }

    ret = ctx->pmeth->ctrl(ctx, EVP_PKEY_CTRL_PEER_KEY, 0, peer);

    if (ret <= 0)
        return ret;

    if (ret == 2)
        return 1;

    if (ctx->pkey == NULL) {
        EVPerr(EVP_F_EVP_PKEY_DERIVE_SET_PEER, EVP_R_NO_KEY_SET);
        return -1;
    }

    if (ctx->pkey->type != peer->type) {
        EVPerr(EVP_F_EVP_PKEY_DERIVE_SET_PEER, EVP_R_DIFFERENT_KEY_TYPES);
        return -1;
    }

    /*
     * For clarity.  The error is if parameters in peer are
     * present (!missing) but don't match.  EVP_PKEY_parameters_eq may return
     * 1 (match), 0 (don't match) and -2 (comparison is not defined).  -1
     * (different key types) is impossible here because it is checked earlier.
     * -2 is OK for us here, as well as 1, so we can check for 0 only.
     */
    if (!EVP_PKEY_missing_parameters(peer) &&
        !EVP_PKEY_parameters_eq(ctx->pkey, peer)) {
        EVPerr(EVP_F_EVP_PKEY_DERIVE_SET_PEER, EVP_R_DIFFERENT_PARAMETERS);
        return -1;
    }

    EVP_PKEY_free(ctx->peerkey);
    ctx->peerkey = peer;

    ret = ctx->pmeth->ctrl(ctx, EVP_PKEY_CTRL_PEER_KEY, 1, peer);

    if (ret <= 0) {
        ctx->peerkey = NULL;
        return ret;
    }

    EVP_PKEY_up_ref(peer);
    return 1;
#endif
}

int EVP_PKEY_derive(EVP_PKEY_CTX *ctx, unsigned char *key, size_t *pkeylen)
{
    int ret;

    if (ctx == NULL) {
        EVPerr(EVP_F_EVP_PKEY_DERIVE,
               EVP_R_OPERATION_NOT_SUPPORTED_FOR_THIS_KEYTYPE);
        return -2;
    }

    if (!EVP_PKEY_CTX_IS_DERIVE_OP(ctx)) {
        EVPerr(EVP_F_EVP_PKEY_DERIVE, EVP_R_OPERATON_NOT_INITIALIZED);
        return -1;
    }

    if (ctx->op.kex.exchprovctx == NULL)
        goto legacy;

    ret = ctx->op.kex.exchange->derive(ctx->op.kex.exchprovctx, key, pkeylen,
                                       SIZE_MAX);

    return ret;
 legacy:
    if (ctx ==  NULL || ctx->pmeth == NULL || ctx->pmeth->derive == NULL) {
        EVPerr(EVP_F_EVP_PKEY_DERIVE,
               EVP_R_OPERATION_NOT_SUPPORTED_FOR_THIS_KEYTYPE);
        return -2;
    }

    M_check_autoarg(ctx, key, pkeylen, EVP_F_EVP_PKEY_DERIVE)
        return ctx->pmeth->derive(ctx, key, pkeylen);
}

int EVP_KEYEXCH_number(const EVP_KEYEXCH *keyexch)
{
    return keyexch->name_id;
}

int EVP_KEYEXCH_is_a(const EVP_KEYEXCH *keyexch, const char *name)
{
    return evp_is_a(keyexch->prov, keyexch->name_id, NULL, name);
}

void EVP_KEYEXCH_do_all_provided(OPENSSL_CTX *libctx,
                                 void (*fn)(EVP_KEYEXCH *keyexch, void *arg),
                                 void *arg)
{
    evp_generic_do_all(libctx, OSSL_OP_KEYEXCH,
                       (void (*)(void *, void *))fn, arg,
                       evp_keyexch_from_dispatch,
                       (void (*)(void *))EVP_KEYEXCH_free);
}

void EVP_KEYEXCH_names_do_all(const EVP_KEYEXCH *keyexch,
                              void (*fn)(const char *name, void *data),
                              void *data)
{
    if (keyexch->prov != NULL)
        evp_names_do_all(keyexch->prov, keyexch->name_id, fn, data);
}

const OSSL_PARAM *EVP_KEYEXCH_gettable_ctx_params(const EVP_KEYEXCH *keyexch)
{
    void *provctx;

    if (keyexch == NULL || keyexch->gettable_ctx_params == NULL)
        return NULL;

    provctx = ossl_provider_ctx(EVP_KEYEXCH_provider(keyexch));
    return keyexch->gettable_ctx_params(provctx);
}

const OSSL_PARAM *EVP_KEYEXCH_settable_ctx_params(const EVP_KEYEXCH *keyexch)
{
    void *provctx;

    if (keyexch == NULL || keyexch->settable_ctx_params == NULL)
        return NULL;
    provctx = ossl_provider_ctx(EVP_KEYEXCH_provider(keyexch));
    return keyexch->settable_ctx_params(provctx);
}
