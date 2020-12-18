/*
 * Copyright 2006-2020 The OpenSSL Project Authors. All Rights Reserved.
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
#include "internal/cryptlib.h"
#include <openssl/asn1t.h>
#include <openssl/x509.h>
#include <openssl/bn.h>
#include <openssl/cms.h>
#include <openssl/core_names.h>
#include <openssl/param_build.h>
#include "crypto/asn1.h"
#include "crypto/evp.h"
#include "crypto/rsa.h"
#include "rsa_local.h"

#ifndef OPENSSL_NO_CMS
static int rsa_cms_sign(CMS_SignerInfo *si);
static int rsa_cms_verify(CMS_SignerInfo *si);
static int rsa_cms_decrypt(CMS_RecipientInfo *ri);
static int rsa_cms_encrypt(CMS_RecipientInfo *ri);
#endif

static RSA_PSS_PARAMS *rsa_pss_decode(const X509_ALGOR *alg);
static int rsa_sync_to_pss_params_30(RSA *rsa);

/* Set any parameters associated with pkey */
static int rsa_param_encode(const EVP_PKEY *pkey,
                            ASN1_STRING **pstr, int *pstrtype)
{
    const RSA *rsa = pkey->pkey.rsa;

    *pstr = NULL;
    /* If RSA it's just NULL type */
    if (RSA_test_flags(rsa, RSA_FLAG_TYPE_MASK) != RSA_FLAG_TYPE_RSASSAPSS) {
        *pstrtype = V_ASN1_NULL;
        return 1;
    }
    /* If no PSS parameters we omit parameters entirely */
    if (rsa->pss == NULL) {
        *pstrtype = V_ASN1_UNDEF;
        return 1;
    }
    /* Encode PSS parameters */
    if (ASN1_item_pack(rsa->pss, ASN1_ITEM_rptr(RSA_PSS_PARAMS), pstr) == NULL)
        return 0;

    *pstrtype = V_ASN1_SEQUENCE;
    return 1;
}
/* Decode any parameters and set them in RSA structure */
static int rsa_param_decode(RSA *rsa, const X509_ALGOR *alg)
{
    const ASN1_OBJECT *algoid;
    const void *algp;
    int algptype;

    X509_ALGOR_get0(&algoid, &algptype, &algp, alg);
    if (OBJ_obj2nid(algoid) != EVP_PKEY_RSA_PSS)
        return 1;
    if (algptype == V_ASN1_UNDEF)
        return 1;
    if (algptype != V_ASN1_SEQUENCE) {
        RSAerr(RSA_F_RSA_PARAM_DECODE, RSA_R_INVALID_PSS_PARAMETERS);
        return 0;
    }
    rsa->pss = rsa_pss_decode(alg);
    if (rsa->pss == NULL)
        return 0;
    if (!rsa_sync_to_pss_params_30(rsa))
        return 0;
    return 1;
}

static int rsa_pub_encode(X509_PUBKEY *pk, const EVP_PKEY *pkey)
{
    unsigned char *penc = NULL;
    int penclen;
    ASN1_STRING *str;
    int strtype;

    if (!rsa_param_encode(pkey, &str, &strtype))
        return 0;
    penclen = i2d_RSAPublicKey(pkey->pkey.rsa, &penc);
    if (penclen <= 0)
        return 0;
    if (X509_PUBKEY_set0_param(pk, OBJ_nid2obj(pkey->ameth->pkey_id),
                               strtype, str, penc, penclen))
        return 1;

    OPENSSL_free(penc);
    return 0;
}

static int rsa_pub_decode(EVP_PKEY *pkey, const X509_PUBKEY *pubkey)
{
    const unsigned char *p;
    int pklen;
    X509_ALGOR *alg;
    RSA *rsa = NULL;

    if (!X509_PUBKEY_get0_param(NULL, &p, &pklen, &alg, pubkey))
        return 0;
    if ((rsa = d2i_RSAPublicKey(NULL, &p, pklen)) == NULL) {
        RSAerr(RSA_F_RSA_PUB_DECODE, ERR_R_RSA_LIB);
        return 0;
    }
    if (!rsa_param_decode(rsa, alg)) {
        RSA_free(rsa);
        return 0;
    }

    RSA_clear_flags(rsa, RSA_FLAG_TYPE_MASK);
    switch (pkey->ameth->pkey_id) {
    case EVP_PKEY_RSA:
        RSA_set_flags(rsa, RSA_FLAG_TYPE_RSA);
        break;
    case EVP_PKEY_RSA_PSS:
        RSA_set_flags(rsa, RSA_FLAG_TYPE_RSASSAPSS);
        break;
    default:
        /* Leave the type bits zero */
        break;
    }

    if (!EVP_PKEY_assign(pkey, pkey->ameth->pkey_id, rsa)) {
        RSA_free(rsa);
        return 0;
    }
    return 1;
}

static int rsa_pub_cmp(const EVP_PKEY *a, const EVP_PKEY *b)
{
    if (BN_cmp(b->pkey.rsa->n, a->pkey.rsa->n) != 0
        || BN_cmp(b->pkey.rsa->e, a->pkey.rsa->e) != 0)
        return 0;
    return 1;
}

static int old_rsa_priv_decode(EVP_PKEY *pkey,
                               const unsigned char **pder, int derlen)
{
    RSA *rsa;

    if ((rsa = d2i_RSAPrivateKey(NULL, pder, derlen)) == NULL) {
        RSAerr(RSA_F_OLD_RSA_PRIV_DECODE, ERR_R_RSA_LIB);
        return 0;
    }
    EVP_PKEY_assign(pkey, pkey->ameth->pkey_id, rsa);
    return 1;
}

static int old_rsa_priv_encode(const EVP_PKEY *pkey, unsigned char **pder)
{
    return i2d_RSAPrivateKey(pkey->pkey.rsa, pder);
}

static int rsa_priv_encode(PKCS8_PRIV_KEY_INFO *p8, const EVP_PKEY *pkey)
{
    unsigned char *rk = NULL;
    int rklen;
    ASN1_STRING *str;
    int strtype;

    if (!rsa_param_encode(pkey, &str, &strtype))
        return 0;
    rklen = i2d_RSAPrivateKey(pkey->pkey.rsa, &rk);

    if (rklen <= 0) {
        RSAerr(RSA_F_RSA_PRIV_ENCODE, ERR_R_MALLOC_FAILURE);
        ASN1_STRING_free(str);
        return 0;
    }

    if (!PKCS8_pkey_set0(p8, OBJ_nid2obj(pkey->ameth->pkey_id), 0,
                         strtype, str, rk, rklen)) {
        RSAerr(RSA_F_RSA_PRIV_ENCODE, ERR_R_MALLOC_FAILURE);
        ASN1_STRING_free(str);
        return 0;
    }

    return 1;
}

static int rsa_priv_decode(EVP_PKEY *pkey, const PKCS8_PRIV_KEY_INFO *p8)
{
    const unsigned char *p;
    RSA *rsa;
    int pklen;
    const X509_ALGOR *alg;

    if (!PKCS8_pkey_get0(NULL, &p, &pklen, &alg, p8))
        return 0;
    rsa = d2i_RSAPrivateKey(NULL, &p, pklen);
    if (rsa == NULL) {
        RSAerr(RSA_F_RSA_PRIV_DECODE, ERR_R_RSA_LIB);
        return 0;
    }
    if (!rsa_param_decode(rsa, alg)) {
        RSA_free(rsa);
        return 0;
    }

    RSA_clear_flags(rsa, RSA_FLAG_TYPE_MASK);
    switch (pkey->ameth->pkey_id) {
    case EVP_PKEY_RSA:
        RSA_set_flags(rsa, RSA_FLAG_TYPE_RSA);
        break;
    case EVP_PKEY_RSA_PSS:
        RSA_set_flags(rsa, RSA_FLAG_TYPE_RSASSAPSS);
        break;
    default:
        /* Leave the type bits zero */
        break;
    }

    EVP_PKEY_assign(pkey, pkey->ameth->pkey_id, rsa);
    return 1;
}

static int int_rsa_size(const EVP_PKEY *pkey)
{
    return RSA_size(pkey->pkey.rsa);
}

static int rsa_bits(const EVP_PKEY *pkey)
{
    return BN_num_bits(pkey->pkey.rsa->n);
}

static int rsa_security_bits(const EVP_PKEY *pkey)
{
    return RSA_security_bits(pkey->pkey.rsa);
}

static void int_rsa_free(EVP_PKEY *pkey)
{
    RSA_free(pkey->pkey.rsa);
}

static X509_ALGOR *rsa_mgf1_decode(X509_ALGOR *alg)
{
    if (OBJ_obj2nid(alg->algorithm) != NID_mgf1)
        return NULL;
    return ASN1_TYPE_unpack_sequence(ASN1_ITEM_rptr(X509_ALGOR),
                                     alg->parameter);
}

static int rsa_pss_param_print(BIO *bp, int pss_key, RSA_PSS_PARAMS *pss,
                               int indent)
{
    int rv = 0;
    X509_ALGOR *maskHash = NULL;

    if (!BIO_indent(bp, indent, 128))
        goto err;
    if (pss_key) {
        if (pss == NULL) {
            if (BIO_puts(bp, "No PSS parameter restrictions\n") <= 0)
                return 0;
            return 1;
        } else {
            if (BIO_puts(bp, "PSS parameter restrictions:") <= 0)
                return 0;
        }
    } else if (pss == NULL) {
        if (BIO_puts(bp,"(INVALID PSS PARAMETERS)\n") <= 0)
            return 0;
        return 1;
    }
    if (BIO_puts(bp, "\n") <= 0)
        goto err;
    if (pss_key)
        indent += 2;
    if (!BIO_indent(bp, indent, 128))
        goto err;
    if (BIO_puts(bp, "Hash Algorithm: ") <= 0)
        goto err;

    if (pss->hashAlgorithm) {
        if (i2a_ASN1_OBJECT(bp, pss->hashAlgorithm->algorithm) <= 0)
            goto err;
    } else if (BIO_puts(bp, "sha1 (default)") <= 0) {
        goto err;
    }

    if (BIO_puts(bp, "\n") <= 0)
        goto err;

    if (!BIO_indent(bp, indent, 128))
        goto err;

    if (BIO_puts(bp, "Mask Algorithm: ") <= 0)
        goto err;
    if (pss->maskGenAlgorithm) {
        if (i2a_ASN1_OBJECT(bp, pss->maskGenAlgorithm->algorithm) <= 0)
            goto err;
        if (BIO_puts(bp, " with ") <= 0)
            goto err;
        maskHash = rsa_mgf1_decode(pss->maskGenAlgorithm);
        if (maskHash != NULL) {
            if (i2a_ASN1_OBJECT(bp, maskHash->algorithm) <= 0)
                goto err;
        } else if (BIO_puts(bp, "INVALID") <= 0) {
            goto err;
        }
    } else if (BIO_puts(bp, "mgf1 with sha1 (default)") <= 0) {
        goto err;
    }
    BIO_puts(bp, "\n");

    if (!BIO_indent(bp, indent, 128))
        goto err;
    if (BIO_printf(bp, "%s Salt Length: 0x", pss_key ? "Minimum" : "") <= 0)
        goto err;
    if (pss->saltLength) {
        if (i2a_ASN1_INTEGER(bp, pss->saltLength) <= 0)
            goto err;
    } else if (BIO_puts(bp, "14 (default)") <= 0) {
        goto err;
    }
    BIO_puts(bp, "\n");

    if (!BIO_indent(bp, indent, 128))
        goto err;
    if (BIO_puts(bp, "Trailer Field: 0x") <= 0)
        goto err;
    if (pss->trailerField) {
        if (i2a_ASN1_INTEGER(bp, pss->trailerField) <= 0)
            goto err;
    } else if (BIO_puts(bp, "BC (default)") <= 0) {
        goto err;
    }
    BIO_puts(bp, "\n");

    rv = 1;

 err:
    X509_ALGOR_free(maskHash);
    return rv;

}

static int pkey_rsa_print(BIO *bp, const EVP_PKEY *pkey, int off, int priv)
{
    const RSA *x = pkey->pkey.rsa;
    char *str;
    const char *s;
    int ret = 0, mod_len = 0, ex_primes;

    if (x->n != NULL)
        mod_len = BN_num_bits(x->n);
    ex_primes = sk_RSA_PRIME_INFO_num(x->prime_infos);

    if (!BIO_indent(bp, off, 128))
        goto err;

    if (BIO_printf(bp, "%s ", pkey_is_pss(pkey) ?  "RSA-PSS" : "RSA") <= 0)
        goto err;

    if (priv && x->d) {
        if (BIO_printf(bp, "Private-Key: (%d bit, %d primes)\n",
                       mod_len, ex_primes <= 0 ? 2 : ex_primes + 2) <= 0)
            goto err;
        str = "modulus:";
        s = "publicExponent:";
    } else {
        if (BIO_printf(bp, "Public-Key: (%d bit)\n", mod_len) <= 0)
            goto err;
        str = "Modulus:";
        s = "Exponent:";
    }
    if (!ASN1_bn_print(bp, str, x->n, NULL, off))
        goto err;
    if (!ASN1_bn_print(bp, s, x->e, NULL, off))
        goto err;
    if (priv) {
        int i;

        if (!ASN1_bn_print(bp, "privateExponent:", x->d, NULL, off))
            goto err;
        if (!ASN1_bn_print(bp, "prime1:", x->p, NULL, off))
            goto err;
        if (!ASN1_bn_print(bp, "prime2:", x->q, NULL, off))
            goto err;
        if (!ASN1_bn_print(bp, "exponent1:", x->dmp1, NULL, off))
            goto err;
        if (!ASN1_bn_print(bp, "exponent2:", x->dmq1, NULL, off))
            goto err;
        if (!ASN1_bn_print(bp, "coefficient:", x->iqmp, NULL, off))
            goto err;
        for (i = 0; i < sk_RSA_PRIME_INFO_num(x->prime_infos); i++) {
            /* print multi-prime info */
            BIGNUM *bn = NULL;
            RSA_PRIME_INFO *pinfo;
            int j;

            pinfo = sk_RSA_PRIME_INFO_value(x->prime_infos, i);
            for (j = 0; j < 3; j++) {
                if (!BIO_indent(bp, off, 128))
                    goto err;
                switch (j) {
                case 0:
                    if (BIO_printf(bp, "prime%d:", i + 3) <= 0)
                        goto err;
                    bn = pinfo->r;
                    break;
                case 1:
                    if (BIO_printf(bp, "exponent%d:", i + 3) <= 0)
                        goto err;
                    bn = pinfo->d;
                    break;
                case 2:
                    if (BIO_printf(bp, "coefficient%d:", i + 3) <= 0)
                        goto err;
                    bn = pinfo->t;
                    break;
                default:
                    break;
                }
                if (!ASN1_bn_print(bp, "", bn, NULL, off))
                    goto err;
            }
        }
    }
    if (pkey_is_pss(pkey) && !rsa_pss_param_print(bp, 1, x->pss, off))
        goto err;
    ret = 1;
 err:
    return ret;
}

static int rsa_pub_print(BIO *bp, const EVP_PKEY *pkey, int indent,
                         ASN1_PCTX *ctx)
{
    return pkey_rsa_print(bp, pkey, indent, 0);
}

static int rsa_priv_print(BIO *bp, const EVP_PKEY *pkey, int indent,
                          ASN1_PCTX *ctx)
{
    return pkey_rsa_print(bp, pkey, indent, 1);
}

static RSA_PSS_PARAMS *rsa_pss_decode(const X509_ALGOR *alg)
{
    RSA_PSS_PARAMS *pss;

    pss = ASN1_TYPE_unpack_sequence(ASN1_ITEM_rptr(RSA_PSS_PARAMS),
                                    alg->parameter);

    if (pss == NULL)
        return NULL;

    if (pss->maskGenAlgorithm != NULL) {
        pss->maskHash = rsa_mgf1_decode(pss->maskGenAlgorithm);
        if (pss->maskHash == NULL) {
            RSA_PSS_PARAMS_free(pss);
            return NULL;
        }
    }

    return pss;
}

static int rsa_sig_print(BIO *bp, const X509_ALGOR *sigalg,
                         const ASN1_STRING *sig, int indent, ASN1_PCTX *pctx)
{
    if (OBJ_obj2nid(sigalg->algorithm) == EVP_PKEY_RSA_PSS) {
        int rv;
        RSA_PSS_PARAMS *pss = rsa_pss_decode(sigalg);

        rv = rsa_pss_param_print(bp, 0, pss, indent);
        RSA_PSS_PARAMS_free(pss);
        if (!rv)
            return 0;
    } else if (BIO_puts(bp, "\n") <= 0) {
        return 0;
    }
    if (sig)
        return X509_signature_dump(bp, sig, indent);
    return 1;
}

static int rsa_pkey_ctrl(EVP_PKEY *pkey, int op, long arg1, void *arg2)
{
    X509_ALGOR *alg = NULL;
    const EVP_MD *md;
    const EVP_MD *mgf1md;
    int min_saltlen;

    switch (op) {

    case ASN1_PKEY_CTRL_PKCS7_SIGN:
        if (arg1 == 0)
            PKCS7_SIGNER_INFO_get0_algs(arg2, NULL, NULL, &alg);
        break;

    case ASN1_PKEY_CTRL_PKCS7_ENCRYPT:
        if (pkey_is_pss(pkey))
            return -2;
        if (arg1 == 0)
            PKCS7_RECIP_INFO_get0_alg(arg2, &alg);
        break;
#ifndef OPENSSL_NO_CMS
    case ASN1_PKEY_CTRL_CMS_SIGN:
        if (arg1 == 0)
            return rsa_cms_sign(arg2);
        else if (arg1 == 1)
            return rsa_cms_verify(arg2);
        break;

    case ASN1_PKEY_CTRL_CMS_ENVELOPE:
        if (pkey_is_pss(pkey))
            return -2;
        if (arg1 == 0)
            return rsa_cms_encrypt(arg2);
        else if (arg1 == 1)
            return rsa_cms_decrypt(arg2);
        break;

    case ASN1_PKEY_CTRL_CMS_RI_TYPE:
        if (pkey_is_pss(pkey))
            return -2;
        *(int *)arg2 = CMS_RECIPINFO_TRANS;
        return 1;
#endif

    case ASN1_PKEY_CTRL_DEFAULT_MD_NID:
        if (pkey->pkey.rsa->pss != NULL) {
            if (!rsa_pss_get_param(pkey->pkey.rsa->pss, &md, &mgf1md,
                                   &min_saltlen)) {
                RSAerr(0, ERR_R_INTERNAL_ERROR);
                return 0;
            }
            *(int *)arg2 = EVP_MD_type(md);
            /* Return of 2 indicates this MD is mandatory */
            return 2;
        }
        *(int *)arg2 = NID_sha256;
        return 1;

    default:
        return -2;

    }

    if (alg)
        X509_ALGOR_set0(alg, OBJ_nid2obj(NID_rsaEncryption), V_ASN1_NULL, 0);

    return 1;

}

/* allocate and set algorithm ID from EVP_MD, default SHA1 */
static int rsa_md_to_algor(X509_ALGOR **palg, const EVP_MD *md)
{
    if (md == NULL || EVP_MD_type(md) == NID_sha1)
        return 1;
    *palg = X509_ALGOR_new();
    if (*palg == NULL)
        return 0;
    X509_ALGOR_set_md(*palg, md);
    return 1;
}

/* Allocate and set MGF1 algorithm ID from EVP_MD */
static int rsa_md_to_mgf1(X509_ALGOR **palg, const EVP_MD *mgf1md)
{
    X509_ALGOR *algtmp = NULL;
    ASN1_STRING *stmp = NULL;

    *palg = NULL;
    if (mgf1md == NULL || EVP_MD_type(mgf1md) == NID_sha1)
        return 1;
    /* need to embed algorithm ID inside another */
    if (!rsa_md_to_algor(&algtmp, mgf1md))
        goto err;
    if (ASN1_item_pack(algtmp, ASN1_ITEM_rptr(X509_ALGOR), &stmp) == NULL)
         goto err;
    *palg = X509_ALGOR_new();
    if (*palg == NULL)
        goto err;
    X509_ALGOR_set0(*palg, OBJ_nid2obj(NID_mgf1), V_ASN1_SEQUENCE, stmp);
    stmp = NULL;
 err:
    ASN1_STRING_free(stmp);
    X509_ALGOR_free(algtmp);
    if (*palg)
        return 1;
    return 0;
}

/* convert algorithm ID to EVP_MD, default SHA1 */
static const EVP_MD *rsa_algor_to_md(X509_ALGOR *alg)
{
    const EVP_MD *md;

    if (!alg)
        return EVP_sha1();
    md = EVP_get_digestbyobj(alg->algorithm);
    if (md == NULL)
        RSAerr(RSA_F_RSA_ALGOR_TO_MD, RSA_R_UNKNOWN_DIGEST);
    return md;
}

/*
 * Convert EVP_PKEY_CTX in PSS mode into corresponding algorithm parameter,
 * suitable for setting an AlgorithmIdentifier.
 */

static RSA_PSS_PARAMS *rsa_ctx_to_pss(EVP_PKEY_CTX *pkctx)
{
    const EVP_MD *sigmd, *mgf1md;
    EVP_PKEY *pk = EVP_PKEY_CTX_get0_pkey(pkctx);
    RSA *rsa = EVP_PKEY_get0_RSA(pk);
    int saltlen;

    if (EVP_PKEY_CTX_get_signature_md(pkctx, &sigmd) <= 0)
        return NULL;
    if (EVP_PKEY_CTX_get_rsa_mgf1_md(pkctx, &mgf1md) <= 0)
        return NULL;
    if (!EVP_PKEY_CTX_get_rsa_pss_saltlen(pkctx, &saltlen))
        return NULL;
    if (saltlen == -1) {
        saltlen = EVP_MD_size(sigmd);
    } else if (saltlen == -2 || saltlen == -3) {
        saltlen = RSA_size(rsa) - EVP_MD_size(sigmd) - 2;
        if ((EVP_PKEY_bits(pk) & 0x7) == 1)
            saltlen--;
        if (saltlen < 0)
            return NULL;
    }

    return rsa_pss_params_create(sigmd, mgf1md, saltlen);
}

RSA_PSS_PARAMS *rsa_pss_params_create(const EVP_MD *sigmd,
                                      const EVP_MD *mgf1md, int saltlen)
{
    RSA_PSS_PARAMS *pss = RSA_PSS_PARAMS_new();

    if (pss == NULL)
        goto err;
    if (saltlen != 20) {
        pss->saltLength = ASN1_INTEGER_new();
        if (pss->saltLength == NULL)
            goto err;
        if (!ASN1_INTEGER_set(pss->saltLength, saltlen))
            goto err;
    }
    if (!rsa_md_to_algor(&pss->hashAlgorithm, sigmd))
        goto err;
    if (mgf1md == NULL)
        mgf1md = sigmd;
    if (!rsa_md_to_mgf1(&pss->maskGenAlgorithm, mgf1md))
        goto err;
    if (!rsa_md_to_algor(&pss->maskHash, mgf1md))
        goto err;
    return pss;
 err:
    RSA_PSS_PARAMS_free(pss);
    return NULL;
}

static ASN1_STRING *rsa_ctx_to_pss_string(EVP_PKEY_CTX *pkctx)
{
    RSA_PSS_PARAMS *pss = rsa_ctx_to_pss(pkctx);
    ASN1_STRING *os;

    if (pss == NULL)
        return NULL;

    os = ASN1_item_pack(pss, ASN1_ITEM_rptr(RSA_PSS_PARAMS), NULL);
    RSA_PSS_PARAMS_free(pss);
    return os;
}

/*
 * From PSS AlgorithmIdentifier set public key parameters. If pkey isn't NULL
 * then the EVP_MD_CTX is setup and initialised. If it is NULL parameters are
 * passed to pkctx instead.
 */

static int rsa_pss_to_ctx(EVP_MD_CTX *ctx, EVP_PKEY_CTX *pkctx,
                          const X509_ALGOR *sigalg, EVP_PKEY *pkey)
{
    int rv = -1;
    int saltlen;
    const EVP_MD *mgf1md = NULL, *md = NULL;
    RSA_PSS_PARAMS *pss;

    /* Sanity check: make sure it is PSS */
    if (OBJ_obj2nid(sigalg->algorithm) != EVP_PKEY_RSA_PSS) {
        RSAerr(RSA_F_RSA_PSS_TO_CTX, RSA_R_UNSUPPORTED_SIGNATURE_TYPE);
        return -1;
    }
    /* Decode PSS parameters */
    pss = rsa_pss_decode(sigalg);

    if (!rsa_pss_get_param(pss, &md, &mgf1md, &saltlen)) {
        RSAerr(RSA_F_RSA_PSS_TO_CTX, RSA_R_INVALID_PSS_PARAMETERS);
        goto err;
    }

    /* We have all parameters now set up context */
    if (pkey) {
        if (!EVP_DigestVerifyInit(ctx, &pkctx, md, NULL, pkey))
            goto err;
    } else {
        const EVP_MD *checkmd;
        if (EVP_PKEY_CTX_get_signature_md(pkctx, &checkmd) <= 0)
            goto err;
        if (EVP_MD_type(md) != EVP_MD_type(checkmd)) {
            RSAerr(RSA_F_RSA_PSS_TO_CTX, RSA_R_DIGEST_DOES_NOT_MATCH);
            goto err;
        }
    }

    if (EVP_PKEY_CTX_set_rsa_padding(pkctx, RSA_PKCS1_PSS_PADDING) <= 0)
        goto err;

    if (EVP_PKEY_CTX_set_rsa_pss_saltlen(pkctx, saltlen) <= 0)
        goto err;

    if (EVP_PKEY_CTX_set_rsa_mgf1_md(pkctx, mgf1md) <= 0)
        goto err;
    /* Carry on */
    rv = 1;

 err:
    RSA_PSS_PARAMS_free(pss);
    return rv;
}

static int rsa_pss_verify_param(const EVP_MD **pmd, const EVP_MD **pmgf1md,
                                int *psaltlen, int *ptrailerField)
{
    if (psaltlen != NULL && *psaltlen < 0) {
        ERR_raise(ERR_LIB_RSA, RSA_R_INVALID_SALT_LENGTH);
        return 0;
    }
    /*
     * low-level routines support only trailer field 0xbc (value 1) and
     * PKCS#1 says we should reject any other value anyway.
     */
    if (ptrailerField != NULL && *ptrailerField != 1) {
        ERR_raise(ERR_LIB_RSA, RSA_R_INVALID_TRAILER);
        return 0;
    }
    return 1;
}

static int rsa_pss_get_param_unverified(const RSA_PSS_PARAMS *pss,
                                        const EVP_MD **pmd,
                                        const EVP_MD **pmgf1md,
                                        int *psaltlen, int *ptrailerField)
{
    RSA_PSS_PARAMS_30 pss_params;

    /* Get the defaults from the ONE place */
    (void)rsa_pss_params_30_set_defaults(&pss_params);

    if (pss == NULL)
        return 0;
    *pmd = rsa_algor_to_md(pss->hashAlgorithm);
    if (*pmd == NULL)
        return 0;
    *pmgf1md = rsa_algor_to_md(pss->maskHash);
    if (*pmgf1md == NULL)
        return 0;
    if (pss->saltLength)
        *psaltlen = ASN1_INTEGER_get(pss->saltLength);
    else
        *psaltlen = rsa_pss_params_30_saltlen(&pss_params);
    if (pss->trailerField)
        *ptrailerField = ASN1_INTEGER_get(pss->trailerField);
    else
        *ptrailerField = rsa_pss_params_30_trailerfield(&pss_params);;

    return 1;
}

int rsa_pss_get_param(const RSA_PSS_PARAMS *pss, const EVP_MD **pmd,
                      const EVP_MD **pmgf1md, int *psaltlen)
{
    /*
     * Callers do not care about the trailer field, and yet, we must
     * pass it from get_param to verify_param, since the latter checks
     * its value.
     *
     * When callers start caring, it's a simple thing to add another
     * argument to this function.
     */
    int trailerField = 0;

    return rsa_pss_get_param_unverified(pss, pmd, pmgf1md, psaltlen,
                                        &trailerField)
        && rsa_pss_verify_param(pmd, pmgf1md, psaltlen, &trailerField);
}

static int rsa_sync_to_pss_params_30(RSA *rsa)
{
    if (rsa != NULL && rsa->pss != NULL) {
        const EVP_MD *md = NULL, *mgf1md = NULL;
        int md_nid, mgf1md_nid, saltlen, trailerField;
        RSA_PSS_PARAMS_30 pss_params;

        /*
         * We don't care about the validity of the fields here, we just
         * want to synchronise values.  Verifying here makes it impossible
         * to even read a key with invalid values, making it hard to test
         * a bad situation.
         *
         * Other routines use rsa_pss_get_param(), so the values will be
         * checked, eventually.
         */
        if (!rsa_pss_get_param_unverified(rsa->pss, &md, &mgf1md,
                                          &saltlen, &trailerField))
            return 0;
        md_nid = EVP_MD_type(md);
        mgf1md_nid = EVP_MD_type(mgf1md);
        if (!rsa_pss_params_30_set_defaults(&pss_params)
            || !rsa_pss_params_30_set_hashalg(&pss_params, md_nid)
            || !rsa_pss_params_30_set_maskgenhashalg(&pss_params, mgf1md_nid)
            || !rsa_pss_params_30_set_saltlen(&pss_params, saltlen)
            || !rsa_pss_params_30_set_trailerfield(&pss_params, trailerField))
            return 0;
        rsa->pss_params = pss_params;
    }
    return 1;
}

#ifndef OPENSSL_NO_CMS
static int rsa_cms_verify(CMS_SignerInfo *si)
{
    int nid, nid2;
    X509_ALGOR *alg;
    EVP_PKEY_CTX *pkctx = CMS_SignerInfo_get0_pkey_ctx(si);

    CMS_SignerInfo_get0_algs(si, NULL, NULL, NULL, &alg);
    nid = OBJ_obj2nid(alg->algorithm);
    if (nid == EVP_PKEY_RSA_PSS)
        return rsa_pss_to_ctx(NULL, pkctx, alg, NULL);
    /* Only PSS allowed for PSS keys */
    if (pkey_ctx_is_pss(pkctx)) {
        RSAerr(RSA_F_RSA_CMS_VERIFY, RSA_R_ILLEGAL_OR_UNSUPPORTED_PADDING_MODE);
        return 0;
    }
    if (nid == NID_rsaEncryption)
        return 1;
    /* Workaround for some implementation that use a signature OID */
    if (OBJ_find_sigid_algs(nid, NULL, &nid2)) {
        if (nid2 == NID_rsaEncryption)
            return 1;
    }
    return 0;
}
#endif

/*
 * Customised RSA item verification routine. This is called when a signature
 * is encountered requiring special handling. We currently only handle PSS.
 */

static int rsa_item_verify(EVP_MD_CTX *ctx, const ASN1_ITEM *it,
                           const void *asn, const X509_ALGOR *sigalg,
                           const ASN1_BIT_STRING *sig, EVP_PKEY *pkey)
{
    /* Sanity check: make sure it is PSS */
    if (OBJ_obj2nid(sigalg->algorithm) != EVP_PKEY_RSA_PSS) {
        RSAerr(RSA_F_RSA_ITEM_VERIFY, RSA_R_UNSUPPORTED_SIGNATURE_TYPE);
        return -1;
    }
    if (rsa_pss_to_ctx(ctx, NULL, sigalg, pkey) > 0) {
        /* Carry on */
        return 2;
    }
    return -1;
}

#ifndef OPENSSL_NO_CMS
static int rsa_cms_sign(CMS_SignerInfo *si)
{
    int pad_mode = RSA_PKCS1_PADDING;
    X509_ALGOR *alg;
    EVP_PKEY_CTX *pkctx = CMS_SignerInfo_get0_pkey_ctx(si);
    ASN1_STRING *os = NULL;

    CMS_SignerInfo_get0_algs(si, NULL, NULL, NULL, &alg);
    if (pkctx) {
        if (EVP_PKEY_CTX_get_rsa_padding(pkctx, &pad_mode) <= 0)
            return 0;
    }
    if (pad_mode == RSA_PKCS1_PADDING) {
        X509_ALGOR_set0(alg, OBJ_nid2obj(NID_rsaEncryption), V_ASN1_NULL, 0);
        return 1;
    }
    /* We don't support it */
    if (pad_mode != RSA_PKCS1_PSS_PADDING)
        return 0;
    os = rsa_ctx_to_pss_string(pkctx);
    if (!os)
        return 0;
    X509_ALGOR_set0(alg, OBJ_nid2obj(EVP_PKEY_RSA_PSS), V_ASN1_SEQUENCE, os);
    return 1;
}
#endif

static int rsa_item_sign(EVP_MD_CTX *ctx, const ASN1_ITEM *it, const void *asn,
                         X509_ALGOR *alg1, X509_ALGOR *alg2,
                         ASN1_BIT_STRING *sig)
{
    int pad_mode;
    EVP_PKEY_CTX *pkctx = EVP_MD_CTX_pkey_ctx(ctx);

    if (EVP_PKEY_CTX_get_rsa_padding(pkctx, &pad_mode) <= 0)
        return 0;
    if (pad_mode == RSA_PKCS1_PADDING)
        return 2;
    if (pad_mode == RSA_PKCS1_PSS_PADDING) {
        ASN1_STRING *os1 = NULL;
        os1 = rsa_ctx_to_pss_string(pkctx);
        if (!os1)
            return 0;
        /* Duplicate parameters if we have to */
        if (alg2) {
            ASN1_STRING *os2 = ASN1_STRING_dup(os1);
            if (!os2) {
                ASN1_STRING_free(os1);
                return 0;
            }
            X509_ALGOR_set0(alg2, OBJ_nid2obj(EVP_PKEY_RSA_PSS),
                            V_ASN1_SEQUENCE, os2);
        }
        X509_ALGOR_set0(alg1, OBJ_nid2obj(EVP_PKEY_RSA_PSS),
                        V_ASN1_SEQUENCE, os1);
        return 3;
    }
    return 2;
}

static int rsa_sig_info_set(X509_SIG_INFO *siginf, const X509_ALGOR *sigalg,
                            const ASN1_STRING *sig)
{
    int rv = 0;
    int mdnid, saltlen;
    uint32_t flags;
    const EVP_MD *mgf1md = NULL, *md = NULL;
    RSA_PSS_PARAMS *pss;
    int secbits;

    /* Sanity check: make sure it is PSS */
    if (OBJ_obj2nid(sigalg->algorithm) != EVP_PKEY_RSA_PSS)
        return 0;
    /* Decode PSS parameters */
    pss = rsa_pss_decode(sigalg);
    if (!rsa_pss_get_param(pss, &md, &mgf1md, &saltlen))
        goto err;
    mdnid = EVP_MD_type(md);
    /*
     * For TLS need SHA256, SHA384 or SHA512, digest and MGF1 digest must
     * match and salt length must equal digest size
     */
    if ((mdnid == NID_sha256 || mdnid == NID_sha384 || mdnid == NID_sha512)
            && mdnid == EVP_MD_type(mgf1md) && saltlen == EVP_MD_size(md))
        flags = X509_SIG_INFO_TLS;
    else
        flags = 0;
    /* Note: security bits half number of digest bits */
    secbits = EVP_MD_size(md) * 4;
    /*
     * SHA1 and MD5 are known to be broken. Reduce security bits so that
     * they're no longer accepted at security level 1. The real values don't
     * really matter as long as they're lower than 80, which is our security
     * level 1.
     * https://eprint.iacr.org/2020/014 puts a chosen-prefix attack for SHA1 at
     * 2^63.4
     * https://documents.epfl.ch/users/l/le/lenstra/public/papers/lat.pdf
     * puts a chosen-prefix attack for MD5 at 2^39.
     */
    if (mdnid == NID_sha1)
        secbits = 64;
    else if (mdnid == NID_md5_sha1)
        secbits = 68;
    else if (mdnid == NID_md5)
        secbits = 39;
    X509_SIG_INFO_set(siginf, mdnid, EVP_PKEY_RSA_PSS, secbits,
                      flags);
    rv = 1;
    err:
    RSA_PSS_PARAMS_free(pss);
    return rv;
}

#ifndef OPENSSL_NO_CMS
static RSA_OAEP_PARAMS *rsa_oaep_decode(const X509_ALGOR *alg)
{
    RSA_OAEP_PARAMS *oaep;

    oaep = ASN1_TYPE_unpack_sequence(ASN1_ITEM_rptr(RSA_OAEP_PARAMS),
                                     alg->parameter);

    if (oaep == NULL)
        return NULL;

    if (oaep->maskGenFunc != NULL) {
        oaep->maskHash = rsa_mgf1_decode(oaep->maskGenFunc);
        if (oaep->maskHash == NULL) {
            RSA_OAEP_PARAMS_free(oaep);
            return NULL;
        }
    }
    return oaep;
}

static int rsa_cms_decrypt(CMS_RecipientInfo *ri)
{
    EVP_PKEY_CTX *pkctx;
    X509_ALGOR *cmsalg;
    int nid;
    int rv = -1;
    unsigned char *label = NULL;
    int labellen = 0;
    const EVP_MD *mgf1md = NULL, *md = NULL;
    RSA_OAEP_PARAMS *oaep;

    pkctx = CMS_RecipientInfo_get0_pkey_ctx(ri);
    if (pkctx == NULL)
        return 0;
    if (!CMS_RecipientInfo_ktri_get0_algs(ri, NULL, NULL, &cmsalg))
        return -1;
    nid = OBJ_obj2nid(cmsalg->algorithm);
    if (nid == NID_rsaEncryption)
        return 1;
    if (nid != NID_rsaesOaep) {
        RSAerr(RSA_F_RSA_CMS_DECRYPT, RSA_R_UNSUPPORTED_ENCRYPTION_TYPE);
        return -1;
    }
    /* Decode OAEP parameters */
    oaep = rsa_oaep_decode(cmsalg);

    if (oaep == NULL) {
        RSAerr(RSA_F_RSA_CMS_DECRYPT, RSA_R_INVALID_OAEP_PARAMETERS);
        goto err;
    }

    mgf1md = rsa_algor_to_md(oaep->maskHash);
    if (mgf1md == NULL)
        goto err;
    md = rsa_algor_to_md(oaep->hashFunc);
    if (md == NULL)
        goto err;

    if (oaep->pSourceFunc != NULL) {
        X509_ALGOR *plab = oaep->pSourceFunc;

        if (OBJ_obj2nid(plab->algorithm) != NID_pSpecified) {
            RSAerr(RSA_F_RSA_CMS_DECRYPT, RSA_R_UNSUPPORTED_LABEL_SOURCE);
            goto err;
        }
        if (plab->parameter->type != V_ASN1_OCTET_STRING) {
            RSAerr(RSA_F_RSA_CMS_DECRYPT, RSA_R_INVALID_LABEL);
            goto err;
        }

        label = plab->parameter->value.octet_string->data;
        /* Stop label being freed when OAEP parameters are freed */
        plab->parameter->value.octet_string->data = NULL;
        labellen = plab->parameter->value.octet_string->length;
    }

    if (EVP_PKEY_CTX_set_rsa_padding(pkctx, RSA_PKCS1_OAEP_PADDING) <= 0)
        goto err;
    if (EVP_PKEY_CTX_set_rsa_oaep_md(pkctx, md) <= 0)
        goto err;
    if (EVP_PKEY_CTX_set_rsa_mgf1_md(pkctx, mgf1md) <= 0)
        goto err;
    if (label != NULL
            && EVP_PKEY_CTX_set0_rsa_oaep_label(pkctx, label, labellen) <= 0)
        goto err;
    /* Carry on */
    rv = 1;

 err:
    RSA_OAEP_PARAMS_free(oaep);
    return rv;
}

static int rsa_cms_encrypt(CMS_RecipientInfo *ri)
{
    const EVP_MD *md, *mgf1md;
    RSA_OAEP_PARAMS *oaep = NULL;
    ASN1_STRING *os = NULL;
    X509_ALGOR *alg;
    EVP_PKEY_CTX *pkctx = CMS_RecipientInfo_get0_pkey_ctx(ri);
    int pad_mode = RSA_PKCS1_PADDING, rv = 0, labellen;
    unsigned char *label;

    if (CMS_RecipientInfo_ktri_get0_algs(ri, NULL, NULL, &alg) <= 0)
        return 0;
    if (pkctx) {
        if (EVP_PKEY_CTX_get_rsa_padding(pkctx, &pad_mode) <= 0)
            return 0;
    }
    if (pad_mode == RSA_PKCS1_PADDING) {
        X509_ALGOR_set0(alg, OBJ_nid2obj(NID_rsaEncryption), V_ASN1_NULL, 0);
        return 1;
    }
    /* Not supported */
    if (pad_mode != RSA_PKCS1_OAEP_PADDING)
        return 0;
    if (EVP_PKEY_CTX_get_rsa_oaep_md(pkctx, &md) <= 0)
        goto err;
    if (EVP_PKEY_CTX_get_rsa_mgf1_md(pkctx, &mgf1md) <= 0)
        goto err;
    labellen = EVP_PKEY_CTX_get0_rsa_oaep_label(pkctx, &label);
    if (labellen < 0)
        goto err;
    oaep = RSA_OAEP_PARAMS_new();
    if (oaep == NULL)
        goto err;
    if (!rsa_md_to_algor(&oaep->hashFunc, md))
        goto err;
    if (!rsa_md_to_mgf1(&oaep->maskGenFunc, mgf1md))
        goto err;
    if (labellen > 0) {
        ASN1_OCTET_STRING *los;
        oaep->pSourceFunc = X509_ALGOR_new();
        if (oaep->pSourceFunc == NULL)
            goto err;
        los = ASN1_OCTET_STRING_new();
        if (los == NULL)
            goto err;
        if (!ASN1_OCTET_STRING_set(los, label, labellen)) {
            ASN1_OCTET_STRING_free(los);
            goto err;
        }
        X509_ALGOR_set0(oaep->pSourceFunc, OBJ_nid2obj(NID_pSpecified),
                        V_ASN1_OCTET_STRING, los);
    }
    /* create string with pss parameter encoding. */
    if (!ASN1_item_pack(oaep, ASN1_ITEM_rptr(RSA_OAEP_PARAMS), &os))
         goto err;
    X509_ALGOR_set0(alg, OBJ_nid2obj(NID_rsaesOaep), V_ASN1_SEQUENCE, os);
    os = NULL;
    rv = 1;
 err:
    RSA_OAEP_PARAMS_free(oaep);
    ASN1_STRING_free(os);
    return rv;
}
#endif

static int rsa_pkey_check(const EVP_PKEY *pkey)
{
    return RSA_check_key_ex(pkey->pkey.rsa, NULL);
}

static size_t rsa_pkey_dirty_cnt(const EVP_PKEY *pkey)
{
    return pkey->pkey.rsa->dirty_cnt;
}

/*
 * For the moment, we trust the call path, where keys going through
 * rsa_pkey_export_to() match a KEYMGMT for the "RSA" keytype, while
 * keys going through rsa_pss_pkey_export_to() match a KEYMGMT for the
 * "RSA-PSS" keytype.
 * TODO(3.0) Investigate whether we should simply continue to trust the
 * call path, or if we should strengthen this function by checking that
 * |rsa_type| matches the RSA key subtype.  The latter requires ensuring
 * that the type flag for the RSA key is properly set by other functions
 * in this file.
 */
static int rsa_int_export_to(const EVP_PKEY *from, int rsa_type,
                             void *to_keydata, EVP_KEYMGMT *to_keymgmt,
                             OPENSSL_CTX *libctx, const char *propq)
{
    RSA *rsa = from->pkey.rsa;
    OSSL_PARAM_BLD *tmpl = OSSL_PARAM_BLD_new();
    OSSL_PARAM *params = NULL;
    int selection = 0;
    int rv = 0;

    if (tmpl == NULL)
        return 0;
    /*
     * If the RSA method is foreign, then we can't be sure of anything, and
     * can therefore not export or pretend to export.
     */
    if (RSA_get_method(rsa) != RSA_PKCS1_OpenSSL())
        goto err;

    /* Public parameters must always be present */
    if (RSA_get0_n(rsa) == NULL || RSA_get0_e(rsa) == NULL)
        goto err;

    if (!rsa_todata(rsa, tmpl, NULL))
        goto err;

    selection |= OSSL_KEYMGMT_SELECT_PUBLIC_KEY;
    if (RSA_get0_d(rsa) != NULL)
        selection |= OSSL_KEYMGMT_SELECT_PRIVATE_KEY;

    if (rsa->pss != NULL) {
        const EVP_MD *md = NULL, *mgf1md = NULL;
        int md_nid, mgf1md_nid, saltlen, trailerfield;
        RSA_PSS_PARAMS_30 pss_params;

        if (!rsa_pss_get_param_unverified(rsa->pss, &md, &mgf1md,
                                          &saltlen, &trailerfield))
            goto err;
        md_nid = EVP_MD_type(md);
        mgf1md_nid = EVP_MD_type(mgf1md);
        if (!rsa_pss_params_30_set_defaults(&pss_params)
            || !rsa_pss_params_30_set_hashalg(&pss_params, md_nid)
            || !rsa_pss_params_30_set_maskgenhashalg(&pss_params, mgf1md_nid)
            || !rsa_pss_params_30_set_saltlen(&pss_params, saltlen)
            || !rsa_pss_params_30_todata(&pss_params, propq, tmpl, NULL))
            goto err;
        selection |= OSSL_KEYMGMT_SELECT_OTHER_PARAMETERS;
    }

    if ((params = OSSL_PARAM_BLD_to_param(tmpl)) == NULL)
        goto err;

    /* We export, the provider imports */
    rv = evp_keymgmt_import(to_keymgmt, to_keydata, selection, params);

 err:
    OSSL_PARAM_BLD_free_params(params);
    OSSL_PARAM_BLD_free(tmpl);
    return rv;
}

static int rsa_int_import_from(const OSSL_PARAM params[], void *vpctx,
                               int rsa_type)
{
    EVP_PKEY_CTX *pctx = vpctx;
    EVP_PKEY *pkey = EVP_PKEY_CTX_get0_pkey(pctx);
    RSA *rsa = rsa_new_with_ctx(pctx->libctx);
    RSA_PSS_PARAMS_30 rsa_pss_params = { 0, };
    int ok = 0;

    if (rsa == NULL) {
        ERR_raise(ERR_LIB_DH, ERR_R_MALLOC_FAILURE);
        return 0;
    }

    RSA_clear_flags(rsa, RSA_FLAG_TYPE_MASK);
    RSA_set_flags(rsa, rsa_type);

    if (!rsa_pss_params_30_fromdata(&rsa_pss_params, params, pctx->libctx))
        goto err;

    switch (rsa_type) {
    case RSA_FLAG_TYPE_RSA:
        /*
         * Were PSS parameters filled in?
         * In that case, something's wrong
         */
        if (!rsa_pss_params_30_is_unrestricted(&rsa_pss_params))
            goto err;
        break;
    case RSA_FLAG_TYPE_RSASSAPSS:
        /*
         * Were PSS parameters filled in?  In that case, create the old
         * RSA_PSS_PARAMS structure.  Otherwise, this is an unrestricted key.
         */
        if (!rsa_pss_params_30_is_unrestricted(&rsa_pss_params)) {
            /* Create the older RSA_PSS_PARAMS from RSA_PSS_PARAMS_30 data */
            int mdnid = rsa_pss_params_30_hashalg(&rsa_pss_params);
            int mgf1mdnid = rsa_pss_params_30_maskgenhashalg(&rsa_pss_params);
            int saltlen = rsa_pss_params_30_saltlen(&rsa_pss_params);
            const EVP_MD *md = EVP_get_digestbynid(mdnid);
            const EVP_MD *mgf1md = EVP_get_digestbynid(mgf1mdnid);

            if ((rsa->pss = rsa_pss_params_create(md, mgf1md, saltlen)) == NULL)
                goto err;
        }
        break;
    default:
        /* RSA key sub-types we don't know how to handle yet */
        goto err;
    }

    if (!rsa_fromdata(rsa, params))
        goto err;

    switch (rsa_type) {
    case RSA_FLAG_TYPE_RSA:
        ok = EVP_PKEY_assign_RSA(pkey, rsa);
        break;
    case RSA_FLAG_TYPE_RSASSAPSS:
        ok = EVP_PKEY_assign(pkey, EVP_PKEY_RSA_PSS, rsa);
        break;
    }

 err:
    if (!ok)
        RSA_free(rsa);
    return ok;
}

static int rsa_pkey_export_to(const EVP_PKEY *from, void *to_keydata,
                              EVP_KEYMGMT *to_keymgmt, OPENSSL_CTX *libctx,
                              const char *propq)
{
    return rsa_int_export_to(from, RSA_FLAG_TYPE_RSA, to_keydata,
                             to_keymgmt, libctx, propq);
}

static int rsa_pss_pkey_export_to(const EVP_PKEY *from, void *to_keydata,
                                  EVP_KEYMGMT *to_keymgmt, OPENSSL_CTX *libctx,
                                  const char *propq)
{
    return rsa_int_export_to(from, RSA_FLAG_TYPE_RSASSAPSS, to_keydata,
                             to_keymgmt, libctx, propq);
}

static int rsa_pkey_import_from(const OSSL_PARAM params[], void *vpctx)
{
    return rsa_int_import_from(params, vpctx, RSA_FLAG_TYPE_RSA);
}

static int rsa_pss_pkey_import_from(const OSSL_PARAM params[], void *vpctx)
{
    return rsa_int_import_from(params, vpctx, RSA_FLAG_TYPE_RSASSAPSS);
}

const EVP_PKEY_ASN1_METHOD rsa_asn1_meths[2] = {
    {
     EVP_PKEY_RSA,
     EVP_PKEY_RSA,
     ASN1_PKEY_SIGPARAM_NULL,

     "RSA",
     "OpenSSL RSA method",

     rsa_pub_decode,
     rsa_pub_encode,
     rsa_pub_cmp,
     rsa_pub_print,

     rsa_priv_decode,
     rsa_priv_encode,
     rsa_priv_print,

     int_rsa_size,
     rsa_bits,
     rsa_security_bits,

     0, 0, 0, 0, 0, 0,

     rsa_sig_print,
     int_rsa_free,
     rsa_pkey_ctrl,
     old_rsa_priv_decode,
     old_rsa_priv_encode,
     rsa_item_verify,
     rsa_item_sign,
     rsa_sig_info_set,
     rsa_pkey_check,

     0, 0,
     0, 0, 0, 0,

     rsa_pkey_dirty_cnt,
     rsa_pkey_export_to,
     rsa_pkey_import_from
    },

    {
     EVP_PKEY_RSA2,
     EVP_PKEY_RSA,
     ASN1_PKEY_ALIAS}
};

const EVP_PKEY_ASN1_METHOD rsa_pss_asn1_meth = {
     EVP_PKEY_RSA_PSS,
     EVP_PKEY_RSA_PSS,
     ASN1_PKEY_SIGPARAM_NULL,

     "RSA-PSS",
     "OpenSSL RSA-PSS method",

     rsa_pub_decode,
     rsa_pub_encode,
     rsa_pub_cmp,
     rsa_pub_print,

     rsa_priv_decode,
     rsa_priv_encode,
     rsa_priv_print,

     int_rsa_size,
     rsa_bits,
     rsa_security_bits,

     0, 0, 0, 0, 0, 0,

     rsa_sig_print,
     int_rsa_free,
     rsa_pkey_ctrl,
     0, 0,
     rsa_item_verify,
     rsa_item_sign,
     0,
     rsa_pkey_check,

     0, 0,
     0, 0, 0, 0,

     rsa_pkey_dirty_cnt,
     rsa_pss_pkey_export_to,
     rsa_pss_pkey_import_from
};
