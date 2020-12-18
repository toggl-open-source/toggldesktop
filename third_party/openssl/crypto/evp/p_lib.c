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
#include <openssl/err.h>
#include <openssl/objects.h>
#include <openssl/evp.h>
#include <openssl/x509.h>
#include <openssl/rsa.h>
#include <openssl/dsa.h>
#include <openssl/dh.h>
#include <openssl/ec.h>
#include <openssl/cmac.h>
#include <openssl/engine.h>
#include <openssl/params.h>
#include <openssl/param_build.h>
#include <openssl/encoder.h>
#include <openssl/core_names.h>

#include "crypto/asn1.h"
#include "crypto/evp.h"
#include "crypto/ecx.h"
#include "internal/evp.h"
#include "internal/provider.h"
#include "evp_local.h"

#include "crypto/ec.h"

/* TODO remove this when the EVP_PKEY_is_a() #legacy support hack is removed */
#include "e_os.h"                /* strcasecmp on Windows */

static int pkey_set_type(EVP_PKEY *pkey, ENGINE *e, int type, const char *str,
                         int len, EVP_KEYMGMT *keymgmt);
static void evp_pkey_free_it(EVP_PKEY *key);

#ifndef FIPS_MODULE

/* The type of parameters selected in key parameter functions */
# define SELECT_PARAMETERS OSSL_KEYMGMT_SELECT_DOMAIN_PARAMETERS

int EVP_PKEY_bits(const EVP_PKEY *pkey)
{
    if (pkey != NULL) {
        if (pkey->ameth == NULL)
            return pkey->cache.bits;
        else if (pkey->ameth->pkey_bits)
            return pkey->ameth->pkey_bits(pkey);
    }
    return 0;
}

int EVP_PKEY_security_bits(const EVP_PKEY *pkey)
{
    if (pkey == NULL)
        return 0;
    if (pkey->ameth == NULL)
        return pkey->cache.security_bits;
    if (pkey->ameth->pkey_security_bits == NULL)
        return -2;
    return pkey->ameth->pkey_security_bits(pkey);
}

int EVP_PKEY_save_parameters(EVP_PKEY *pkey, int mode)
{
# ifndef OPENSSL_NO_DSA
    if (pkey->type == EVP_PKEY_DSA) {
        int ret = pkey->save_parameters;

        if (mode >= 0)
            pkey->save_parameters = mode;
        return ret;
    }
# endif
# ifndef OPENSSL_NO_EC
    if (pkey->type == EVP_PKEY_EC) {
        int ret = pkey->save_parameters;

        if (mode >= 0)
            pkey->save_parameters = mode;
        return ret;
    }
# endif
    return 0;
}

int EVP_PKEY_set_ex_data(EVP_PKEY *key, int idx, void *arg)
{
    return CRYPTO_set_ex_data(&key->ex_data, idx, arg);
}

void *EVP_PKEY_get_ex_data(const EVP_PKEY *key, int idx)
{
    return CRYPTO_get_ex_data(&key->ex_data, idx);
}

int EVP_PKEY_copy_parameters(EVP_PKEY *to, const EVP_PKEY *from)
{
    /*
     * TODO: clean up legacy stuff from this function when legacy support
     * is gone.
     */

    /*
     * If |to| is a legacy key and |from| isn't, we must downgrade |from|.
     * If that fails, this function fails.
     */
    if (evp_pkey_is_legacy(to) && evp_pkey_is_provided(from))
        if (!evp_pkey_downgrade((EVP_PKEY *)from))
            return 0;

    /*
     * Make sure |to| is typed.  Content is less important at this early
     * stage.
     *
     * 1.  If |to| is untyped, assign |from|'s key type to it.
     * 2.  If |to| contains a legacy key, compare its |type| to |from|'s.
     *     (|from| was already downgraded above)
     *
     * If |to| is a provided key, there's nothing more to do here, functions
     * like evp_keymgmt_util_copy() and evp_pkey_export_to_provider() called
     * further down help us find out if they are the same or not.
     */
    if (evp_pkey_is_blank(to)) {
        if (evp_pkey_is_legacy(from)) {
            if (EVP_PKEY_set_type(to, from->type) == 0)
                return 0;
        } else {
            if (EVP_PKEY_set_type_by_keymgmt(to, from->keymgmt) == 0)
                return 0;
        }
    } else if (evp_pkey_is_legacy(to)) {
        if (to->type != from->type) {
            EVPerr(EVP_F_EVP_PKEY_COPY_PARAMETERS, EVP_R_DIFFERENT_KEY_TYPES);
            goto err;
        }
    }

    if (EVP_PKEY_missing_parameters(from)) {
        EVPerr(EVP_F_EVP_PKEY_COPY_PARAMETERS, EVP_R_MISSING_PARAMETERS);
        goto err;
    }

    if (!EVP_PKEY_missing_parameters(to)) {
        if (EVP_PKEY_parameters_eq(to, from) == 1)
            return 1;
        EVPerr(EVP_F_EVP_PKEY_COPY_PARAMETERS, EVP_R_DIFFERENT_PARAMETERS);
        return 0;
    }

    /* For purely provided keys, we just call the keymgmt utility */
    if (to->keymgmt != NULL && from->keymgmt != NULL)
        return evp_keymgmt_util_copy(to, (EVP_PKEY *)from, SELECT_PARAMETERS);

    /*
     * If |to| is provided, we know that |from| is legacy at this point.
     * Try exporting |from| to |to|'s keymgmt, then use evp_keymgmt_copy()
     * to copy the appropriate data to |to|'s keydata.
     */
    if (to->keymgmt != NULL) {
        EVP_KEYMGMT *to_keymgmt = to->keymgmt;
        void *from_keydata =
            evp_pkey_export_to_provider((EVP_PKEY *)from, NULL, &to_keymgmt,
                                        NULL);

        /*
         * If we get a NULL, it could be an internal error, or it could be
         * that there's a key mismatch.  We're pretending the latter...
         */
        if (from_keydata == NULL) {
            ERR_raise(ERR_LIB_EVP, EVP_R_DIFFERENT_KEY_TYPES);
            return 0;
        }
        return evp_keymgmt_copy(to->keymgmt, to->keydata, from_keydata,
                                SELECT_PARAMETERS);
    }

    /* Both keys are legacy */
    if (from->ameth != NULL && from->ameth->param_copy != NULL)
        return from->ameth->param_copy(to, from);
 err:
    return 0;
}

int EVP_PKEY_missing_parameters(const EVP_PKEY *pkey)
{
    if (pkey != NULL) {
        if (pkey->keymgmt != NULL)
            return !evp_keymgmt_util_has((EVP_PKEY *)pkey, SELECT_PARAMETERS);
        else if (pkey->ameth != NULL && pkey->ameth->param_missing != NULL)
            return pkey->ameth->param_missing(pkey);
    }
    return 0;
}

/*
 * This function is called for any mixture of keys except pure legacy pair.
 * TODO When legacy keys are gone, we replace a call to this functions with
 * a call to evp_keymgmt_util_match().
 */
static int evp_pkey_cmp_any(const EVP_PKEY *a, const EVP_PKEY *b,
                            int selection)
{
    EVP_KEYMGMT *keymgmt1 = NULL, *keymgmt2 = NULL;
    void *keydata1 = NULL, *keydata2 = NULL, *tmp_keydata = NULL;

    /* If none of them are provided, this function shouldn't have been called */
    if (!ossl_assert(evp_pkey_is_provided(a) || evp_pkey_is_provided(b)))
        return -2;

    /* For purely provided keys, we just call the keymgmt utility */
    if (evp_pkey_is_provided(a) && evp_pkey_is_provided(b))
        return evp_keymgmt_util_match((EVP_PKEY *)a, (EVP_PKEY *)b, selection);

    /*
     * At this point, one of them is provided, the other not.  This allows
     * us to compare types using legacy NIDs.
     */
    if (evp_pkey_is_legacy(a)
        && !EVP_KEYMGMT_is_a(b->keymgmt, OBJ_nid2sn(a->type)))
        return -1;               /* not the same key type */
    if (evp_pkey_is_legacy(b)
        && !EVP_KEYMGMT_is_a(a->keymgmt, OBJ_nid2sn(b->type)))
        return -1;               /* not the same key type */

    /*
     * We've determined that they both are the same keytype, so the next
     * step is to do a bit of cross export to ensure we have keydata for
     * both keys in the same keymgmt.
     */
    keymgmt1 = a->keymgmt;
    keydata1 = a->keydata;
    keymgmt2 = b->keymgmt;
    keydata2 = b->keydata;

    if (keymgmt2 != NULL && keymgmt2->match != NULL) {
        tmp_keydata =
            evp_pkey_export_to_provider((EVP_PKEY *)a, NULL, &keymgmt2, NULL);
        if (tmp_keydata != NULL) {
            keymgmt1 = keymgmt2;
            keydata1 = tmp_keydata;
        }
    }
    if (tmp_keydata == NULL && keymgmt1 != NULL && keymgmt1->match != NULL) {
        tmp_keydata =
            evp_pkey_export_to_provider((EVP_PKEY *)b, NULL, &keymgmt1, NULL);
        if (tmp_keydata != NULL) {
            keymgmt2 = keymgmt1;
            keydata2 = tmp_keydata;
        }
    }

    /* If we still don't have matching keymgmt implementations, we give up */
    if (keymgmt1 != keymgmt2)
        return -2;

    /* If the keymgmt implementations are NULL, the export failed */
    if (keymgmt1 == NULL)
        return -2;

    return evp_keymgmt_match(keymgmt1, keydata1, keydata2, selection);
}

int EVP_PKEY_cmp_parameters(const EVP_PKEY *a, const EVP_PKEY *b)
{
    return EVP_PKEY_parameters_eq(a, b);
}

int EVP_PKEY_parameters_eq(const EVP_PKEY *a, const EVP_PKEY *b)
{
    /*
     * TODO: clean up legacy stuff from this function when legacy support
     * is gone.
     */

    if (a->keymgmt != NULL || b->keymgmt != NULL)
        return evp_pkey_cmp_any(a, b, SELECT_PARAMETERS);

    /* All legacy keys */
    if (a->type != b->type)
        return -1;
    if (a->ameth != NULL && a->ameth->param_cmp != NULL)
        return a->ameth->param_cmp(a, b);
    return -2;
}

int EVP_PKEY_cmp(const EVP_PKEY *a, const EVP_PKEY *b)
{
    return EVP_PKEY_eq(a, b);
}

int EVP_PKEY_eq(const EVP_PKEY *a, const EVP_PKEY *b)
{
    /*
     * TODO: clean up legacy stuff from this function when legacy support
     * is gone.
     */

    if (a->keymgmt != NULL || b->keymgmt != NULL)
        return evp_pkey_cmp_any(a, b, (SELECT_PARAMETERS
                                       | OSSL_KEYMGMT_SELECT_PUBLIC_KEY));

    /* All legacy keys */
    if (a->type != b->type)
        return -1;

    if (a->ameth != NULL) {
        int ret;
        /* Compare parameters if the algorithm has them */
        if (a->ameth->param_cmp != NULL) {
            ret = a->ameth->param_cmp(a, b);
            if (ret <= 0)
                return ret;
        }

        if (a->ameth->pub_cmp != NULL)
            return a->ameth->pub_cmp(a, b);
    }

    return -2;
}


static EVP_PKEY *new_raw_key_int(OPENSSL_CTX *libctx,
                                 const char *strtype,
                                 const char *propq,
                                 int nidtype,
                                 ENGINE *e,
                                 const unsigned char *key,
                                 size_t len,
                                 int key_is_priv)
{
    EVP_PKEY *pkey = NULL;
    EVP_PKEY_CTX *ctx = NULL;
    const EVP_PKEY_ASN1_METHOD *ameth = NULL;
    int result = 0;

# ifndef OPENSSL_NO_ENGINE
    /* Check if there is an Engine for this type */
    if (e == NULL) {
        ENGINE *tmpe = NULL;

        if (strtype != NULL)
            ameth = EVP_PKEY_asn1_find_str(&tmpe, strtype, -1);
        else if (nidtype != EVP_PKEY_NONE)
            ameth = EVP_PKEY_asn1_find(&tmpe, nidtype);

        /* If tmpe is NULL then no engine is claiming to support this type */
        if (tmpe == NULL)
            ameth = NULL;

        ENGINE_finish(tmpe);
    }
# endif

    if (e == NULL && ameth == NULL) {
        /*
         * No engine is claiming to support this type, so lets see if we have
         * a provider.
         */
        ctx = EVP_PKEY_CTX_new_from_name(libctx,
                                         strtype != NULL ? strtype
                                                         : OBJ_nid2sn(nidtype),
                                         propq);
        if (ctx == NULL)
            goto err;
        /* May fail if no provider available */
        ERR_set_mark();
        if (EVP_PKEY_key_fromdata_init(ctx) == 1) {
            OSSL_PARAM params[] = { OSSL_PARAM_END, OSSL_PARAM_END };

            ERR_clear_last_mark();
            params[0] = OSSL_PARAM_construct_octet_string(
                            key_is_priv ? OSSL_PKEY_PARAM_PRIV_KEY
                                        : OSSL_PKEY_PARAM_PUB_KEY,
                            (void *)key, len);

            if (EVP_PKEY_fromdata(ctx, &pkey, params) != 1) {
                EVPerr(0, EVP_R_KEY_SETUP_FAILED);
                goto err;
            }

            EVP_PKEY_CTX_free(ctx);

            return pkey;
        }
        ERR_pop_to_mark();
        /* else not supported so fallback to legacy */
    }

    /* Legacy code path */

    pkey = EVP_PKEY_new();
    if (pkey == NULL) {
        EVPerr(0, ERR_R_MALLOC_FAILURE);
        goto err;
    }

    if (!pkey_set_type(pkey, e, nidtype, strtype, -1, NULL)) {
        /* EVPerr already called */
        goto err;
    }

    if (!ossl_assert(pkey->ameth != NULL))
        goto err;

    if (key_is_priv) {
        if (pkey->ameth->set_priv_key == NULL) {
            EVPerr(0, EVP_R_OPERATION_NOT_SUPPORTED_FOR_THIS_KEYTYPE);
            goto err;
        }

        if (!pkey->ameth->set_priv_key(pkey, key, len)) {
            EVPerr(0, EVP_R_KEY_SETUP_FAILED);
            goto err;
        }
    } else {
        if (pkey->ameth->set_pub_key == NULL) {
            EVPerr(0, EVP_R_OPERATION_NOT_SUPPORTED_FOR_THIS_KEYTYPE);
            goto err;
        }

        if (!pkey->ameth->set_pub_key(pkey, key, len)) {
            EVPerr(0, EVP_R_KEY_SETUP_FAILED);
            goto err;
        }
    }

    result = 1;
 err:
    if (!result) {
        EVP_PKEY_free(pkey);
        pkey = NULL;
    }
    EVP_PKEY_CTX_free(ctx);
    return pkey;
}

EVP_PKEY *EVP_PKEY_new_raw_private_key_with_libctx(OPENSSL_CTX *libctx,
                                                   const char *keytype,
                                                   const char *propq,
                                                   const unsigned char *priv,
                                                   size_t len)
{
    return new_raw_key_int(libctx, keytype, propq, EVP_PKEY_NONE, NULL, priv,
                           len, 1);
}

EVP_PKEY *EVP_PKEY_new_raw_private_key(int type, ENGINE *e,
                                       const unsigned char *priv,
                                       size_t len)
{
    return new_raw_key_int(NULL, NULL, NULL, type, e, priv, len, 1);
}

EVP_PKEY *EVP_PKEY_new_raw_public_key_with_libctx(OPENSSL_CTX *libctx,
                                                  const char *keytype,
                                                  const char *propq,
                                                  const unsigned char *pub,
                                                  size_t len)
{
    return new_raw_key_int(libctx, keytype, propq, EVP_PKEY_NONE, NULL, pub,
                           len, 0);
}

EVP_PKEY *EVP_PKEY_new_raw_public_key(int type, ENGINE *e,
                                      const unsigned char *pub,
                                      size_t len)
{
    return new_raw_key_int(NULL, NULL, NULL, type, e, pub, len, 0);
}

struct raw_key_details_st
{
    unsigned char **key;
    size_t *len;
    int selection;
};

static OSSL_CALLBACK get_raw_key_details;
static int get_raw_key_details(const OSSL_PARAM params[], void *arg)
{
    const OSSL_PARAM *p = NULL;
    struct raw_key_details_st *raw_key = arg;

    if (raw_key->selection == OSSL_KEYMGMT_SELECT_PRIVATE_KEY) {
        if ((p = OSSL_PARAM_locate_const(params, OSSL_PKEY_PARAM_PRIV_KEY))
                != NULL)
            return OSSL_PARAM_get_octet_string(p, (void **)raw_key->key,
                                               SIZE_MAX, raw_key->len);
    } else if (raw_key->selection == OSSL_KEYMGMT_SELECT_PUBLIC_KEY) {
        if ((p = OSSL_PARAM_locate_const(params, OSSL_PKEY_PARAM_PUB_KEY))
                != NULL)
            return OSSL_PARAM_get_octet_string(p, (void **)raw_key->key,
                                               SIZE_MAX, raw_key->len);
    }

    return 0;
}

int EVP_PKEY_get_raw_private_key(const EVP_PKEY *pkey, unsigned char *priv,
                                 size_t *len)
{
    if (pkey->keymgmt != NULL) {
        struct raw_key_details_st raw_key;

        raw_key.key = priv == NULL ? NULL : &priv;
        raw_key.len = len;
        raw_key.selection = OSSL_KEYMGMT_SELECT_PRIVATE_KEY;

        return evp_keymgmt_util_export(pkey, OSSL_KEYMGMT_SELECT_PRIVATE_KEY,
                                       get_raw_key_details, &raw_key);
    }

    if (pkey->ameth == NULL) {
        EVPerr(0, EVP_R_OPERATION_NOT_SUPPORTED_FOR_THIS_KEYTYPE);
        return 0;
    }

    if (pkey->ameth->get_priv_key == NULL) {
        EVPerr(0, EVP_R_OPERATION_NOT_SUPPORTED_FOR_THIS_KEYTYPE);
        return 0;
    }

    if (!pkey->ameth->get_priv_key(pkey, priv, len)) {
        EVPerr(0, EVP_R_GET_RAW_KEY_FAILED);
        return 0;
    }

    return 1;
}

int EVP_PKEY_get_raw_public_key(const EVP_PKEY *pkey, unsigned char *pub,
                                size_t *len)
{
    if (pkey->keymgmt != NULL) {
        struct raw_key_details_st raw_key;

        raw_key.key = pub == NULL ? NULL : &pub;
        raw_key.len = len;
        raw_key.selection = OSSL_KEYMGMT_SELECT_PUBLIC_KEY;

        return evp_keymgmt_util_export(pkey, OSSL_KEYMGMT_SELECT_PUBLIC_KEY,
                                       get_raw_key_details, &raw_key);
    }

    if (pkey->ameth == NULL) {
        EVPerr(0, EVP_R_OPERATION_NOT_SUPPORTED_FOR_THIS_KEYTYPE);
        return 0;
    }

     if (pkey->ameth->get_pub_key == NULL) {
        EVPerr(EVP_F_EVP_PKEY_GET_RAW_PUBLIC_KEY,
               EVP_R_OPERATION_NOT_SUPPORTED_FOR_THIS_KEYTYPE);
        return 0;
    }

    if (!pkey->ameth->get_pub_key(pkey, pub, len)) {
        EVPerr(EVP_F_EVP_PKEY_GET_RAW_PUBLIC_KEY, EVP_R_GET_RAW_KEY_FAILED);
        return 0;
    }

    return 1;
}

static EVP_PKEY *new_cmac_key_int(const unsigned char *priv, size_t len,
                                  const char *cipher_name,
                                  const EVP_CIPHER *cipher, OPENSSL_CTX *libctx,
                                  const char *propq, ENGINE *e)
{
# ifndef OPENSSL_NO_CMAC
#  ifndef OPENSSL_NO_ENGINE
    const char *engine_id = e != NULL ? ENGINE_get_id(e) : NULL;
#  endif
    OSSL_PARAM params[5], *p = params;
    EVP_PKEY *pkey = NULL;
    EVP_PKEY_CTX *ctx;

    if (cipher != NULL)
        cipher_name = EVP_CIPHER_name(cipher);

    if (cipher_name == NULL) {
        EVPerr(0, EVP_R_KEY_SETUP_FAILED);
        return NULL;
    }

    ctx = EVP_PKEY_CTX_new_from_name(libctx, "CMAC", propq);
    if (ctx == NULL)
        goto err;

    if (!EVP_PKEY_key_fromdata_init(ctx)) {
        EVPerr(0, EVP_R_KEY_SETUP_FAILED);
        goto err;
    }

    *p++ = OSSL_PARAM_construct_octet_string(OSSL_PKEY_PARAM_PRIV_KEY,
                                            (void *)priv, len);
    *p++ = OSSL_PARAM_construct_utf8_string(OSSL_PKEY_PARAM_CIPHER,
                                            (char *)cipher_name, 0);
    if (propq != NULL)
        *p++ = OSSL_PARAM_construct_utf8_string(OSSL_PKEY_PARAM_PROPERTIES,
                                                (char *)propq, 0);
#  ifndef OPENSSL_NO_ENGINE
    if (engine_id != NULL)
        *p++ = OSSL_PARAM_construct_utf8_string(OSSL_PKEY_PARAM_ENGINE,
                                                (char *)engine_id, 0);
#  endif
    *p = OSSL_PARAM_construct_end();

    if (!EVP_PKEY_fromdata(ctx, &pkey, params)) {
        EVPerr(0, EVP_R_KEY_SETUP_FAILED);
        goto err;
    }

 err:
    EVP_PKEY_CTX_free(ctx);

    return pkey;
# else
    EVPerr(0, EVP_R_OPERATION_NOT_SUPPORTED_FOR_THIS_KEYTYPE);
    return NULL;
# endif
}

EVP_PKEY *EVP_PKEY_new_CMAC_key_with_libctx(const unsigned char *priv,
                                            size_t len,
                                            const char *cipher_name,
                                            OPENSSL_CTX *libctx,
                                            const char *propq)
{
    return new_cmac_key_int(priv, len, cipher_name, NULL, libctx, propq, NULL);
}

EVP_PKEY *EVP_PKEY_new_CMAC_key(ENGINE *e, const unsigned char *priv,
                                size_t len, const EVP_CIPHER *cipher)
{
    return new_cmac_key_int(priv, len, NULL, cipher, NULL, NULL, e);
}

int EVP_PKEY_set_type(EVP_PKEY *pkey, int type)
{
    return pkey_set_type(pkey, NULL, type, NULL, -1, NULL);
}

int EVP_PKEY_set_type_str(EVP_PKEY *pkey, const char *str, int len)
{
    return pkey_set_type(pkey, NULL, EVP_PKEY_NONE, str, len, NULL);
}

int EVP_PKEY_set_alias_type(EVP_PKEY *pkey, int type)
{
    if (pkey->type == type) {
        return 1; /* it already is that type */
    }

    /*
     * The application is requesting to alias this to a different pkey type,
     * but not one that resolves to the base type.
     */
    if (EVP_PKEY_type(type) != EVP_PKEY_base_id(pkey)) {
        EVPerr(EVP_F_EVP_PKEY_SET_ALIAS_TYPE, EVP_R_UNSUPPORTED_ALGORITHM);
        return 0;
    }

    pkey->type = type;
    return 1;
}

# ifndef OPENSSL_NO_ENGINE
int EVP_PKEY_set1_engine(EVP_PKEY *pkey, ENGINE *e)
{
    if (e != NULL) {
        if (!ENGINE_init(e)) {
            EVPerr(EVP_F_EVP_PKEY_SET1_ENGINE, ERR_R_ENGINE_LIB);
            return 0;
        }
        if (ENGINE_get_pkey_meth(e, pkey->type) == NULL) {
            ENGINE_finish(e);
            EVPerr(EVP_F_EVP_PKEY_SET1_ENGINE, EVP_R_UNSUPPORTED_ALGORITHM);
            return 0;
        }
    }
    ENGINE_finish(pkey->pmeth_engine);
    pkey->pmeth_engine = e;
    return 1;
}

ENGINE *EVP_PKEY_get0_engine(const EVP_PKEY *pkey)
{
    return pkey->engine;
}
# endif
int EVP_PKEY_assign(EVP_PKEY *pkey, int type, void *key)
{
    int alias = type;

#ifndef OPENSSL_NO_EC
    if ((key != NULL) && (EVP_PKEY_type(type) == EVP_PKEY_EC)) {
        const EC_GROUP *group = EC_KEY_get0_group(key);

        if (group != NULL && EC_GROUP_get_curve_name(group) == NID_sm2)
            alias = EVP_PKEY_SM2;
    }
#endif

    if (pkey == NULL || !EVP_PKEY_set_type(pkey, type))
        return 0;
    if (!EVP_PKEY_set_alias_type(pkey, alias))
        return 0;
    pkey->pkey.ptr = key;
    return (key != NULL);
}

void *EVP_PKEY_get0(const EVP_PKEY *pkey)
{
    if (pkey == NULL)
        return NULL;
    if (!evp_pkey_downgrade((EVP_PKEY *)pkey)) {
        ERR_raise(ERR_LIB_EVP, EVP_R_INACCESSIBLE_KEY);
        return NULL;
    }
    return pkey->pkey.ptr;
}

const unsigned char *EVP_PKEY_get0_hmac(const EVP_PKEY *pkey, size_t *len)
{
    ASN1_OCTET_STRING *os = NULL;
    if (pkey->type != EVP_PKEY_HMAC) {
        EVPerr(EVP_F_EVP_PKEY_GET0_HMAC, EVP_R_EXPECTING_AN_HMAC_KEY);
        return NULL;
    }
    os = EVP_PKEY_get0(pkey);
    *len = os->length;
    return os->data;
}

# ifndef OPENSSL_NO_POLY1305
const unsigned char *EVP_PKEY_get0_poly1305(const EVP_PKEY *pkey, size_t *len)
{
    ASN1_OCTET_STRING *os = NULL;
    if (pkey->type != EVP_PKEY_POLY1305) {
        EVPerr(EVP_F_EVP_PKEY_GET0_POLY1305, EVP_R_EXPECTING_A_POLY1305_KEY);
        return NULL;
    }
    os = EVP_PKEY_get0(pkey);
    *len = os->length;
    return os->data;
}
# endif

# ifndef OPENSSL_NO_SIPHASH
const unsigned char *EVP_PKEY_get0_siphash(const EVP_PKEY *pkey, size_t *len)
{
    ASN1_OCTET_STRING *os = NULL;

    if (pkey->type != EVP_PKEY_SIPHASH) {
        EVPerr(EVP_F_EVP_PKEY_GET0_SIPHASH, EVP_R_EXPECTING_A_SIPHASH_KEY);
        return NULL;
    }
    os = EVP_PKEY_get0(pkey);
    *len = os->length;
    return os->data;
}
# endif

# ifndef OPENSSL_NO_RSA
int EVP_PKEY_set1_RSA(EVP_PKEY *pkey, RSA *key)
{
    int ret = EVP_PKEY_assign_RSA(pkey, key);
    if (ret)
        RSA_up_ref(key);
    return ret;
}

RSA *EVP_PKEY_get0_RSA(const EVP_PKEY *pkey)
{
    if (!evp_pkey_downgrade((EVP_PKEY *)pkey)) {
        ERR_raise(ERR_LIB_EVP, EVP_R_INACCESSIBLE_KEY);
        return NULL;
    }
    if (pkey->type != EVP_PKEY_RSA && pkey->type != EVP_PKEY_RSA_PSS) {
        EVPerr(EVP_F_EVP_PKEY_GET0_RSA, EVP_R_EXPECTING_AN_RSA_KEY);
        return NULL;
    }
    return pkey->pkey.rsa;
}

RSA *EVP_PKEY_get1_RSA(EVP_PKEY *pkey)
{
    RSA *ret = EVP_PKEY_get0_RSA(pkey);
    if (ret != NULL)
        RSA_up_ref(ret);
    return ret;
}
# endif

# ifndef OPENSSL_NO_DSA
DSA *EVP_PKEY_get0_DSA(const EVP_PKEY *pkey)
{
    if (!evp_pkey_downgrade((EVP_PKEY *)pkey)) {
        ERR_raise(ERR_LIB_EVP, EVP_R_INACCESSIBLE_KEY);
        return NULL;
    }
    if (pkey->type != EVP_PKEY_DSA) {
        EVPerr(EVP_F_EVP_PKEY_GET0_DSA, EVP_R_EXPECTING_A_DSA_KEY);
        return NULL;
    }
    return pkey->pkey.dsa;
}

int EVP_PKEY_set1_DSA(EVP_PKEY *pkey, DSA *key)
{
    int ret = EVP_PKEY_assign_DSA(pkey, key);
    if (ret)
        DSA_up_ref(key);
    return ret;
}
DSA *EVP_PKEY_get1_DSA(EVP_PKEY *pkey)
{
    DSA *ret = EVP_PKEY_get0_DSA(pkey);
    if (ret != NULL)
        DSA_up_ref(ret);
    return ret;
}
# endif /*  OPENSSL_NO_DSA */
#endif /* FIPS_MODULE */

#ifndef FIPS_MODULE
# ifndef OPENSSL_NO_EC
int EVP_PKEY_set1_EC_KEY(EVP_PKEY *pkey, EC_KEY *key)
{
    int ret = EVP_PKEY_assign_EC_KEY(pkey, key);
    if (ret)
        EC_KEY_up_ref(key);
    return ret;
}

EC_KEY *EVP_PKEY_get0_EC_KEY(const EVP_PKEY *pkey)
{
    if (!evp_pkey_downgrade((EVP_PKEY *)pkey)) {
        ERR_raise(ERR_LIB_EVP, EVP_R_INACCESSIBLE_KEY);
        return NULL;
    }
    if (EVP_PKEY_base_id(pkey) != EVP_PKEY_EC) {
        EVPerr(EVP_F_EVP_PKEY_GET0_EC_KEY, EVP_R_EXPECTING_A_EC_KEY);
        return NULL;
    }
    return pkey->pkey.ec;
}

EC_KEY *EVP_PKEY_get1_EC_KEY(EVP_PKEY *pkey)
{
    EC_KEY *ret = EVP_PKEY_get0_EC_KEY(pkey);
    if (ret != NULL)
        EC_KEY_up_ref(ret);
    return ret;
}

static int EVP_PKEY_set1_ECX_KEY(EVP_PKEY *pkey, int type, ECX_KEY *key)
{
    int ret = EVP_PKEY_assign(pkey, type, key);
    if (ret)
        ecx_key_up_ref(key);
    return ret;
}

static ECX_KEY *EVP_PKEY_get0_ECX_KEY(const EVP_PKEY *pkey, int type)
{
    if (!evp_pkey_downgrade((EVP_PKEY *)pkey)) {
        ERR_raise(ERR_LIB_EVP, EVP_R_INACCESSIBLE_KEY);
        return NULL;
    }
    if (EVP_PKEY_base_id(pkey) != type) {
        ERR_raise(ERR_LIB_EVP, EVP_R_EXPECTING_A_ECX_KEY);
        return NULL;
    }
    return pkey->pkey.ecx;
}

static ECX_KEY *EVP_PKEY_get1_ECX_KEY(EVP_PKEY *pkey, int type)
{
    ECX_KEY *ret = EVP_PKEY_get0_ECX_KEY(pkey, type);
    if (ret != NULL)
        ecx_key_up_ref(ret);
    return ret;
}

#  define IMPLEMENT_ECX_VARIANT(NAME)                                   \
    int EVP_PKEY_set1_##NAME(EVP_PKEY *pkey, ECX_KEY *key)              \
    {                                                                   \
        return EVP_PKEY_set1_ECX_KEY(pkey, EVP_PKEY_##NAME, key);       \
    }                                                                   \
    ECX_KEY *EVP_PKEY_get0_##NAME(const EVP_PKEY *pkey)                 \
    {                                                                   \
        return EVP_PKEY_get0_ECX_KEY(pkey, EVP_PKEY_##NAME);            \
    }                                                                   \
    ECX_KEY *EVP_PKEY_get1_##NAME(EVP_PKEY *pkey)                       \
    {                                                                   \
        return EVP_PKEY_get1_ECX_KEY(pkey, EVP_PKEY_##NAME);            \
    }
IMPLEMENT_ECX_VARIANT(X25519)
IMPLEMENT_ECX_VARIANT(X448)
IMPLEMENT_ECX_VARIANT(ED25519)
IMPLEMENT_ECX_VARIANT(ED448)

# endif

# ifndef OPENSSL_NO_DH

int EVP_PKEY_set1_DH(EVP_PKEY *pkey, DH *key)
{
    int type = DH_get0_q(key) == NULL ? EVP_PKEY_DH : EVP_PKEY_DHX;
    int ret = EVP_PKEY_assign(pkey, type, key);

    if (ret)
        DH_up_ref(key);
    return ret;
}

DH *EVP_PKEY_get0_DH(const EVP_PKEY *pkey)
{
    if (!evp_pkey_downgrade((EVP_PKEY *)pkey)) {
        ERR_raise(ERR_LIB_EVP, EVP_R_INACCESSIBLE_KEY);
        return NULL;
    }
    if (pkey->type != EVP_PKEY_DH && pkey->type != EVP_PKEY_DHX) {
        EVPerr(EVP_F_EVP_PKEY_GET0_DH, EVP_R_EXPECTING_A_DH_KEY);
        return NULL;
    }
    return pkey->pkey.dh;
}

DH *EVP_PKEY_get1_DH(EVP_PKEY *pkey)
{
    DH *ret = EVP_PKEY_get0_DH(pkey);
    if (ret != NULL)
        DH_up_ref(ret);
    return ret;
}
# endif

int EVP_PKEY_type(int type)
{
    int ret;
    const EVP_PKEY_ASN1_METHOD *ameth;
    ENGINE *e;
    ameth = EVP_PKEY_asn1_find(&e, type);
    if (ameth)
        ret = ameth->pkey_id;
    else
        ret = NID_undef;
# ifndef OPENSSL_NO_ENGINE
    ENGINE_finish(e);
# endif
    return ret;
}

int EVP_PKEY_id(const EVP_PKEY *pkey)
{
    return pkey->type;
}

int EVP_PKEY_base_id(const EVP_PKEY *pkey)
{
    return EVP_PKEY_type(pkey->type);
}

#ifndef FIPS_MODULE
int evp_pkey_name2type(const char *name)
{
    /*
     * These hard coded cases are pure hackery to get around the fact
     * that names in crypto/objects/objects.txt are a mess.  There is
     * no "EC", and "RSA" leads to the NID for 2.5.8.1.1, an OID that's
     * fallen out in favor of { pkcs-1 1 }, i.e. 1.2.840.113549.1.1.1,
     * the NID of which is used for EVP_PKEY_RSA.  Strangely enough,
     * "DSA" is accurate...  but still, better be safe and hard-code
     * names that we know.
     * On a similar topic, EVP_PKEY_type(EVP_PKEY_SM2) will result in
     * EVP_PKEY_EC, because of aliasing.
     * TODO Clean this away along with all other #legacy support.
     */
    int type = NID_undef;

    if (strcasecmp(name, "RSA") == 0)
        type = EVP_PKEY_RSA;
    else if (strcasecmp(name, "RSA-PSS") == 0)
        type = EVP_PKEY_RSA_PSS;
    else if (strcasecmp(name, "EC") == 0)
        type = EVP_PKEY_EC;
    else if (strcasecmp(name, "ED25519") == 0)
        type = EVP_PKEY_ED25519;
    else if (strcasecmp(name, "ED448") == 0)
        type = EVP_PKEY_ED448;
    else if (strcasecmp(name, "X25519") == 0)
        type = EVP_PKEY_X25519;
    else if (strcasecmp(name, "X448") == 0)
        type = EVP_PKEY_X448;
    else if (strcasecmp(name, "SM2") == 0)
        type = EVP_PKEY_SM2;
    else if (strcasecmp(name, "DH") == 0)
        type = EVP_PKEY_DH;
    else if (strcasecmp(name, "X9.42 DH") == 0)
        type = EVP_PKEY_DHX;
    else if (strcasecmp(name, "DSA") == 0)
        type = EVP_PKEY_DSA;

    if (type == NID_undef)
        type = EVP_PKEY_type(OBJ_sn2nid(name));
    if (type == NID_undef)
        type = EVP_PKEY_type(OBJ_ln2nid(name));

    return type;
}
#endif

int EVP_PKEY_is_a(const EVP_PKEY *pkey, const char *name)
{
#ifndef FIPS_MODULE
    if (pkey->keymgmt == NULL) {
        int type = evp_pkey_name2type(name);

        return pkey->type == type;
    }
#endif
    return EVP_KEYMGMT_is_a(pkey->keymgmt, name);
}

int EVP_PKEY_can_sign(const EVP_PKEY *pkey)
{
    if (pkey->keymgmt == NULL) {
        switch (EVP_PKEY_base_id(pkey)) {
        case EVP_PKEY_RSA:
            return 1;
#ifndef OPENSSL_NO_DSA
        case EVP_PKEY_DSA:
            return 1;
#endif
#ifndef OPENSSL_NO_EC
        case EVP_PKEY_ED25519:
        case EVP_PKEY_ED448:
            return 1;
        case EVP_PKEY_EC:        /* Including SM2 */
            return EC_KEY_can_sign(pkey->pkey.ec);
#endif
        default:
            break;
        }
    } else {
        const OSSL_PROVIDER *prov = EVP_KEYMGMT_provider(pkey->keymgmt);
        OPENSSL_CTX *libctx = ossl_provider_library_context(prov);
        const char *supported_sig =
            pkey->keymgmt->query_operation_name != NULL
            ? pkey->keymgmt->query_operation_name(OSSL_OP_SIGNATURE)
            : evp_first_name(prov, pkey->keymgmt->name_id);
        EVP_SIGNATURE *signature = NULL;

        signature = EVP_SIGNATURE_fetch(libctx, supported_sig, NULL);
        if (signature != NULL) {
            EVP_SIGNATURE_free(signature);
            return 1;
        }
    }
    return 0;
}

#ifndef OPENSSL_NO_EC
/*
 * TODO rewrite when we have proper data extraction functions
 * Note: an octet pointer would be desirable!
 */
static OSSL_CALLBACK get_ec_curve_name_cb;
static int get_ec_curve_name_cb(const OSSL_PARAM params[], void *arg)
{
    const OSSL_PARAM *p = NULL;

    if ((p = OSSL_PARAM_locate_const(params, OSSL_PKEY_PARAM_GROUP_NAME)) != NULL)
        return OSSL_PARAM_get_utf8_string(p, arg, 0);

    /* If there is no curve name, this is not an EC key */
    return 0;
}

int evp_pkey_get_EC_KEY_curve_nid(const EVP_PKEY *pkey)
{
    int ret = NID_undef;

    if (pkey->keymgmt == NULL) {
        if (EVP_PKEY_base_id(pkey) == EVP_PKEY_EC) {
            EC_KEY *ec = EVP_PKEY_get0_EC_KEY(pkey);

            ret = EC_GROUP_get_curve_name(EC_KEY_get0_group(ec));
        }
    } else if (EVP_PKEY_is_a(pkey, "EC") || EVP_PKEY_is_a(pkey, "SM2")) {
        char *curve_name = NULL;

        ret = evp_keymgmt_util_export(pkey,
                                      OSSL_KEYMGMT_SELECT_DOMAIN_PARAMETERS,
                                      get_ec_curve_name_cb, &curve_name);
        if (ret)
            ret = ec_curve_name2nid(curve_name);
        OPENSSL_free(curve_name);
    }

    return ret;
}
#endif

static int print_reset_indent(BIO **out, int pop_f_prefix, long saved_indent)
{
    BIO_set_indent(*out, saved_indent);
    if (pop_f_prefix) {
        BIO *next = BIO_pop(*out);

        BIO_free(*out);
        *out = next;
    }
    return 1;
}

static int print_set_indent(BIO **out, int *pop_f_prefix, long *saved_indent,
                            long indent)
{
    *pop_f_prefix = 0;
    *saved_indent = 0;
    if (indent > 0) {
        long i = BIO_get_indent(*out);

        *saved_indent =  (i < 0 ? 0 : i);
        if (BIO_set_indent(*out, indent) <= 0) {
            if ((*out = BIO_push(BIO_new(BIO_f_prefix()), *out)) == NULL)
                return 0;
            *pop_f_prefix = 1;
        }
        if (BIO_set_indent(*out, indent) <= 0) {
            print_reset_indent(out, *pop_f_prefix, *saved_indent);
            return 0;
        }
    }
    return 1;
}

static int unsup_alg(BIO *out, const EVP_PKEY *pkey, int indent,
                     const char *kstr)
{
    return BIO_indent(out, indent, 128)
        && BIO_printf(out, "%s algorithm \"%s\" unsupported\n",
                      kstr, OBJ_nid2ln(pkey->type)) > 0;
}

static int print_pkey(const EVP_PKEY *pkey, BIO *out, int indent,
                      const char *propquery /* For provided encoding */,
                      int (*legacy_print)(BIO *out, const EVP_PKEY *pkey,
                                          int indent, ASN1_PCTX *pctx),
                      ASN1_PCTX *legacy_pctx /* For legacy print */)
{
    int pop_f_prefix;
    long saved_indent;
    OSSL_ENCODER_CTX *ctx = NULL;
    int ret = -2;                /* default to unsupported */

    if (!print_set_indent(&out, &pop_f_prefix, &saved_indent, indent))
        return 0;

    ctx = OSSL_ENCODER_CTX_new_by_EVP_PKEY(pkey, propquery);
    if (OSSL_ENCODER_CTX_get_encoder(ctx) != NULL)
        ret = OSSL_ENCODER_to_bio(ctx, out);
    OSSL_ENCODER_CTX_free(ctx);

    if (ret != -2)
        goto end;

    /* legacy fallback */
    if (legacy_print != NULL)
        ret = legacy_print(out, pkey, 0, legacy_pctx);
    else
        ret = unsup_alg(out, pkey, 0, "Public Key");

 end:
    print_reset_indent(&out, pop_f_prefix, saved_indent);
    return ret;
}

int EVP_PKEY_print_public(BIO *out, const EVP_PKEY *pkey,
                          int indent, ASN1_PCTX *pctx)
{
    return print_pkey(pkey, out, indent, OSSL_ENCODER_PUBKEY_TO_TEXT_PQ,
                      (pkey->ameth != NULL ? pkey->ameth->pub_print : NULL),
                      pctx);
}

int EVP_PKEY_print_private(BIO *out, const EVP_PKEY *pkey,
                           int indent, ASN1_PCTX *pctx)
{
    return print_pkey(pkey, out, indent, OSSL_ENCODER_PrivateKey_TO_TEXT_PQ,
                      (pkey->ameth != NULL ? pkey->ameth->priv_print : NULL),
                      pctx);
}

int EVP_PKEY_print_params(BIO *out, const EVP_PKEY *pkey,
                          int indent, ASN1_PCTX *pctx)
{
    return print_pkey(pkey, out, indent, OSSL_ENCODER_Parameters_TO_TEXT_PQ,
                      (pkey->ameth != NULL ? pkey->ameth->param_print : NULL),
                      pctx);
}

static int legacy_asn1_ctrl_to_param(EVP_PKEY *pkey, int op,
                                     int arg1, void *arg2)
{
    if (pkey->keymgmt == NULL)
        return 0;
    switch (op) {
    case ASN1_PKEY_CTRL_DEFAULT_MD_NID:
        {
            char mdname[80] = "";
            int rv = EVP_PKEY_get_default_digest_name(pkey, mdname,
                                                      sizeof(mdname));

            if (rv > 0) {
                int nid;

                nid = OBJ_sn2nid(mdname);
                if (nid == NID_undef)
                    nid = OBJ_ln2nid(mdname);
                *(int *)arg2 = nid;
            }
            return rv;
        }
    default:
        return -2;
    }
}

static int evp_pkey_asn1_ctrl(EVP_PKEY *pkey, int op, int arg1, void *arg2)
{
    if (pkey->ameth == NULL)
        return legacy_asn1_ctrl_to_param(pkey, op, arg1, arg2);
    if (pkey->ameth->pkey_ctrl == NULL)
        return -2;
    return pkey->ameth->pkey_ctrl(pkey, op, arg1, arg2);
}

int EVP_PKEY_get_default_digest_nid(EVP_PKEY *pkey, int *pnid)
{
    return evp_pkey_asn1_ctrl(pkey, ASN1_PKEY_CTRL_DEFAULT_MD_NID, 0, pnid);
}

int EVP_PKEY_get_default_digest_name(EVP_PKEY *pkey,
                                     char *mdname, size_t mdname_sz)
{
    if (pkey->ameth == NULL)
        return evp_keymgmt_util_get_deflt_digest_name(pkey->keymgmt,
                                                      pkey->keydata,
                                                      mdname, mdname_sz);

    {
        int nid = NID_undef;
        int rv = EVP_PKEY_get_default_digest_nid(pkey, &nid);
        const char *name = rv > 0 ? OBJ_nid2sn(nid) : NULL;

        if (rv > 0)
            OPENSSL_strlcpy(mdname, name, mdname_sz);
        return rv;
    }
}

int EVP_PKEY_supports_digest_nid(EVP_PKEY *pkey, int nid)
{
    int rv, default_nid;

    rv = evp_pkey_asn1_ctrl(pkey, ASN1_PKEY_CTRL_SUPPORTS_MD_NID, nid, NULL);
    if (rv == -2) {
        /*
         * If there is a mandatory default digest and this isn't it, then
         * the answer is 'no'.
         */
        rv = EVP_PKEY_get_default_digest_nid(pkey, &default_nid);
        if (rv == 2)
            return (nid == default_nid);
        /* zero is an error from EVP_PKEY_get_default_digest_nid() */
        if (rv == 0)
            return -1;
    }
    return rv;
}

int EVP_PKEY_set1_tls_encodedpoint(EVP_PKEY *pkey,
                               const unsigned char *pt, size_t ptlen)
{
    if (pkey->ameth == NULL) {
        OSSL_PARAM params[2] = { OSSL_PARAM_END, OSSL_PARAM_END };

        if (pkey->keymgmt == NULL || pkey->keydata == NULL)
            return 0;

        params[0] =
            OSSL_PARAM_construct_octet_string(OSSL_PKEY_PARAM_TLS_ENCODED_PT,
                                              (unsigned char *)pt, ptlen);
        return evp_keymgmt_set_params(pkey->keymgmt, pkey->keydata, params);
    }

    if (ptlen > INT_MAX)
        return 0;
    if (evp_pkey_asn1_ctrl(pkey, ASN1_PKEY_CTRL_SET1_TLS_ENCPT, ptlen,
                           (void *)pt) <= 0)
        return 0;
    return 1;
}

size_t EVP_PKEY_get1_tls_encodedpoint(EVP_PKEY *pkey, unsigned char **ppt)
{
    int rv;

    if (pkey->ameth == NULL) {
        OSSL_PARAM params[2] = { OSSL_PARAM_END, OSSL_PARAM_END };

        if (pkey->keymgmt == NULL || pkey->keydata == NULL)
            return 0;

        params[0] =
            OSSL_PARAM_construct_octet_string(OSSL_PKEY_PARAM_TLS_ENCODED_PT,
                                              NULL, 0);
        if (!evp_keymgmt_get_params(pkey->keymgmt, pkey->keydata, params))
            return 0;

        *ppt = OPENSSL_malloc(params[0].return_size);
        if (*ppt == NULL)
            return 0;

        params[0] =
            OSSL_PARAM_construct_octet_string(OSSL_PKEY_PARAM_TLS_ENCODED_PT,
                                              *ppt, params[0].return_size);
        if (!evp_keymgmt_get_params(pkey->keymgmt, pkey->keydata, params))
            return 0;

        return params[0].return_size;
    }


    rv = evp_pkey_asn1_ctrl(pkey, ASN1_PKEY_CTRL_GET1_TLS_ENCPT, 0, ppt);
    if (rv <= 0)
        return 0;
    return rv;
}

#endif /* FIPS_MODULE */

/*- All methods below can also be used in FIPS_MODULE */

static int evp_pkey_reset_unlocked(EVP_PKEY *pk)
{
    if (pk == NULL)
        return 0;

    memset(pk, 0, sizeof(*pk));
    pk->type = EVP_PKEY_NONE;
    pk->save_type = EVP_PKEY_NONE;
    pk->references = 1;
    pk->save_parameters = 1;
    return 1;
}

EVP_PKEY *EVP_PKEY_new(void)
{
    EVP_PKEY *ret = OPENSSL_zalloc(sizeof(*ret));

    if (ret == NULL) {
        EVPerr(EVP_F_EVP_PKEY_NEW, ERR_R_MALLOC_FAILURE);
        return NULL;
    }

    if (!evp_pkey_reset_unlocked(ret))
        goto err;

    ret->lock = CRYPTO_THREAD_lock_new();
    if (ret->lock == NULL) {
        EVPerr(EVP_F_EVP_PKEY_NEW, ERR_R_MALLOC_FAILURE);
        goto err;
    }
#ifndef FIPS_MODULE
    if (!CRYPTO_new_ex_data(CRYPTO_EX_INDEX_EVP_PKEY, ret, &ret->ex_data)) {
        EVPerr(EVP_F_EVP_PKEY_NEW, ERR_R_MALLOC_FAILURE);
        goto err;
    }
#endif
    return ret;

 err:
    CRYPTO_THREAD_lock_free(ret->lock);
    OPENSSL_free(ret);
    return NULL;
}

/*
 * Setup a public key management method.
 *
 * For legacy keys, either |type| or |str| is expected to have the type
 * information.  In this case, the setup consists of finding an ASN1 method
 * and potentially an ENGINE, and setting those fields in |pkey|.
 *
 * For provider side keys, |keymgmt| is expected to be non-NULL.  In this
 * case, the setup consists of setting the |keymgmt| field in |pkey|.
 *
 * If pkey is NULL just return 1 or 0 if the key management method exists.
 */

static int pkey_set_type(EVP_PKEY *pkey, ENGINE *e, int type, const char *str,
                         int len, EVP_KEYMGMT *keymgmt)
{
#ifndef FIPS_MODULE
    const EVP_PKEY_ASN1_METHOD *ameth = NULL;
    ENGINE **eptr = (e == NULL) ? &e :  NULL;
#endif

    /*
     * The setups can't set both legacy and provider side methods.
     * It is forbidden
     */
    if (!ossl_assert(type == EVP_PKEY_NONE || keymgmt == NULL)
        || !ossl_assert(e == NULL || keymgmt == NULL)) {
        ERR_raise(ERR_LIB_EVP, ERR_R_INTERNAL_ERROR);
        return 0;
    }

    if (pkey != NULL) {
        int free_it = 0;

#ifndef FIPS_MODULE
        free_it = free_it || pkey->pkey.ptr != NULL;
#endif
        free_it = free_it || pkey->keydata != NULL;
        if (free_it)
            evp_pkey_free_it(pkey);
#ifndef FIPS_MODULE
        /*
         * If key type matches and a method exists then this lookup has
         * succeeded once so just indicate success.
         */
        if (pkey->type != EVP_PKEY_NONE
            && type == pkey->save_type
            && pkey->ameth != NULL)
            return 1;
# ifndef OPENSSL_NO_ENGINE
        /* If we have ENGINEs release them */
        ENGINE_finish(pkey->engine);
        pkey->engine = NULL;
        ENGINE_finish(pkey->pmeth_engine);
        pkey->pmeth_engine = NULL;
# endif
#endif
    }
#ifndef FIPS_MODULE
    if (str != NULL)
        ameth = EVP_PKEY_asn1_find_str(eptr, str, len);
    else if (type != EVP_PKEY_NONE)
        ameth = EVP_PKEY_asn1_find(eptr, type);
# ifndef OPENSSL_NO_ENGINE
    if (pkey == NULL && eptr != NULL)
        ENGINE_finish(e);
# endif
#endif


    {
        int check = 1;

#ifndef FIPS_MODULE
        check = check && ameth == NULL;
#endif
        check = check && keymgmt == NULL;
        if (check) {
            EVPerr(EVP_F_PKEY_SET_TYPE, EVP_R_UNSUPPORTED_ALGORITHM);
            return 0;
        }
    }
    if (pkey != NULL) {
        if (keymgmt != NULL && !EVP_KEYMGMT_up_ref(keymgmt)) {
            ERR_raise(ERR_LIB_EVP, ERR_R_INTERNAL_ERROR);
            return 0;
        }

        pkey->keymgmt = keymgmt;

        pkey->save_type = type;
        pkey->type = type;

#ifndef FIPS_MODULE
        /*
         * If the internal "origin" key is provider side, don't save |ameth|.
         * The main reason is that |ameth| is one factor to detect that the
         * internal "origin" key is a legacy one.
         */
        if (keymgmt == NULL)
            pkey->ameth = ameth;
        pkey->engine = e;

        /*
         * The EVP_PKEY_ASN1_METHOD |pkey_id| retains its legacy key purpose
         * for any key type that has a legacy implementation, regardless of
         * if the internal key is a legacy or a provider side one.  When
         * there is no legacy implementation for the key, the type becomes
         * EVP_PKEY_KEYMGMT, which indicates that one should be cautious
         * with functions that expect legacy internal keys.
         */
        if (ameth != NULL)
            pkey->type = ameth->pkey_id;
        else
            pkey->type = EVP_PKEY_KEYMGMT;
#endif
    }
    return 1;
}

#ifndef FIPS_MODULE
static void find_ameth(const char *name, void *data)
{
    const char **str = data;

    /*
     * The error messages from pkey_set_type() are uninteresting here,
     * and misleading.
     */
    ERR_set_mark();

    if (pkey_set_type(NULL, NULL, EVP_PKEY_NONE, name, strlen(name),
                      NULL)) {
        if (str[0] == NULL)
            str[0] = name;
        else if (str[1] == NULL)
            str[1] = name;
    }

    ERR_pop_to_mark();
}
#endif

int EVP_PKEY_set_type_by_keymgmt(EVP_PKEY *pkey, EVP_KEYMGMT *keymgmt)
{
#ifndef FIPS_MODULE
# define EVP_PKEY_TYPE_STR str[0]
# define EVP_PKEY_TYPE_STRLEN (str[0] == NULL ? -1 : (int)strlen(str[0]))
    /*
     * Find at most two strings that have an associated EVP_PKEY_ASN1_METHOD
     * Ideally, only one should be found.  If two (or more) are found, the
     * match is ambiguous.  This should never happen, but...
     */
    const char *str[2] = { NULL, NULL };

    EVP_KEYMGMT_names_do_all(keymgmt, find_ameth, &str);
    if (str[1] != NULL) {
        ERR_raise(ERR_LIB_EVP, ERR_R_INTERNAL_ERROR);
        return 0;
    }
#else
# define EVP_PKEY_TYPE_STR NULL
# define EVP_PKEY_TYPE_STRLEN -1
#endif
    return pkey_set_type(pkey, NULL, EVP_PKEY_NONE,
                         EVP_PKEY_TYPE_STR, EVP_PKEY_TYPE_STRLEN,
                         keymgmt);

#undef EVP_PKEY_TYPE_STR
#undef EVP_PKEY_TYPE_STRLEN
}

int EVP_PKEY_up_ref(EVP_PKEY *pkey)
{
    int i;

    if (CRYPTO_UP_REF(&pkey->references, &i, pkey->lock) <= 0)
        return 0;

    REF_PRINT_COUNT("EVP_PKEY", pkey);
    REF_ASSERT_ISNT(i < 2);
    return ((i > 1) ? 1 : 0);
}

#ifndef FIPS_MODULE
void evp_pkey_free_legacy(EVP_PKEY *x)
{
    if (x->ameth != NULL) {
        if (x->ameth->pkey_free != NULL)
            x->ameth->pkey_free(x);
        x->pkey.ptr = NULL;
    }
# ifndef OPENSSL_NO_ENGINE
    ENGINE_finish(x->engine);
    x->engine = NULL;
    ENGINE_finish(x->pmeth_engine);
    x->pmeth_engine = NULL;
# endif
}
#endif  /* FIPS_MODULE */

static void evp_pkey_free_it(EVP_PKEY *x)
{
    /* internal function; x is never NULL */

    evp_keymgmt_util_clear_operation_cache(x);
#ifndef FIPS_MODULE
    evp_pkey_free_legacy(x);
#endif

    if (x->keymgmt != NULL) {
        evp_keymgmt_freedata(x->keymgmt, x->keydata);
        EVP_KEYMGMT_free(x->keymgmt);
        x->keymgmt = NULL;
        x->keydata = NULL;
    }
    x->type = EVP_PKEY_NONE;
}

void EVP_PKEY_free(EVP_PKEY *x)
{
    int i;

    if (x == NULL)
        return;

    CRYPTO_DOWN_REF(&x->references, &i, x->lock);
    REF_PRINT_COUNT("EVP_PKEY", x);
    if (i > 0)
        return;
    REF_ASSERT_ISNT(i < 0);
    evp_pkey_free_it(x);
#ifndef FIPS_MODULE
    CRYPTO_free_ex_data(CRYPTO_EX_INDEX_EVP_PKEY, x, &x->ex_data);
#endif
    CRYPTO_THREAD_lock_free(x->lock);
#ifndef FIPS_MODULE
    sk_X509_ATTRIBUTE_pop_free(x->attributes, X509_ATTRIBUTE_free);
#endif
    OPENSSL_free(x);
}

int EVP_PKEY_size(const EVP_PKEY *pkey)
{
    int size = 0;

    if (pkey != NULL) {
        size = pkey->cache.size;
#ifndef FIPS_MODULE
        if (pkey->ameth != NULL && pkey->ameth->pkey_size != NULL)
            size = pkey->ameth->pkey_size(pkey);
#endif
    }
    return size;
}

void *evp_pkey_export_to_provider(EVP_PKEY *pk, OPENSSL_CTX *libctx,
                                  EVP_KEYMGMT **keymgmt,
                                  const char *propquery)
{
    EVP_KEYMGMT *allocated_keymgmt = NULL;
    EVP_KEYMGMT *tmp_keymgmt = NULL;
    void *keydata = NULL;
    int check;

    if (pk == NULL)
        return NULL;

    /* No key data => nothing to export */
    check = 1;
#ifndef FIPS_MODULE
    check = check && pk->pkey.ptr == NULL;
#endif
    check = check && pk->keydata == NULL;
    if (check)
        return NULL;

#ifndef FIPS_MODULE
    if (pk->pkey.ptr != NULL) {
        /*
         * If the legacy key doesn't have an dirty counter or export function,
         * give up
         */
        if (pk->ameth->dirty_cnt == NULL || pk->ameth->export_to == NULL)
            return NULL;
    }
#endif

    if (keymgmt != NULL) {
        tmp_keymgmt = *keymgmt;
        *keymgmt = NULL;
    }

    /*
     * If no keymgmt was given or found, get a default keymgmt.  We do so by
     * letting EVP_PKEY_CTX_new_from_pkey() do it for us, then we steal it.
     */
    if (tmp_keymgmt == NULL) {
        EVP_PKEY_CTX *ctx = EVP_PKEY_CTX_new_from_pkey(libctx, pk, propquery);

        tmp_keymgmt = ctx->keymgmt;
        ctx->keymgmt = NULL;
        EVP_PKEY_CTX_free(ctx);
    }

    /* If there's still no keymgmt to be had, give up */
    if (tmp_keymgmt == NULL)
        goto end;

#ifndef FIPS_MODULE
    if (pk->pkey.ptr != NULL) {
        size_t i = 0;

        /*
         * If the legacy "origin" hasn't changed since last time, we try
         * to find our keymgmt in the operation cache.  If it has changed,
         * |i| remains zero, and we will clear the cache further down.
         */
        if (pk->ameth->dirty_cnt(pk) == pk->dirty_cnt_copy) {
            i = evp_keymgmt_util_find_operation_cache_index(pk, tmp_keymgmt);

            /*
             * If |tmp_keymgmt| is present in the operation cache, it means
             * that export doesn't need to be redone.  In that case, we take
             * token copies of the cached pointers, to have token success
             * values to return.
             */
            if (i < OSSL_NELEM(pk->operation_cache)
                && pk->operation_cache[i].keymgmt != NULL) {
                keydata = pk->operation_cache[i].keydata;
                goto end;
            }
        }

        /*
         * TODO(3.0) Right now, we assume we have ample space.  We will have
         * to think about a cache aging scheme, though, if |i| indexes outside
         * the array.
         */
        if (!ossl_assert(i < OSSL_NELEM(pk->operation_cache)))
            goto end;

        /* Make sure that the keymgmt key type matches the legacy NID */
        if (!ossl_assert(EVP_KEYMGMT_is_a(tmp_keymgmt, OBJ_nid2sn(pk->type))))
            goto end;

        if ((keydata = evp_keymgmt_newdata(tmp_keymgmt)) == NULL)
            goto end;

        if (!pk->ameth->export_to(pk, keydata, tmp_keymgmt, libctx, propquery)) {
            evp_keymgmt_freedata(tmp_keymgmt, keydata);
            keydata = NULL;
            goto end;
        }

        /*
         * If the dirty counter changed since last time, then clear the
         * operation cache.  In that case, we know that |i| is zero.  Just
         * in case this is a re-export, we increment then decrement the
         * keymgmt reference counter.
         */
        if (!EVP_KEYMGMT_up_ref(tmp_keymgmt)) { /* refcnt++ */
            evp_keymgmt_freedata(tmp_keymgmt, keydata);
            keydata = NULL;
            goto end;
        }
        if (pk->ameth->dirty_cnt(pk) != pk->dirty_cnt_copy)
            evp_keymgmt_util_clear_operation_cache(pk);
        EVP_KEYMGMT_free(tmp_keymgmt); /* refcnt-- */

        /* Add the new export to the operation cache */
        if (!evp_keymgmt_util_cache_keydata(pk, i, tmp_keymgmt, keydata)) {
            evp_keymgmt_freedata(tmp_keymgmt, keydata);
            keydata = NULL;
            goto end;
        }

        /* Synchronize the dirty count */
        pk->dirty_cnt_copy = pk->ameth->dirty_cnt(pk);
        goto end;
    }
#endif  /* FIPS_MODULE */

    keydata = evp_keymgmt_util_export_to_provider(pk, tmp_keymgmt);

 end:
    /*
     * If nothing was exported, |tmp_keymgmt| might point at a freed
     * EVP_KEYMGMT, so we clear it to be safe.  It shouldn't be useful for
     * the caller either way in that case.
     */
    if (keydata == NULL)
        tmp_keymgmt = NULL;

    if (keymgmt != NULL)
        *keymgmt = tmp_keymgmt;

    EVP_KEYMGMT_free(allocated_keymgmt);
    return keydata;
}

#ifndef FIPS_MODULE
int evp_pkey_copy_downgraded(EVP_PKEY **dest, const EVP_PKEY *src)
{
    if (!ossl_assert(dest != NULL))
        return 0;

    if (evp_pkey_is_assigned(src) && evp_pkey_is_provided(src)) {
        EVP_KEYMGMT *keymgmt = src->keymgmt;
        void *keydata = src->keydata;
        int type = src->type;
        const char *keytype = NULL;

        keytype = evp_first_name(EVP_KEYMGMT_provider(keymgmt),
                                 keymgmt->name_id);

        /*
         * If the type is EVP_PKEY_NONE, then we have a problem somewhere
         * else in our code.  If it's not one of the well known EVP_PKEY_xxx
         * values, it should at least be EVP_PKEY_KEYMGMT at this point.
         * TODO(3.0) remove this check when we're confident that the rest
         * of the code treats this correctly.
         */
        if (!ossl_assert(type != EVP_PKEY_NONE)) {
            ERR_raise_data(ERR_LIB_EVP, ERR_R_INTERNAL_ERROR,
                           "keymgmt key type = %s but legacy type = EVP_PKEY_NONE",
                           keytype);
            return 0;
        }

        /* Prefer the legacy key type name for error reporting */
        if (type != EVP_PKEY_KEYMGMT)
            keytype = OBJ_nid2sn(type);

        /* Make sure we have a clean slate to copy into */
        if (*dest == NULL)
            *dest = EVP_PKEY_new();
        else
            evp_pkey_free_it(*dest);

        if (EVP_PKEY_set_type(*dest, type)) {
            /* If the key is typed but empty, we're done */
            if (keydata == NULL)
                return 1;

            if ((*dest)->ameth->import_from == NULL) {
                ERR_raise_data(ERR_LIB_EVP, EVP_R_NO_IMPORT_FUNCTION,
                               "key type = %s", keytype);
            } else {
                /*
                 * We perform the export in the same libctx as the keymgmt
                 * that we are using.
                 */
                OPENSSL_CTX *libctx =
                    ossl_provider_library_context(keymgmt->prov);
                EVP_PKEY_CTX *pctx =
                    EVP_PKEY_CTX_new_from_pkey(libctx, *dest, NULL);

                if (pctx == NULL)
                    ERR_raise(ERR_LIB_EVP, ERR_R_MALLOC_FAILURE);

                if (pctx != NULL
                    && evp_keymgmt_export(keymgmt, keydata,
                                          OSSL_KEYMGMT_SELECT_ALL,
                                          (*dest)->ameth->import_from,
                                          pctx)) {
                    /* Synchronize the dirty count */
                    (*dest)->dirty_cnt_copy = (*dest)->ameth->dirty_cnt(*dest);

                    EVP_PKEY_CTX_free(pctx);
                    return 1;
                }
                EVP_PKEY_CTX_free(pctx);
            }

            ERR_raise_data(ERR_LIB_EVP, EVP_R_KEYMGMT_EXPORT_FAILURE,
                           "key type = %s", keytype);
        }
    }

    return 0;
}

int evp_pkey_downgrade(EVP_PKEY *pk)
{
    EVP_PKEY tmp_copy;           /* Stack allocated! */

    /* If this isn't an assigned provider side key, we're done */
    if (!evp_pkey_is_assigned(pk) || !evp_pkey_is_provided(pk))
        return 1;

    /*
     * To be able to downgrade, we steal the contents of |pk|, then reset
     * it, and finally try to make it a downgraded copy.  If any of that
     * fails, we restore the copied contents into |pk|.
     */
    tmp_copy = *pk;

    if (evp_pkey_reset_unlocked(pk)
        && evp_pkey_copy_downgraded(&pk, &tmp_copy)) {
        /* Restore the common attributes, then empty |tmp_copy| */
        pk->references = tmp_copy.references;
        pk->lock = tmp_copy.lock;
        pk->attributes = tmp_copy.attributes;
        pk->save_parameters = tmp_copy.save_parameters;
        pk->ex_data = tmp_copy.ex_data;

        /* Ensure that stuff we've copied won't be freed */
        tmp_copy.lock = NULL;
        tmp_copy.attributes = NULL;
        memset(&tmp_copy.ex_data, 0, sizeof(tmp_copy.ex_data));

        /*
         * Save the provider side data in the operation cache, so they'll
         * find it again.  |pk| is new, so it's safe to assume slot zero
         * is free.
         * Note that evp_keymgmt_util_cache_keydata() increments keymgmt's
         * reference count, so we need to decrement it, or there will be a
         * leak.
         */
        evp_keymgmt_util_cache_keydata(pk, 0, tmp_copy.keymgmt,
                                       tmp_copy.keydata);
        EVP_KEYMGMT_free(tmp_copy.keymgmt);

        /*
         * Clear keymgmt and keydata from |tmp_copy|, or they'll get
         * inadvertently freed.
         */
        tmp_copy.keymgmt = NULL;
        tmp_copy.keydata = NULL;

        evp_pkey_free_it(&tmp_copy);

        return 1;
    }

    *pk = tmp_copy;
    return 0;
}
#endif  /* FIPS_MODULE */

const OSSL_PARAM *EVP_PKEY_gettable_params(EVP_PKEY *pkey)
{
    if (pkey == NULL
        || pkey->keymgmt == NULL
        || pkey->keydata == NULL)
        return 0;
    return EVP_KEYMGMT_gettable_params(pkey->keymgmt);
}

int EVP_PKEY_get_bn_param(EVP_PKEY *pkey, const char *key_name, BIGNUM **bn)
{
    int ret = 0;
    OSSL_PARAM params[2];
    unsigned char buffer[2048];
    unsigned char *buf = NULL;
    size_t buf_sz = 0;

    if (pkey == NULL
        || pkey->keymgmt == NULL
        || pkey->keydata == NULL
        || key_name == NULL
        || bn == NULL)
        return 0;

    memset(buffer, 0, sizeof(buffer));
    params[0] = OSSL_PARAM_construct_BN(key_name, buffer, sizeof(buffer));
    params[1] = OSSL_PARAM_construct_end();
    if (!evp_keymgmt_get_params(pkey->keymgmt, pkey->keydata, params)) {
        if (!OSSL_PARAM_modified(params) || params[0].return_size == 0)
            return 0;
        buf_sz = params[0].return_size;
        /*
         * If it failed because the buffer was too small then allocate the
         * required buffer size and retry.
         */
        buf = OPENSSL_zalloc(buf_sz);
        if (buf == NULL)
            return 0;
        params[0].data = buf;
        params[0].data_size = buf_sz;

        if (!evp_keymgmt_get_params(pkey->keymgmt, pkey->keydata, params))
            goto err;
    }
    /* Fail if the param was not found */
    if (!OSSL_PARAM_modified(params))
        goto err;
    ret = OSSL_PARAM_get_BN(params, bn);
err:
    OPENSSL_free(buf);
    return ret;
}

int EVP_PKEY_get_octet_string_param(EVP_PKEY *pkey, const char *key_name,
                                    unsigned char *buf, size_t max_buf_sz,
                                    size_t *out_sz)
{
    OSSL_PARAM params[2];

    if (pkey == NULL
        || pkey->keymgmt == NULL
        || pkey->keydata == NULL
        || key_name == NULL)
        return 0;

    params[0] = OSSL_PARAM_construct_octet_string(key_name, buf, max_buf_sz);
    params[1] = OSSL_PARAM_construct_end();
    if (!evp_keymgmt_get_params(pkey->keymgmt, pkey->keydata, params)
        || !OSSL_PARAM_modified(params))
        return 0;
    if (out_sz != NULL)
        *out_sz = params[0].return_size;
    return 1;
}

int EVP_PKEY_get_utf8_string_param(EVP_PKEY *pkey, const char *key_name,
                                    char *str, size_t max_buf_sz,
                                    size_t *out_sz)
{
    OSSL_PARAM params[2];

    if (pkey == NULL
        || pkey->keymgmt == NULL
        || pkey->keydata == NULL
        || key_name == NULL)
        return 0;

    params[0] = OSSL_PARAM_construct_utf8_string(key_name, str, max_buf_sz);
    params[1] = OSSL_PARAM_construct_end();
    if (!evp_keymgmt_get_params(pkey->keymgmt, pkey->keydata, params)
        || !OSSL_PARAM_modified(params))
        return 0;
    if (out_sz != NULL)
        *out_sz = params[0].return_size;
    return 1;
}

int EVP_PKEY_get_int_param(EVP_PKEY *pkey, const char *key_name, int *out)
{
    OSSL_PARAM params[2];

    if (pkey == NULL
        || pkey->keymgmt == NULL
        || pkey->keydata == NULL
        || key_name == NULL)
        return 0;

    params[0] = OSSL_PARAM_construct_int(key_name, out);
    params[1] = OSSL_PARAM_construct_end();
    if (!evp_keymgmt_get_params(pkey->keymgmt, pkey->keydata, params)
        || !OSSL_PARAM_modified(params))
        return 0;
    return 1;
}

int EVP_PKEY_get_size_t_param(EVP_PKEY *pkey, const char *key_name, size_t *out)
{
    OSSL_PARAM params[2];

    if (pkey == NULL
        || pkey->keymgmt == NULL
        || pkey->keydata == NULL
        || key_name == NULL)
        return 0;

    params[0] = OSSL_PARAM_construct_size_t(key_name, out);
    params[1] = OSSL_PARAM_construct_end();
    if (!evp_keymgmt_get_params(pkey->keymgmt, pkey->keydata, params)
        || !OSSL_PARAM_modified(params))
        return 0;
    return 1;
}
