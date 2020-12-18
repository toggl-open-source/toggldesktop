/*
 * Copyright 2006-2020 The OpenSSL Project Authors. All Rights Reserved.
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
#include <openssl/x509.h>
#include <openssl/asn1.h>
#include "dh_local.h"
#include <openssl/bn.h>
#include "crypto/asn1.h"
#include "crypto/dh.h"
#include "crypto/evp.h"
#include <openssl/cms.h>
#include <openssl/core_names.h>
#include <openssl/param_build.h>
#include "internal/ffc.h"

/*
 * i2d/d2i like DH parameter functions which use the appropriate routine for
 * PKCS#3 DH or X9.42 DH.
 */

static DH *d2i_dhp(const EVP_PKEY *pkey, const unsigned char **pp,
                   long length)
{
    DH *dh = NULL;
    int is_dhx = (pkey->ameth == &dhx_asn1_meth);

    if (is_dhx)
        dh = d2i_DHxparams(NULL, pp, length);
    else
        dh = d2i_DHparams(NULL, pp, length);

    if (dh != NULL) {
        DH_clear_flags(dh, DH_FLAG_TYPE_MASK);
        DH_set_flags(dh, is_dhx ? DH_FLAG_TYPE_DHX : DH_FLAG_TYPE_DH);
    }
    return dh;
}

static int i2d_dhp(const EVP_PKEY *pkey, const DH *a, unsigned char **pp)
{
    if (pkey->ameth == &dhx_asn1_meth)
        return i2d_DHxparams(a, pp);
    return i2d_DHparams(a, pp);
}

static void int_dh_free(EVP_PKEY *pkey)
{
    DH_free(pkey->pkey.dh);
}

static int dh_pub_decode(EVP_PKEY *pkey, const X509_PUBKEY *pubkey)
{
    const unsigned char *p, *pm;
    int pklen, pmlen;
    int ptype;
    const void *pval;
    const ASN1_STRING *pstr;
    X509_ALGOR *palg;
    ASN1_INTEGER *public_key = NULL;

    DH *dh = NULL;

    if (!X509_PUBKEY_get0_param(NULL, &p, &pklen, &palg, pubkey))
        return 0;
    X509_ALGOR_get0(NULL, &ptype, &pval, palg);

    if (ptype != V_ASN1_SEQUENCE) {
        DHerr(DH_F_DH_PUB_DECODE, DH_R_PARAMETER_ENCODING_ERROR);
        goto err;
    }

    pstr = pval;
    pm = pstr->data;
    pmlen = pstr->length;

    if ((dh = d2i_dhp(pkey, &pm, pmlen)) == NULL) {
        DHerr(DH_F_DH_PUB_DECODE, DH_R_DECODE_ERROR);
        goto err;
    }

    if ((public_key = d2i_ASN1_INTEGER(NULL, &p, pklen)) == NULL) {
        DHerr(DH_F_DH_PUB_DECODE, DH_R_DECODE_ERROR);
        goto err;
    }

    /* We have parameters now set public key */
    if ((dh->pub_key = ASN1_INTEGER_to_BN(public_key, NULL)) == NULL) {
        DHerr(DH_F_DH_PUB_DECODE, DH_R_BN_DECODE_ERROR);
        goto err;
    }

    ASN1_INTEGER_free(public_key);
    EVP_PKEY_assign(pkey, pkey->ameth->pkey_id, dh);
    return 1;

 err:
    ASN1_INTEGER_free(public_key);
    DH_free(dh);
    return 0;
}

static int dh_pub_encode(X509_PUBKEY *pk, const EVP_PKEY *pkey)
{
    DH *dh;
    int ptype;
    unsigned char *penc = NULL;
    int penclen;
    ASN1_STRING *str;
    ASN1_INTEGER *pub_key = NULL;

    dh = pkey->pkey.dh;

    str = ASN1_STRING_new();
    if (str == NULL) {
        DHerr(DH_F_DH_PUB_ENCODE, ERR_R_MALLOC_FAILURE);
        goto err;
    }
    str->length = i2d_dhp(pkey, dh, &str->data);
    if (str->length <= 0) {
        DHerr(DH_F_DH_PUB_ENCODE, ERR_R_MALLOC_FAILURE);
        goto err;
    }
    ptype = V_ASN1_SEQUENCE;

    pub_key = BN_to_ASN1_INTEGER(dh->pub_key, NULL);
    if (pub_key == NULL)
        goto err;

    penclen = i2d_ASN1_INTEGER(pub_key, &penc);

    ASN1_INTEGER_free(pub_key);

    if (penclen <= 0) {
        DHerr(DH_F_DH_PUB_ENCODE, ERR_R_MALLOC_FAILURE);
        goto err;
    }

    if (X509_PUBKEY_set0_param(pk, OBJ_nid2obj(pkey->ameth->pkey_id),
                               ptype, str, penc, penclen))
        return 1;

 err:
    OPENSSL_free(penc);
    ASN1_STRING_free(str);

    return 0;
}

/*
 * PKCS#8 DH is defined in PKCS#11 of all places. It is similar to DH in that
 * the AlgorithmIdentifier contains the parameters, the private key is
 * explicitly included and the pubkey must be recalculated.
 */

static int dh_priv_decode(EVP_PKEY *pkey, const PKCS8_PRIV_KEY_INFO *p8)
{
    const unsigned char *p, *pm;
    int pklen, pmlen;
    int ptype;
    const void *pval;
    const ASN1_STRING *pstr;
    const X509_ALGOR *palg;
    ASN1_INTEGER *privkey = NULL;
    DH *dh = NULL;

    if (!PKCS8_pkey_get0(NULL, &p, &pklen, &palg, p8))
        return 0;

    X509_ALGOR_get0(NULL, &ptype, &pval, palg);

    if (ptype != V_ASN1_SEQUENCE)
        goto decerr;
    if ((privkey = d2i_ASN1_INTEGER(NULL, &p, pklen)) == NULL)
        goto decerr;

    pstr = pval;
    pm = pstr->data;
    pmlen = pstr->length;
    if ((dh = d2i_dhp(pkey, &pm, pmlen)) == NULL)
        goto decerr;

    /* We have parameters now set private key */
    if ((dh->priv_key = BN_secure_new()) == NULL
        || !ASN1_INTEGER_to_BN(privkey, dh->priv_key)) {
        DHerr(DH_F_DH_PRIV_DECODE, DH_R_BN_ERROR);
        goto dherr;
    }
    /* Calculate public key, increments dirty_cnt */
    if (!DH_generate_key(dh))
        goto dherr;

    EVP_PKEY_assign(pkey, pkey->ameth->pkey_id, dh);

    ASN1_STRING_clear_free(privkey);

    return 1;

 decerr:
    DHerr(DH_F_DH_PRIV_DECODE, EVP_R_DECODE_ERROR);
 dherr:
    DH_free(dh);
    ASN1_STRING_clear_free(privkey);
    return 0;
}

static int dh_priv_encode(PKCS8_PRIV_KEY_INFO *p8, const EVP_PKEY *pkey)
{
    ASN1_STRING *params = NULL;
    ASN1_INTEGER *prkey = NULL;
    unsigned char *dp = NULL;
    int dplen;

    params = ASN1_STRING_new();

    if (params == NULL) {
        DHerr(DH_F_DH_PRIV_ENCODE, ERR_R_MALLOC_FAILURE);
        goto err;
    }

    params->length = i2d_dhp(pkey, pkey->pkey.dh, &params->data);
    if (params->length <= 0) {
        DHerr(DH_F_DH_PRIV_ENCODE, ERR_R_MALLOC_FAILURE);
        goto err;
    }
    params->type = V_ASN1_SEQUENCE;

    /* Get private key into integer */
    prkey = BN_to_ASN1_INTEGER(pkey->pkey.dh->priv_key, NULL);

    if (prkey == NULL) {
        DHerr(DH_F_DH_PRIV_ENCODE, DH_R_BN_ERROR);
        goto err;
    }

    dplen = i2d_ASN1_INTEGER(prkey, &dp);

    ASN1_STRING_clear_free(prkey);
    prkey = NULL;

    if (!PKCS8_pkey_set0(p8, OBJ_nid2obj(pkey->ameth->pkey_id), 0,
                         V_ASN1_SEQUENCE, params, dp, dplen))
        goto err;

    return 1;

 err:
    OPENSSL_free(dp);
    ASN1_STRING_free(params);
    ASN1_STRING_clear_free(prkey);
    return 0;
}

static int dh_param_decode(EVP_PKEY *pkey,
                           const unsigned char **pder, int derlen)
{
    DH *dh;

    if ((dh = d2i_dhp(pkey, pder, derlen)) == NULL) {
        DHerr(DH_F_DH_PARAM_DECODE, ERR_R_DH_LIB);
        return 0;
    }
    dh->dirty_cnt++;
    EVP_PKEY_assign(pkey, pkey->ameth->pkey_id, dh);
    return 1;
}

static int dh_param_encode(const EVP_PKEY *pkey, unsigned char **pder)
{
    return i2d_dhp(pkey, pkey->pkey.dh, pder);
}

static int do_dh_print(BIO *bp, const DH *x, int indent, int ptype)
{
    int reason = ERR_R_BUF_LIB;
    const char *ktype = NULL;
    BIGNUM *priv_key, *pub_key;

    if (ptype == 2)
        priv_key = x->priv_key;
    else
        priv_key = NULL;

    if (ptype > 0)
        pub_key = x->pub_key;
    else
        pub_key = NULL;

    if (x->params.p == NULL || (ptype == 2 && priv_key == NULL)
            || (ptype > 0 && pub_key == NULL)) {
        reason = ERR_R_PASSED_NULL_PARAMETER;
        goto err;
    }

    if (ptype == 2)
        ktype = "DH Private-Key";
    else if (ptype == 1)
        ktype = "DH Public-Key";
    else
        ktype = "DH Parameters";

    if (!BIO_indent(bp, indent, 128)
            || BIO_printf(bp, "%s: (%d bit)\n", ktype, DH_bits(x)) <= 0)
        goto err;
    indent += 4;

    if (!ASN1_bn_print(bp, "private-key:", priv_key, NULL, indent))
        goto err;
    if (!ASN1_bn_print(bp, "public-key:", pub_key, NULL, indent))
        goto err;

    if (!ffc_params_print(bp, &x->params, indent))
        goto err;

    if (x->length != 0) {
        if (!BIO_indent(bp, indent, 128)
                || BIO_printf(bp, "recommended-private-length: %d bits\n",
                              (int)x->length) <= 0)
            goto err;
    }

    return 1;

 err:
    DHerr(DH_F_DO_DH_PRINT, reason);
    return 0;
}

static int int_dh_size(const EVP_PKEY *pkey)
{
    return DH_size(pkey->pkey.dh);
}

static int dh_bits(const EVP_PKEY *pkey)
{
    return DH_bits(pkey->pkey.dh);
}

static int dh_security_bits(const EVP_PKEY *pkey)
{
    return DH_security_bits(pkey->pkey.dh);
}

static int dh_cmp_parameters(const EVP_PKEY *a, const EVP_PKEY *b)
{
    return ffc_params_cmp(&a->pkey.dh->params, &a->pkey.dh->params,
                          a->ameth != &dhx_asn1_meth);
}

static int int_dh_param_copy(DH *to, const DH *from, int is_x942)
{
    if (is_x942 == -1)
        is_x942 = (from->params.q != NULL);
    if (!ffc_params_copy(&to->params, &from->params))
        return 0;
    if (!is_x942)
        to->length = from->length;
    to->dirty_cnt++;
    return 1;
}

DH *DHparams_dup(const DH *dh)
{
    DH *ret;
    ret = DH_new();
    if (ret == NULL)
        return NULL;
    if (!int_dh_param_copy(ret, dh, -1)) {
        DH_free(ret);
        return NULL;
    }
    return ret;
}

static int dh_copy_parameters(EVP_PKEY *to, const EVP_PKEY *from)
{
    if (to->pkey.dh == NULL) {
        to->pkey.dh = DH_new();
        if (to->pkey.dh == NULL)
            return 0;
    }
    return int_dh_param_copy(to->pkey.dh, from->pkey.dh,
                             from->ameth == &dhx_asn1_meth);
}

static int dh_missing_parameters(const EVP_PKEY *a)
{
    return a->pkey.dh == NULL
        || a->pkey.dh->params.p == NULL
        || a->pkey.dh->params.g == NULL;
}

static int dh_pub_cmp(const EVP_PKEY *a, const EVP_PKEY *b)
{
    if (dh_cmp_parameters(a, b) == 0)
        return 0;
    if (BN_cmp(b->pkey.dh->pub_key, a->pkey.dh->pub_key) != 0)
        return 0;
    else
        return 1;
}

static int dh_param_print(BIO *bp, const EVP_PKEY *pkey, int indent,
                          ASN1_PCTX *ctx)
{
    return do_dh_print(bp, pkey->pkey.dh, indent, 0);
}

static int dh_public_print(BIO *bp, const EVP_PKEY *pkey, int indent,
                           ASN1_PCTX *ctx)
{
    return do_dh_print(bp, pkey->pkey.dh, indent, 1);
}

static int dh_private_print(BIO *bp, const EVP_PKEY *pkey, int indent,
                            ASN1_PCTX *ctx)
{
    return do_dh_print(bp, pkey->pkey.dh, indent, 2);
}

int DHparams_print(BIO *bp, const DH *x)
{
    return do_dh_print(bp, x, 4, 0);
}

#ifndef OPENSSL_NO_CMS
static int dh_cms_decrypt(CMS_RecipientInfo *ri);
static int dh_cms_encrypt(CMS_RecipientInfo *ri);
#endif

static int dh_pkey_ctrl(EVP_PKEY *pkey, int op, long arg1, void *arg2)
{
    switch (op) {
    case ASN1_PKEY_CTRL_SET1_TLS_ENCPT:
        return dh_buf2key(EVP_PKEY_get0_DH(pkey), arg2, arg1);
    case ASN1_PKEY_CTRL_GET1_TLS_ENCPT:
        return dh_key2buf(EVP_PKEY_get0_DH(pkey), arg2, 0, 1);
    default:
        return -2;
    }
}

static int dhx_pkey_ctrl(EVP_PKEY *pkey, int op, long arg1, void *arg2)
{
    switch (op) {
#ifndef OPENSSL_NO_CMS

    case ASN1_PKEY_CTRL_CMS_ENVELOPE:
        if (arg1 == 1)
            return dh_cms_decrypt(arg2);
        else if (arg1 == 0)
            return dh_cms_encrypt(arg2);
        return -2;

    case ASN1_PKEY_CTRL_CMS_RI_TYPE:
        *(int *)arg2 = CMS_RECIPINFO_AGREE;
        return 1;
#endif
    default:
        return -2;
    }

}

static int dh_pkey_public_check(const EVP_PKEY *pkey)
{
    DH *dh = pkey->pkey.dh;

    if (dh->pub_key == NULL) {
        DHerr(DH_F_DH_PKEY_PUBLIC_CHECK, DH_R_MISSING_PUBKEY);
        return 0;
    }

    return DH_check_pub_key_ex(dh, dh->pub_key);
}

static int dh_pkey_param_check(const EVP_PKEY *pkey)
{
    DH *dh = pkey->pkey.dh;

    return DH_check_ex(dh);
}

static size_t dh_pkey_dirty_cnt(const EVP_PKEY *pkey)
{
    return pkey->pkey.dh->dirty_cnt;
}

static int dh_pkey_export_to(const EVP_PKEY *from, void *to_keydata,
                             EVP_KEYMGMT *to_keymgmt, OPENSSL_CTX *libctx,
                             const char *propq)
{
    DH *dh = from->pkey.dh;
    OSSL_PARAM_BLD *tmpl;
    const BIGNUM *p = DH_get0_p(dh), *g = DH_get0_g(dh), *q = DH_get0_q(dh);
    const BIGNUM *pub_key = DH_get0_pub_key(dh);
    const BIGNUM *priv_key = DH_get0_priv_key(dh);
    OSSL_PARAM *params = NULL;
    int selection = 0;
    int rv = 0;

    /*
     * If the DH method is foreign, then we can't be sure of anything, and
     * can therefore not export or pretend to export.
     */
    if (dh_get_method(dh) != DH_OpenSSL())
        return 0;

    if (p == NULL || g == NULL)
        return 0;

    tmpl = OSSL_PARAM_BLD_new();
    if (tmpl == NULL)
        return 0;
    if (!OSSL_PARAM_BLD_push_BN(tmpl, OSSL_PKEY_PARAM_FFC_P, p)
        || !OSSL_PARAM_BLD_push_BN(tmpl, OSSL_PKEY_PARAM_FFC_G, g))
        goto err;
    if (q != NULL) {
        if (!OSSL_PARAM_BLD_push_BN(tmpl, OSSL_PKEY_PARAM_FFC_Q, q))
            goto err;
    }
    selection |= OSSL_KEYMGMT_SELECT_DOMAIN_PARAMETERS;
    if (pub_key != NULL) {
        if (!OSSL_PARAM_BLD_push_BN(tmpl, OSSL_PKEY_PARAM_PUB_KEY, pub_key))
            goto err;
        selection |= OSSL_KEYMGMT_SELECT_PUBLIC_KEY;
    }
    if (priv_key != NULL) {
        if (!OSSL_PARAM_BLD_push_BN(tmpl, OSSL_PKEY_PARAM_PRIV_KEY,
                                    priv_key))
            goto err;
        selection |= OSSL_KEYMGMT_SELECT_PRIVATE_KEY;
    }

    if ((params = OSSL_PARAM_BLD_to_param(tmpl)) == NULL)
        goto err;

    /* We export, the provider imports */
    rv = evp_keymgmt_import(to_keymgmt, to_keydata, selection, params);

    OSSL_PARAM_BLD_free_params(params);
err:
    OSSL_PARAM_BLD_free(tmpl);
    return rv;
}

static int dh_pkey_import_from_type(const OSSL_PARAM params[], void *vpctx,
                                    int type)
{
    EVP_PKEY_CTX *pctx = vpctx;
    EVP_PKEY *pkey = EVP_PKEY_CTX_get0_pkey(pctx);
    DH *dh = dh_new_with_libctx(pctx->libctx);

    if (dh == NULL) {
        ERR_raise(ERR_LIB_DH, ERR_R_MALLOC_FAILURE);
        return 0;
    }
    DH_clear_flags(dh, DH_FLAG_TYPE_MASK);
    DH_set_flags(dh, type == EVP_PKEY_DH ? DH_FLAG_TYPE_DH : DH_FLAG_TYPE_DHX);

    if (!dh_ffc_params_fromdata(dh, params)
        || !dh_key_fromdata(dh, params)
        || !EVP_PKEY_assign(pkey, type, dh)) {
        DH_free(dh);
        return 0;
    }
    return 1;
}

static int dh_pkey_import_from(const OSSL_PARAM params[], void *vpctx)
{
    return dh_pkey_import_from_type(params, vpctx, EVP_PKEY_DH);
}

static int dhx_pkey_import_from(const OSSL_PARAM params[], void *vpctx)
{
    return dh_pkey_import_from_type(params, vpctx, EVP_PKEY_DHX);
}

const EVP_PKEY_ASN1_METHOD dh_asn1_meth = {
    EVP_PKEY_DH,
    EVP_PKEY_DH,
    0,

    "DH",
    "OpenSSL PKCS#3 DH method",

    dh_pub_decode,
    dh_pub_encode,
    dh_pub_cmp,
    dh_public_print,

    dh_priv_decode,
    dh_priv_encode,
    dh_private_print,

    int_dh_size,
    dh_bits,
    dh_security_bits,

    dh_param_decode,
    dh_param_encode,
    dh_missing_parameters,
    dh_copy_parameters,
    dh_cmp_parameters,
    dh_param_print,
    0,

    int_dh_free,
    dh_pkey_ctrl,

    0, 0, 0, 0, 0,

    0,
    dh_pkey_public_check,
    dh_pkey_param_check,

    0, 0, 0, 0,

    dh_pkey_dirty_cnt,
    dh_pkey_export_to,
    dh_pkey_import_from,
};

const EVP_PKEY_ASN1_METHOD dhx_asn1_meth = {
    EVP_PKEY_DHX,
    EVP_PKEY_DHX,
    0,

    "X9.42 DH",
    "OpenSSL X9.42 DH method",

    dh_pub_decode,
    dh_pub_encode,
    dh_pub_cmp,
    dh_public_print,

    dh_priv_decode,
    dh_priv_encode,
    dh_private_print,

    int_dh_size,
    dh_bits,
    dh_security_bits,

    dh_param_decode,
    dh_param_encode,
    dh_missing_parameters,
    dh_copy_parameters,
    dh_cmp_parameters,
    dh_param_print,
    0,

    int_dh_free,
    dhx_pkey_ctrl,

    0, 0, 0, 0, 0,

    0,
    dh_pkey_public_check,
    dh_pkey_param_check,
    0, 0, 0, 0,
    dh_pkey_dirty_cnt,
    dh_pkey_export_to,
    dhx_pkey_import_from,
};

#ifndef OPENSSL_NO_CMS

static int dh_cms_set_peerkey(EVP_PKEY_CTX *pctx,
                              X509_ALGOR *alg, ASN1_BIT_STRING *pubkey)
{
    const ASN1_OBJECT *aoid;
    int atype;
    const void *aval;
    ASN1_INTEGER *public_key = NULL;
    int rv = 0;
    EVP_PKEY *pkpeer = NULL, *pk = NULL;
    DH *dhpeer = NULL;
    const unsigned char *p;
    int plen;

    X509_ALGOR_get0(&aoid, &atype, &aval, alg);
    if (OBJ_obj2nid(aoid) != NID_dhpublicnumber)
        goto err;
    /* Only absent parameters allowed in RFC XXXX */
    if (atype != V_ASN1_UNDEF && atype == V_ASN1_NULL)
        goto err;

    pk = EVP_PKEY_CTX_get0_pkey(pctx);
    if (pk == NULL)
        goto err;
    if (pk->type != EVP_PKEY_DHX)
        goto err;
    /* Get parameters from parent key */
    dhpeer = DHparams_dup(pk->pkey.dh);
    /* We have parameters now set public key */
    plen = ASN1_STRING_length(pubkey);
    p = ASN1_STRING_get0_data(pubkey);
    if (p == NULL || plen == 0)
        goto err;

    if ((public_key = d2i_ASN1_INTEGER(NULL, &p, plen)) == NULL) {
        DHerr(DH_F_DH_CMS_SET_PEERKEY, DH_R_DECODE_ERROR);
        goto err;
    }

    /* We have parameters now set public key */
    if ((dhpeer->pub_key = ASN1_INTEGER_to_BN(public_key, NULL)) == NULL) {
        DHerr(DH_F_DH_CMS_SET_PEERKEY, DH_R_BN_DECODE_ERROR);
        goto err;
    }

    pkpeer = EVP_PKEY_new();
    if (pkpeer == NULL)
        goto err;
    EVP_PKEY_assign(pkpeer, pk->ameth->pkey_id, dhpeer);
    dhpeer = NULL;
    if (EVP_PKEY_derive_set_peer(pctx, pkpeer) > 0)
        rv = 1;
 err:
    ASN1_INTEGER_free(public_key);
    EVP_PKEY_free(pkpeer);
    DH_free(dhpeer);
    return rv;
}

static int dh_cms_set_shared_info(EVP_PKEY_CTX *pctx, CMS_RecipientInfo *ri)
{
    int rv = 0;

    X509_ALGOR *alg, *kekalg = NULL;
    ASN1_OCTET_STRING *ukm;
    const unsigned char *p;
    unsigned char *dukm = NULL;
    size_t dukmlen = 0;
    int keylen, plen;
    const EVP_CIPHER *kekcipher;
    EVP_CIPHER_CTX *kekctx;

    if (!CMS_RecipientInfo_kari_get0_alg(ri, &alg, &ukm))
        goto err;

    /*
     * For DH we only have one OID permissible. If ever any more get defined
     * we will need something cleverer.
     */
    if (OBJ_obj2nid(alg->algorithm) != NID_id_smime_alg_ESDH) {
        DHerr(DH_F_DH_CMS_SET_SHARED_INFO, DH_R_KDF_PARAMETER_ERROR);
        goto err;
    }

    if (EVP_PKEY_CTX_set_dh_kdf_type(pctx, EVP_PKEY_DH_KDF_X9_42) <= 0)
        goto err;

    if (EVP_PKEY_CTX_set_dh_kdf_md(pctx, EVP_sha1()) <= 0)
        goto err;

    if (alg->parameter->type != V_ASN1_SEQUENCE)
        goto err;

    p = alg->parameter->value.sequence->data;
    plen = alg->parameter->value.sequence->length;
    kekalg = d2i_X509_ALGOR(NULL, &p, plen);
    if (!kekalg)
        goto err;
    kekctx = CMS_RecipientInfo_kari_get0_ctx(ri);
    if (!kekctx)
        goto err;
    kekcipher = EVP_get_cipherbyobj(kekalg->algorithm);
    if (!kekcipher || EVP_CIPHER_mode(kekcipher) != EVP_CIPH_WRAP_MODE)
        goto err;
    if (!EVP_EncryptInit_ex(kekctx, kekcipher, NULL, NULL, NULL))
        goto err;
    if (EVP_CIPHER_asn1_to_param(kekctx, kekalg->parameter) <= 0)
        goto err;

    keylen = EVP_CIPHER_CTX_key_length(kekctx);
    if (EVP_PKEY_CTX_set_dh_kdf_outlen(pctx, keylen) <= 0)
        goto err;
    /* Use OBJ_nid2obj to ensure we use built in OID that isn't freed */
    if (EVP_PKEY_CTX_set0_dh_kdf_oid(pctx,
                                     OBJ_nid2obj(EVP_CIPHER_type(kekcipher)))
        <= 0)
        goto err;

    if (ukm) {
        dukmlen = ASN1_STRING_length(ukm);
        dukm = OPENSSL_memdup(ASN1_STRING_get0_data(ukm), dukmlen);
        if (!dukm)
            goto err;
    }

    if (EVP_PKEY_CTX_set0_dh_kdf_ukm(pctx, dukm, dukmlen) <= 0)
        goto err;
    dukm = NULL;

    rv = 1;
 err:
    X509_ALGOR_free(kekalg);
    OPENSSL_free(dukm);
    return rv;
}

static int dh_cms_decrypt(CMS_RecipientInfo *ri)
{
    EVP_PKEY_CTX *pctx;

    pctx = CMS_RecipientInfo_get0_pkey_ctx(ri);

    if (pctx == NULL)
        return 0;
    /* See if we need to set peer key */
    if (!EVP_PKEY_CTX_get0_peerkey(pctx)) {
        X509_ALGOR *alg;
        ASN1_BIT_STRING *pubkey;
        if (!CMS_RecipientInfo_kari_get0_orig_id(ri, &alg, &pubkey,
                                                 NULL, NULL, NULL))
            return 0;
        if (!alg || !pubkey)
            return 0;
        if (!dh_cms_set_peerkey(pctx, alg, pubkey)) {
            DHerr(DH_F_DH_CMS_DECRYPT, DH_R_PEER_KEY_ERROR);
            return 0;
        }
    }
    /* Set DH derivation parameters and initialise unwrap context */
    if (!dh_cms_set_shared_info(pctx, ri)) {
        DHerr(DH_F_DH_CMS_DECRYPT, DH_R_SHARED_INFO_ERROR);
        return 0;
    }
    return 1;
}

static int dh_cms_encrypt(CMS_RecipientInfo *ri)
{
    EVP_PKEY_CTX *pctx;
    EVP_PKEY *pkey;
    EVP_CIPHER_CTX *ctx;
    int keylen;
    X509_ALGOR *talg, *wrap_alg = NULL;
    const ASN1_OBJECT *aoid;
    ASN1_BIT_STRING *pubkey;
    ASN1_STRING *wrap_str;
    ASN1_OCTET_STRING *ukm;
    unsigned char *penc = NULL, *dukm = NULL;
    int penclen;
    size_t dukmlen = 0;
    int rv = 0;
    int kdf_type, wrap_nid;
    const EVP_MD *kdf_md;

    pctx = CMS_RecipientInfo_get0_pkey_ctx(ri);
    if (pctx == NULL)
        return 0;
    /* Get ephemeral key */
    pkey = EVP_PKEY_CTX_get0_pkey(pctx);
    if (!CMS_RecipientInfo_kari_get0_orig_id(ri, &talg, &pubkey,
                                             NULL, NULL, NULL))
        goto err;
    X509_ALGOR_get0(&aoid, NULL, NULL, talg);
    /* Is everything uninitialised? */
    if (aoid == OBJ_nid2obj(NID_undef)) {
        ASN1_INTEGER *pubk = BN_to_ASN1_INTEGER(pkey->pkey.dh->pub_key, NULL);

        if (pubk == NULL)
            goto err;
        /* Set the key */

        penclen = i2d_ASN1_INTEGER(pubk, &penc);
        ASN1_INTEGER_free(pubk);
        if (penclen <= 0)
            goto err;
        ASN1_STRING_set0(pubkey, penc, penclen);
        pubkey->flags &= ~(ASN1_STRING_FLAG_BITS_LEFT | 0x07);
        pubkey->flags |= ASN1_STRING_FLAG_BITS_LEFT;

        penc = NULL;
        X509_ALGOR_set0(talg, OBJ_nid2obj(NID_dhpublicnumber),
                        V_ASN1_UNDEF, NULL);
    }

    /* See if custom parameters set */
    kdf_type = EVP_PKEY_CTX_get_dh_kdf_type(pctx);
    if (kdf_type <= 0)
        goto err;
    if (!EVP_PKEY_CTX_get_dh_kdf_md(pctx, &kdf_md))
        goto err;

    if (kdf_type == EVP_PKEY_DH_KDF_NONE) {
        kdf_type = EVP_PKEY_DH_KDF_X9_42;
        if (EVP_PKEY_CTX_set_dh_kdf_type(pctx, kdf_type) <= 0)
            goto err;
    } else if (kdf_type != EVP_PKEY_DH_KDF_X9_42)
        /* Unknown KDF */
        goto err;
    if (kdf_md == NULL) {
        /* Only SHA1 supported */
        kdf_md = EVP_sha1();
        if (EVP_PKEY_CTX_set_dh_kdf_md(pctx, kdf_md) <= 0)
            goto err;
    } else if (EVP_MD_type(kdf_md) != NID_sha1)
        /* Unsupported digest */
        goto err;

    if (!CMS_RecipientInfo_kari_get0_alg(ri, &talg, &ukm))
        goto err;

    /* Get wrap NID */
    ctx = CMS_RecipientInfo_kari_get0_ctx(ri);
    wrap_nid = EVP_CIPHER_CTX_type(ctx);
    if (EVP_PKEY_CTX_set0_dh_kdf_oid(pctx, OBJ_nid2obj(wrap_nid)) <= 0)
        goto err;
    keylen = EVP_CIPHER_CTX_key_length(ctx);

    /* Package wrap algorithm in an AlgorithmIdentifier */

    wrap_alg = X509_ALGOR_new();
    if (wrap_alg == NULL)
        goto err;
    wrap_alg->algorithm = OBJ_nid2obj(wrap_nid);
    wrap_alg->parameter = ASN1_TYPE_new();
    if (wrap_alg->parameter == NULL)
        goto err;
    if (EVP_CIPHER_param_to_asn1(ctx, wrap_alg->parameter) <= 0)
        goto err;
    if (ASN1_TYPE_get(wrap_alg->parameter) == NID_undef) {
        ASN1_TYPE_free(wrap_alg->parameter);
        wrap_alg->parameter = NULL;
    }

    if (EVP_PKEY_CTX_set_dh_kdf_outlen(pctx, keylen) <= 0)
        goto err;

    if (ukm) {
        dukmlen = ASN1_STRING_length(ukm);
        dukm = OPENSSL_memdup(ASN1_STRING_get0_data(ukm), dukmlen);
        if (!dukm)
            goto err;
    }

    if (EVP_PKEY_CTX_set0_dh_kdf_ukm(pctx, dukm, dukmlen) <= 0)
        goto err;
    dukm = NULL;

    /*
     * Now need to wrap encoding of wrap AlgorithmIdentifier into parameter
     * of another AlgorithmIdentifier.
     */
    penc = NULL;
    penclen = i2d_X509_ALGOR(wrap_alg, &penc);
    if (penc == NULL || penclen == 0)
        goto err;
    wrap_str = ASN1_STRING_new();
    if (wrap_str == NULL)
        goto err;
    ASN1_STRING_set0(wrap_str, penc, penclen);
    penc = NULL;
    X509_ALGOR_set0(talg, OBJ_nid2obj(NID_id_smime_alg_ESDH),
                    V_ASN1_SEQUENCE, wrap_str);

    rv = 1;

 err:
    OPENSSL_free(penc);
    X509_ALGOR_free(wrap_alg);
    OPENSSL_free(dukm);
    return rv;
}

#endif
