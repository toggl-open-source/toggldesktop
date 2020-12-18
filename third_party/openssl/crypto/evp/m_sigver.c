/*
 * Copyright 2006-2020 The OpenSSL Project Authors. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://www.openssl.org/source/license.html
 */

#include <stdio.h>
#include "internal/cryptlib.h"
#include <openssl/evp.h>
#include <openssl/objects.h>
#include <openssl/x509.h>
#include "crypto/evp.h"
#include "internal/provider.h"
#include "evp_local.h"

#ifndef FIPS_MODULE

static int update(EVP_MD_CTX *ctx, const void *data, size_t datalen)
{
    EVPerr(EVP_F_UPDATE, EVP_R_ONLY_ONESHOT_SUPPORTED);
    return 0;
}

/*
 * If we get the "NULL" md then the name comes back as "UNDEF". We want to use
 * NULL for this.
 */
static const char *canon_mdname(const char *mdname)
{
    if (mdname != NULL && strcmp(mdname, "UNDEF") == 0)
        return NULL;

    return mdname;
}

static int do_sigver_init(EVP_MD_CTX *ctx, EVP_PKEY_CTX **pctx,
                          const EVP_MD *type, const char *mdname,
                          OPENSSL_CTX *libctx, const char *props,
                          ENGINE *e, EVP_PKEY *pkey, int ver)
{
    EVP_PKEY_CTX *locpctx = NULL;
    EVP_SIGNATURE *signature = NULL;
    EVP_KEYMGMT *tmp_keymgmt = NULL;
    const char *supported_sig = NULL;
    char locmdname[80] = "";     /* 80 chars should be enough */
    void *provkey = NULL;
    int ret;

    if (ctx->provctx != NULL) {
        if (!ossl_assert(ctx->digest != NULL)) {
            ERR_raise(ERR_LIB_EVP, EVP_R_INITIALIZATION_ERROR);
            return 0;
        }
        if (ctx->digest->freectx != NULL)
            ctx->digest->freectx(ctx->provctx);
        ctx->provctx = NULL;
    }

    if (ctx->pctx == NULL) {
        if (libctx != NULL)
            ctx->pctx = EVP_PKEY_CTX_new_from_pkey(libctx, pkey, props);
        else
            ctx->pctx = EVP_PKEY_CTX_new(pkey, e);
    }
    if (ctx->pctx == NULL)
        return 0;

    locpctx = ctx->pctx;
    evp_pkey_ctx_free_old_ops(locpctx);

    if (props == NULL)
        props = locpctx->propquery;

    /*
     * TODO when we stop falling back to legacy, this and the ERR_pop_to_mark()
     * calls can be removed.
     */
    ERR_set_mark();

    if (locpctx->engine != NULL || locpctx->keytype == NULL)
        goto legacy;

    /*
     * Ensure that the key is provided, either natively, or as a cached export.
     */
    tmp_keymgmt = locpctx->keymgmt;
    provkey = evp_pkey_export_to_provider(locpctx->pkey, locpctx->libctx,
                                          &tmp_keymgmt, locpctx->propquery);
    if (provkey == NULL) {
        /*
         * If we couldn't find a keymgmt at all try legacy.
         * TODO(3.0): Once all legacy algorithms (SM2, HMAC etc) have provider
         * based implementations this fallback shouldn't be necessary. Either
         * we have an ENGINE based implementation (in which case we should have
         * already fallen back in the test above here), or we don't have the
         * provider based implementation loaded (in which case this is an
         * application config error)
         */
        if (locpctx->keymgmt == NULL)
            goto legacy;
        ERR_raise(ERR_LIB_EVP, EVP_R_INITIALIZATION_ERROR);
        goto err;
    }
    if (!EVP_KEYMGMT_up_ref(tmp_keymgmt)) {
        ERR_clear_last_mark();
        ERR_raise(ERR_LIB_EVP, EVP_R_INITIALIZATION_ERROR);
        goto err;
    }
    EVP_KEYMGMT_free(locpctx->keymgmt);
    locpctx->keymgmt = tmp_keymgmt;

    if (locpctx->keymgmt->query_operation_name != NULL)
        supported_sig =
            locpctx->keymgmt->query_operation_name(OSSL_OP_SIGNATURE);

    /*
     * If we didn't get a supported sig, assume there is one with the
     * same name as the key type.
     */
    if (supported_sig == NULL)
        supported_sig = locpctx->keytype;

    /*
     * Because we cleared out old ops, we shouldn't need to worry about
     * checking if signature is already there.
     */
    signature = EVP_SIGNATURE_fetch(locpctx->libctx, supported_sig,
                                    locpctx->propquery);

    if (signature == NULL
        || (EVP_KEYMGMT_provider(locpctx->keymgmt)
            != EVP_SIGNATURE_provider(signature))) {
        /*
         * We don't need to free ctx->keymgmt here, as it's not necessarily
         * tied to this operation.  It will be freed by EVP_PKEY_CTX_free().
         */
        EVP_SIGNATURE_free(signature);
        goto legacy;
    }

    /*
     * TODO remove this when legacy is gone
     * If we don't have the full support we need with provided methods,
     * let's go see if legacy does.
     */
    ERR_pop_to_mark();

    /* No more legacy from here down to legacy: */

    if (pctx != NULL)
        *pctx = locpctx;

    locpctx->op.sig.signature = signature;
    locpctx->operation = ver ? EVP_PKEY_OP_VERIFYCTX
                             : EVP_PKEY_OP_SIGNCTX;
    locpctx->op.sig.sigprovctx
        = signature->newctx(ossl_provider_ctx(signature->prov), props);
    if (locpctx->op.sig.sigprovctx == NULL) {
        ERR_raise(ERR_LIB_EVP,  EVP_R_INITIALIZATION_ERROR);
        goto err;
    }
    if (type != NULL) {
        ctx->reqdigest = type;
        if (mdname == NULL)
            mdname = canon_mdname(EVP_MD_name(type));
    } else {
        if (mdname == NULL) {
            if (evp_keymgmt_util_get_deflt_digest_name(tmp_keymgmt, provkey,
                                                       locmdname,
                                                       sizeof(locmdname)) > 0) {
                mdname = canon_mdname(locmdname);
            }
        }

        if (mdname != NULL) {
            /*
             * We're about to get a new digest so clear anything associated with
             * an old digest.
             */
            evp_md_ctx_clear_digest(ctx, 1);

            /*
             * This might be requested by a later call to EVP_MD_CTX_md().
             * In that case the "explicit fetch" rules apply for that
             * function (as per man pages), i.e. the ref count is not updated
             * so the EVP_MD should not be used beyound the lifetime of the
             * EVP_MD_CTX.
             */
            ctx->digest = ctx->reqdigest = ctx->fetched_digest =
                EVP_MD_fetch(locpctx->libctx, mdname, props);
            if (ctx->digest == NULL) {
                ERR_raise(ERR_LIB_EVP, EVP_R_INITIALIZATION_ERROR);
                goto err;
            }
        }
    }

    if (ver) {
        if (signature->digest_verify_init == NULL) {
            ERR_raise(ERR_LIB_EVP, EVP_R_INITIALIZATION_ERROR);
            goto err;
        }
        ret = signature->digest_verify_init(locpctx->op.sig.sigprovctx,
                                            mdname, provkey);
    } else {
        if (signature->digest_sign_init == NULL) {
            ERR_raise(ERR_LIB_EVP, EVP_R_INITIALIZATION_ERROR);
            goto err;
        }
        ret = signature->digest_sign_init(locpctx->op.sig.sigprovctx,
                                          mdname, provkey);
    }

    goto end;

 err:
    evp_pkey_ctx_free_old_ops(locpctx);
    locpctx->operation = EVP_PKEY_OP_UNDEFINED;
    return 0;

 legacy:
    /*
     * TODO remove this when legacy is gone
     * If we don't have the full support we need with provided methods,
     * let's go see if legacy does.
     */
    ERR_pop_to_mark();

    if (type == NULL && mdname != NULL)
        type = evp_get_digestbyname_ex(locpctx->libctx, mdname);

    if (ctx->pctx->pmeth == NULL) {
        EVPerr(0, EVP_R_OPERATION_NOT_SUPPORTED_FOR_THIS_KEYTYPE);
        return 0;
    }

    if (!(ctx->pctx->pmeth->flags & EVP_PKEY_FLAG_SIGCTX_CUSTOM)) {

        if (type == NULL) {
            int def_nid;
            if (EVP_PKEY_get_default_digest_nid(pkey, &def_nid) > 0)
                type = EVP_get_digestbynid(def_nid);
        }

        if (type == NULL) {
            EVPerr(EVP_F_DO_SIGVER_INIT, EVP_R_NO_DEFAULT_DIGEST);
            return 0;
        }
    }

    if (ver) {
        if (ctx->pctx->pmeth->verifyctx_init) {
            if (ctx->pctx->pmeth->verifyctx_init(ctx->pctx, ctx) <= 0)
                return 0;
            ctx->pctx->operation = EVP_PKEY_OP_VERIFYCTX;
        } else if (ctx->pctx->pmeth->digestverify != 0) {
            ctx->pctx->operation = EVP_PKEY_OP_VERIFY;
            ctx->update = update;
        } else if (EVP_PKEY_verify_init(ctx->pctx) <= 0) {
            return 0;
        }
    } else {
        if (ctx->pctx->pmeth->signctx_init) {
            if (ctx->pctx->pmeth->signctx_init(ctx->pctx, ctx) <= 0)
                return 0;
            ctx->pctx->operation = EVP_PKEY_OP_SIGNCTX;
        } else if (ctx->pctx->pmeth->digestsign != 0) {
            ctx->pctx->operation = EVP_PKEY_OP_SIGN;
            ctx->update = update;
        } else if (EVP_PKEY_sign_init(ctx->pctx) <= 0) {
            return 0;
        }
    }
    if (EVP_PKEY_CTX_set_signature_md(ctx->pctx, type) <= 0)
        return 0;
    if (pctx)
        *pctx = ctx->pctx;
    if (ctx->pctx->pmeth->flags & EVP_PKEY_FLAG_SIGCTX_CUSTOM)
        return 1;
    if (!EVP_DigestInit_ex(ctx, type, e))
        return 0;
    /*
     * This indicates the current algorithm requires
     * special treatment before hashing the tbs-message.
     */
    ctx->pctx->flag_call_digest_custom = 0;
    if (ctx->pctx->pmeth->digest_custom != NULL)
        ctx->pctx->flag_call_digest_custom = 1;

    ret = 1;

 end:
#ifndef FIPS_MODULE
    if (ret > 0)
        ret = evp_pkey_ctx_use_cached_data(locpctx);
#endif

    return ret > 0 ? 1 : 0;
}

int EVP_DigestSignInit_with_libctx(EVP_MD_CTX *ctx, EVP_PKEY_CTX **pctx,
                                   const char *mdname,
                                   OPENSSL_CTX *libctx, const char *props,
                                   EVP_PKEY *pkey)
{
    return do_sigver_init(ctx, pctx, NULL, mdname, libctx, props, NULL, pkey, 0);
}

int EVP_DigestSignInit(EVP_MD_CTX *ctx, EVP_PKEY_CTX **pctx,
                       const EVP_MD *type, ENGINE *e, EVP_PKEY *pkey)
{
    return do_sigver_init(ctx, pctx, type, NULL, NULL, NULL, e, pkey, 0);
}

int EVP_DigestVerifyInit_with_libctx(EVP_MD_CTX *ctx, EVP_PKEY_CTX **pctx,
                                     const char *mdname,
                                     OPENSSL_CTX *libctx, const char *props,
                                     EVP_PKEY *pkey)
{
    return do_sigver_init(ctx, pctx, NULL, mdname, libctx, props, NULL, pkey, 1);
}

int EVP_DigestVerifyInit(EVP_MD_CTX *ctx, EVP_PKEY_CTX **pctx,
                         const EVP_MD *type, ENGINE *e, EVP_PKEY *pkey)
{
    return do_sigver_init(ctx, pctx, type, NULL, NULL, NULL, e, pkey, 1);
}
#endif /* FIPS_MDOE */

int EVP_DigestSignUpdate(EVP_MD_CTX *ctx, const void *data, size_t dsize)
{
    EVP_PKEY_CTX *pctx = ctx->pctx;

    if (pctx == NULL
            || pctx->operation != EVP_PKEY_OP_SIGNCTX
            || pctx->op.sig.sigprovctx == NULL
            || pctx->op.sig.signature == NULL)
        goto legacy;

    if (pctx->op.sig.signature->digest_sign_update == NULL) {
        ERR_raise(ERR_LIB_EVP, ERR_R_SHOULD_NOT_HAVE_BEEN_CALLED);
        return 0;
    }

    return pctx->op.sig.signature->digest_sign_update(pctx->op.sig.sigprovctx,
                                                      data, dsize);

 legacy:
    if (pctx != NULL) {
        /* do_sigver_init() checked that |digest_custom| is non-NULL */
        if (pctx->flag_call_digest_custom
            && !ctx->pctx->pmeth->digest_custom(ctx->pctx, ctx))
            return 0;
        pctx->flag_call_digest_custom = 0;
    }

    return EVP_DigestUpdate(ctx, data, dsize);
}

int EVP_DigestVerifyUpdate(EVP_MD_CTX *ctx, const void *data, size_t dsize)
{
    EVP_PKEY_CTX *pctx = ctx->pctx;

    if (pctx == NULL
            || pctx->operation != EVP_PKEY_OP_VERIFYCTX
            || pctx->op.sig.sigprovctx == NULL
            || pctx->op.sig.signature == NULL)
        goto legacy;

    if (pctx->op.sig.signature->digest_verify_update == NULL) {
        ERR_raise(ERR_LIB_EVP, ERR_R_SHOULD_NOT_HAVE_BEEN_CALLED);
        return 0;
    }

    return pctx->op.sig.signature->digest_verify_update(pctx->op.sig.sigprovctx,
                                                        data, dsize);

 legacy:
    if (pctx != NULL) {
        /* do_sigver_init() checked that |digest_custom| is non-NULL */
        if (pctx->flag_call_digest_custom
            && !ctx->pctx->pmeth->digest_custom(ctx->pctx, ctx))
            return 0;
        pctx->flag_call_digest_custom = 0;
    }

    return EVP_DigestUpdate(ctx, data, dsize);
}

#ifndef FIPS_MODULE
int EVP_DigestSignFinal(EVP_MD_CTX *ctx, unsigned char *sigret,
                        size_t *siglen)
{
    int sctx = 0, r = 0;
    EVP_PKEY_CTX *pctx = ctx->pctx;

    if (pctx == NULL
            || pctx->operation != EVP_PKEY_OP_SIGNCTX
            || pctx->op.sig.sigprovctx == NULL
            || pctx->op.sig.signature == NULL)
        goto legacy;

    return pctx->op.sig.signature->digest_sign_final(pctx->op.sig.sigprovctx,
                                                     sigret, siglen, SIZE_MAX);

 legacy:
    if (pctx == NULL || pctx->pmeth == NULL) {
        ERR_raise(ERR_LIB_EVP, EVP_R_INITIALIZATION_ERROR);
        return 0;
    }

    /* do_sigver_init() checked that |digest_custom| is non-NULL */
    if (pctx->flag_call_digest_custom
        && !ctx->pctx->pmeth->digest_custom(ctx->pctx, ctx))
        return 0;
    pctx->flag_call_digest_custom = 0;

    if (pctx->pmeth->flags & EVP_PKEY_FLAG_SIGCTX_CUSTOM) {
        if (sigret == NULL)
            return pctx->pmeth->signctx(pctx, sigret, siglen, ctx);
        if (ctx->flags & EVP_MD_CTX_FLAG_FINALISE)
            r = pctx->pmeth->signctx(pctx, sigret, siglen, ctx);
        else {
            EVP_PKEY_CTX *dctx = EVP_PKEY_CTX_dup(pctx);

            if (dctx == NULL)
                return 0;
            r = dctx->pmeth->signctx(dctx, sigret, siglen, ctx);
            EVP_PKEY_CTX_free(dctx);
        }
        return r;
    }
    if (pctx->pmeth->signctx != NULL)
        sctx = 1;
    else
        sctx = 0;
    if (sigret != NULL) {
        unsigned char md[EVP_MAX_MD_SIZE];
        unsigned int mdlen = 0;

        if (ctx->flags & EVP_MD_CTX_FLAG_FINALISE) {
            if (sctx)
                r = pctx->pmeth->signctx(pctx, sigret, siglen, ctx);
            else
                r = EVP_DigestFinal_ex(ctx, md, &mdlen);
        } else {
            EVP_MD_CTX *tmp_ctx = EVP_MD_CTX_new();

            if (tmp_ctx == NULL)
                return 0;
            if (!EVP_MD_CTX_copy_ex(tmp_ctx, ctx)) {
                EVP_MD_CTX_free(tmp_ctx);
                return 0;
            }
            if (sctx)
                r = tmp_ctx->pctx->pmeth->signctx(tmp_ctx->pctx,
                                                  sigret, siglen, tmp_ctx);
            else
                r = EVP_DigestFinal_ex(tmp_ctx, md, &mdlen);
            EVP_MD_CTX_free(tmp_ctx);
        }
        if (sctx || !r)
            return r;
        if (EVP_PKEY_sign(pctx, sigret, siglen, md, mdlen) <= 0)
            return 0;
    } else {
        if (sctx) {
            if (pctx->pmeth->signctx(pctx, sigret, siglen, ctx) <= 0)
                return 0;
        } else {
            int s = EVP_MD_size(ctx->digest);

            if (s < 0 || EVP_PKEY_sign(pctx, sigret, siglen, NULL, s) <= 0)
                return 0;
        }
    }
    return 1;
}

int EVP_DigestSign(EVP_MD_CTX *ctx, unsigned char *sigret, size_t *siglen,
                   const unsigned char *tbs, size_t tbslen)
{
    EVP_PKEY_CTX *pctx = ctx->pctx;

    if (pctx != NULL
            && pctx->operation == EVP_PKEY_OP_SIGNCTX
            && pctx->op.sig.sigprovctx != NULL
            && pctx->op.sig.signature != NULL) {
        if (pctx->op.sig.signature->digest_sign != NULL)
            return pctx->op.sig.signature->digest_sign(pctx->op.sig.sigprovctx,
                                                       sigret, siglen, SIZE_MAX,
                                                       tbs, tbslen);
    } else {
        /* legacy */
        if (ctx->pctx->pmeth != NULL && ctx->pctx->pmeth->digestsign != NULL)
            return ctx->pctx->pmeth->digestsign(ctx, sigret, siglen, tbs, tbslen);
    }

    if (sigret != NULL && EVP_DigestSignUpdate(ctx, tbs, tbslen) <= 0)
        return 0;
    return EVP_DigestSignFinal(ctx, sigret, siglen);
}

int EVP_DigestVerifyFinal(EVP_MD_CTX *ctx, const unsigned char *sig,
                          size_t siglen)
{
    unsigned char md[EVP_MAX_MD_SIZE];
    int r = 0;
    unsigned int mdlen = 0;
    int vctx = 0;
    EVP_PKEY_CTX *pctx = ctx->pctx;

    if (pctx == NULL
            || pctx->operation != EVP_PKEY_OP_VERIFYCTX
            || pctx->op.sig.sigprovctx == NULL
            || pctx->op.sig.signature == NULL)
        goto legacy;

    return pctx->op.sig.signature->digest_verify_final(pctx->op.sig.sigprovctx,
                                                       sig, siglen);

 legacy:
    if (pctx == NULL || pctx->pmeth == NULL) {
        ERR_raise(ERR_LIB_EVP, EVP_R_INITIALIZATION_ERROR);
        return 0;
    }

    /* do_sigver_init() checked that |digest_custom| is non-NULL */
    if (pctx->flag_call_digest_custom
        && !ctx->pctx->pmeth->digest_custom(ctx->pctx, ctx))
        return 0;
    pctx->flag_call_digest_custom = 0;

    if (pctx->pmeth->verifyctx != NULL)
        vctx = 1;
    else
        vctx = 0;
    if (ctx->flags & EVP_MD_CTX_FLAG_FINALISE) {
        if (vctx)
            r = pctx->pmeth->verifyctx(pctx, sig, siglen, ctx);
        else
            r = EVP_DigestFinal_ex(ctx, md, &mdlen);
    } else {
        EVP_MD_CTX *tmp_ctx = EVP_MD_CTX_new();
        if (tmp_ctx == NULL)
            return -1;
        if (!EVP_MD_CTX_copy_ex(tmp_ctx, ctx)) {
            EVP_MD_CTX_free(tmp_ctx);
            return -1;
        }
        if (vctx)
            r = tmp_ctx->pctx->pmeth->verifyctx(tmp_ctx->pctx,
                                                sig, siglen, tmp_ctx);
        else
            r = EVP_DigestFinal_ex(tmp_ctx, md, &mdlen);
        EVP_MD_CTX_free(tmp_ctx);
    }
    if (vctx || !r)
        return r;
    return EVP_PKEY_verify(pctx, sig, siglen, md, mdlen);
}

int EVP_DigestVerify(EVP_MD_CTX *ctx, const unsigned char *sigret,
                     size_t siglen, const unsigned char *tbs, size_t tbslen)
{
    EVP_PKEY_CTX *pctx = ctx->pctx;

    if (pctx != NULL
            && pctx->operation == EVP_PKEY_OP_VERIFYCTX
            && pctx->op.sig.sigprovctx != NULL
            && pctx->op.sig.signature != NULL) {
        if (pctx->op.sig.signature->digest_verify != NULL)
            return pctx->op.sig.signature->digest_verify(pctx->op.sig.sigprovctx,
                                                         sigret, siglen,
                                                         tbs, tbslen);
    } else {
        /* legacy */
        if (ctx->pctx->pmeth != NULL && ctx->pctx->pmeth->digestverify != NULL)
            return ctx->pctx->pmeth->digestverify(ctx, sigret, siglen, tbs, tbslen);
    }

    if (EVP_DigestVerifyUpdate(ctx, tbs, tbslen) <= 0)
        return -1;
    return EVP_DigestVerifyFinal(ctx, sigret, siglen);
}
#endif /* FIPS_MODULE */
