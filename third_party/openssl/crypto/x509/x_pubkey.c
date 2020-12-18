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
#include <openssl/asn1t.h>
#include <openssl/x509.h>
#include "crypto/asn1.h"
#include "crypto/evp.h"
#include "crypto/x509.h"
#include <openssl/rsa.h>
#include <openssl/dsa.h>
#include <openssl/encoder.h>

struct X509_pubkey_st {
    X509_ALGOR *algor;
    ASN1_BIT_STRING *public_key;
    EVP_PKEY *pkey;

    /* extra data for the callback, used by d2i_PUBKEY_ex */
    OPENSSL_CTX *libctx;
    const char *propq;
};

static int x509_pubkey_decode(EVP_PKEY **pk, const X509_PUBKEY *key);

/* Minor tweak to operation: free up EVP_PKEY */
static int pubkey_cb(int operation, ASN1_VALUE **pval, const ASN1_ITEM *it,
                     void *exarg)
{
    if (operation == ASN1_OP_FREE_POST) {
        X509_PUBKEY *pubkey = (X509_PUBKEY *)*pval;
        EVP_PKEY_free(pubkey->pkey);
    } else if (operation == ASN1_OP_D2I_POST) {
        /* Attempt to decode public key and cache in pubkey structure. */
        X509_PUBKEY *pubkey = (X509_PUBKEY *)*pval;
        EVP_PKEY_free(pubkey->pkey);
        pubkey->pkey = NULL;
        /*
         * Opportunistically decode the key but remove any non fatal errors
         * from the queue. Subsequent explicit attempts to decode/use the key
         * will return an appropriate error.
         */
        ERR_set_mark();
        if (x509_pubkey_decode(&pubkey->pkey, pubkey) == -1)
            return 0;
        ERR_pop_to_mark();
    }
    return 1;
}

ASN1_SEQUENCE_cb(X509_PUBKEY, pubkey_cb) = {
        ASN1_SIMPLE(X509_PUBKEY, algor, X509_ALGOR),
        ASN1_SIMPLE(X509_PUBKEY, public_key, ASN1_BIT_STRING)
} ASN1_SEQUENCE_END_cb(X509_PUBKEY, X509_PUBKEY)

IMPLEMENT_ASN1_FUNCTIONS(X509_PUBKEY)
IMPLEMENT_ASN1_DUP_FUNCTION(X509_PUBKEY)

/* TODO should better be called X509_PUBKEY_set1 */
int X509_PUBKEY_set(X509_PUBKEY **x, EVP_PKEY *pkey)
{
    X509_PUBKEY *pk = NULL;

    if (x == NULL)
        return 0;

    if (pkey == NULL)
        goto unsupported;

    if (pkey->ameth != NULL) {
        if ((pk = X509_PUBKEY_new()) == NULL) {
            X509err(X509_F_X509_PUBKEY_SET, ERR_R_MALLOC_FAILURE);
            goto error;
        }
        if (pkey->ameth->pub_encode != NULL) {
            if (!pkey->ameth->pub_encode(pk, pkey)) {
                X509err(X509_F_X509_PUBKEY_SET,
                        X509_R_PUBLIC_KEY_ENCODE_ERROR);
                goto error;
            }
        } else {
            X509err(X509_F_X509_PUBKEY_SET, X509_R_METHOD_NOT_SUPPORTED);
            goto error;
        }
    } else if (pkey->keymgmt != NULL) {
        BIO *bmem = BIO_new(BIO_s_mem());
        const char *encprop = OSSL_ENCODER_PUBKEY_TO_DER_PQ;
        OSSL_ENCODER_CTX *ectx =
            OSSL_ENCODER_CTX_new_by_EVP_PKEY(pkey, encprop);

        if (OSSL_ENCODER_to_bio(ectx, bmem)) {
            const unsigned char *der = NULL;
            long derlen = BIO_get_mem_data(bmem, (char **)&der);

            pk = d2i_X509_PUBKEY(NULL, &der, derlen);
        }

        OSSL_ENCODER_CTX_free(ectx);
        BIO_free(bmem);
    }

    if (pk == NULL)
        goto unsupported;

    X509_PUBKEY_free(*x);
    if (!EVP_PKEY_up_ref(pkey)) {
        X509err(X509_F_X509_PUBKEY_SET, ERR_R_INTERNAL_ERROR);
        goto error;
    }
    *x = pk;

    /*
     * pk->pkey is NULL when using the legacy routine, but is non-NULL when
     * going through the encoder, and for all intents and purposes, it's
     * a perfect copy of |pkey|, just not the same instance.  In that case,
     * we could simply return early, right here.
     * However, in the interest of being cautious leaning on paranoia, some
     * application might very well depend on the passed |pkey| being used
     * and none other, so we spend a few more cycles throwing away the newly
     * created |pk->pkey| and replace it with |pkey|.
     * TODO(3.0) Investigate if it's safe to change to simply return here
     * if |pk->pkey != NULL|.
     */
    if (pk->pkey != NULL)
        EVP_PKEY_free(pk->pkey);

    pk->pkey = pkey;
    return 1;

 unsupported:
    X509err(X509_F_X509_PUBKEY_SET, X509_R_UNSUPPORTED_ALGORITHM);

 error:
    X509_PUBKEY_free(pk);
    return 0;
}

/*
 * Attempt to decode a public key.
 * Returns 1 on success, 0 for a decode failure and -1 for a fatal
 * error e.g. malloc failure.
 */


static int x509_pubkey_decode(EVP_PKEY **ppkey, const X509_PUBKEY *key)
{
    EVP_PKEY *pkey = EVP_PKEY_new();

    if (pkey == NULL) {
        X509err(X509_F_X509_PUBKEY_DECODE, ERR_R_MALLOC_FAILURE);
        return -1;
    }

    if (!EVP_PKEY_set_type(pkey, OBJ_obj2nid(key->algor->algorithm))) {
        X509err(X509_F_X509_PUBKEY_DECODE, X509_R_UNSUPPORTED_ALGORITHM);
        goto error;
    }

    if (pkey->ameth->pub_decode) {
        /*
         * Treat any failure of pub_decode as a decode error. In
         * future we could have different return codes for decode
         * errors and fatal errors such as malloc failure.
         */
        if (!pkey->ameth->pub_decode(pkey, key)) {
            X509err(X509_F_X509_PUBKEY_DECODE, X509_R_PUBLIC_KEY_DECODE_ERROR);
            goto error;
        }
    } else {
        X509err(X509_F_X509_PUBKEY_DECODE, X509_R_METHOD_NOT_SUPPORTED);
        goto error;
    }

    *ppkey = pkey;
    return 1;

 error:
    EVP_PKEY_free(pkey);
    return 0;
}

EVP_PKEY *X509_PUBKEY_get0(const X509_PUBKEY *key)
{
    EVP_PKEY *ret = NULL;

    if (key == NULL || key->public_key == NULL)
        return NULL;

    if (key->pkey != NULL)
        return key->pkey;

    /*
     * When the key ASN.1 is initially parsed an attempt is made to
     * decode the public key and cache the EVP_PKEY structure. If this
     * operation fails the cached value will be NULL. Parsing continues
     * to allow parsing of unknown key types or unsupported forms.
     * We repeat the decode operation so the appropriate errors are left
     * in the queue.
     */
    x509_pubkey_decode(&ret, key);
    /* If decode doesn't fail something bad happened */
    if (ret != NULL) {
        X509err(X509_F_X509_PUBKEY_GET0, ERR_R_INTERNAL_ERROR);
        EVP_PKEY_free(ret);
    }

    return NULL;
}

EVP_PKEY *X509_PUBKEY_get(const X509_PUBKEY *key)
{
    EVP_PKEY *ret = X509_PUBKEY_get0(key);

    if (ret != NULL && !EVP_PKEY_up_ref(ret)) {
        X509err(X509_F_X509_PUBKEY_GET, ERR_R_INTERNAL_ERROR);
        ret = NULL;
    }
    return ret;
}

/*
 * Now three pseudo ASN1 routines that take an EVP_PKEY structure and encode
 * or decode as X509_PUBKEY
 */

EVP_PKEY *d2i_PUBKEY_ex(EVP_PKEY **a, const unsigned char **pp, long length,
                        OPENSSL_CTX *libctx, const char *propq)
{
    X509_PUBKEY *xpk, *xpk2 = NULL, **pxpk = NULL;
    EVP_PKEY *pktmp = NULL;
    const unsigned char *q;

    q = *pp;

    /*
     * If libctx or propq are non-NULL, we take advantage of the reuse
     * feature.  It's not generally recommended, but is safe enough for
     * newly created structures.
     */
    if (libctx != NULL || propq != NULL) {
        xpk2 = OPENSSL_zalloc(sizeof(*xpk2));
        if (xpk2 == NULL) {
            ERR_raise(ERR_LIB_X509, ERR_R_MALLOC_FAILURE);
            return NULL;
        }
        xpk2->libctx = libctx;
        xpk2->propq = propq;
        pxpk = &xpk2;
    }
    xpk = d2i_X509_PUBKEY(pxpk, &q, length);
    if (xpk == NULL)
        goto end;
    pktmp = X509_PUBKEY_get(xpk);
    X509_PUBKEY_free(xpk);
    xpk2 = NULL;                 /* We know that xpk == xpk2 */
    if (pktmp == NULL)
        goto end;
    *pp = q;
    if (a != NULL) {
        EVP_PKEY_free(*a);
        *a = pktmp;
    }
 end:
    X509_PUBKEY_free(xpk2);
    return pktmp;
}

EVP_PKEY *d2i_PUBKEY(EVP_PKEY **a, const unsigned char **pp, long length)
{
    return d2i_PUBKEY_ex(a, pp, length, NULL, NULL);
}

int i2d_PUBKEY(const EVP_PKEY *a, unsigned char **pp)
{
    int ret = -1;

    if (a == NULL)
        return 0;
    if (a->ameth != NULL) {
        X509_PUBKEY *xpk = NULL;

        if ((xpk = X509_PUBKEY_new()) == NULL)
            return -1;

        /* pub_encode() only encode parameters, not the key itself */
        if (a->ameth->pub_encode != NULL && a->ameth->pub_encode(xpk, a)) {
            xpk->pkey = (EVP_PKEY *)a;
            ret = i2d_X509_PUBKEY(xpk, pp);
            xpk->pkey = NULL;
        }
        X509_PUBKEY_free(xpk);
    } else if (a->keymgmt != NULL) {
        const char *encprop = OSSL_ENCODER_PUBKEY_TO_DER_PQ;
        OSSL_ENCODER_CTX *ctx =
            OSSL_ENCODER_CTX_new_by_EVP_PKEY(a, encprop);
        BIO *out = BIO_new(BIO_s_mem());
        BUF_MEM *buf = NULL;

        if (ctx != NULL
            && out != NULL
            && OSSL_ENCODER_CTX_get_encoder(ctx) != NULL
            && OSSL_ENCODER_to_bio(ctx, out)
            && BIO_get_mem_ptr(out, &buf) > 0) {
            ret = buf->length;

            if (pp != NULL) {
                if (*pp == NULL) {
                    *pp = (unsigned char *)buf->data;
                    buf->length = 0;
                    buf->data = NULL;
                } else {
                    memcpy(*pp, buf->data, ret);
                    *pp += ret;
                }
            }
        }
        BIO_free(out);
        OSSL_ENCODER_CTX_free(ctx);
    }

    return ret;
}

/*
 * The following are equivalents but which return RSA and DSA keys
 */
#ifndef OPENSSL_NO_RSA
RSA *d2i_RSA_PUBKEY(RSA **a, const unsigned char **pp, long length)
{
    EVP_PKEY *pkey;
    RSA *key;
    const unsigned char *q;

    q = *pp;
    pkey = d2i_PUBKEY(NULL, &q, length);
    if (pkey == NULL)
        return NULL;
    key = EVP_PKEY_get1_RSA(pkey);
    EVP_PKEY_free(pkey);
    if (key == NULL)
        return NULL;
    *pp = q;
    if (a != NULL) {
        RSA_free(*a);
        *a = key;
    }
    return key;
}

int i2d_RSA_PUBKEY(const RSA *a, unsigned char **pp)
{
    EVP_PKEY *pktmp;
    int ret;
    if (!a)
        return 0;
    pktmp = EVP_PKEY_new();
    if (pktmp == NULL) {
        ASN1err(ASN1_F_I2D_RSA_PUBKEY, ERR_R_MALLOC_FAILURE);
        return -1;
    }
    (void)EVP_PKEY_assign_RSA(pktmp, (RSA *)a);
    ret = i2d_PUBKEY(pktmp, pp);
    pktmp->pkey.ptr = NULL;
    EVP_PKEY_free(pktmp);
    return ret;
}
#endif

#ifndef OPENSSL_NO_DSA
DSA *d2i_DSA_PUBKEY(DSA **a, const unsigned char **pp, long length)
{
    EVP_PKEY *pkey;
    DSA *key;
    const unsigned char *q;

    q = *pp;
    pkey = d2i_PUBKEY(NULL, &q, length);
    if (pkey == NULL)
        return NULL;
    key = EVP_PKEY_get1_DSA(pkey);
    EVP_PKEY_free(pkey);
    if (key == NULL)
        return NULL;
    *pp = q;
    if (a != NULL) {
        DSA_free(*a);
        *a = key;
    }
    return key;
}

int i2d_DSA_PUBKEY(const DSA *a, unsigned char **pp)
{
    EVP_PKEY *pktmp;
    int ret;
    if (!a)
        return 0;
    pktmp = EVP_PKEY_new();
    if (pktmp == NULL) {
        ASN1err(ASN1_F_I2D_DSA_PUBKEY, ERR_R_MALLOC_FAILURE);
        return -1;
    }
    (void)EVP_PKEY_assign_DSA(pktmp, (DSA *)a);
    ret = i2d_PUBKEY(pktmp, pp);
    pktmp->pkey.ptr = NULL;
    EVP_PKEY_free(pktmp);
    return ret;
}
#endif

#ifndef OPENSSL_NO_EC
EC_KEY *d2i_EC_PUBKEY(EC_KEY **a, const unsigned char **pp, long length)
{
    EVP_PKEY *pkey;
    EC_KEY *key;
    const unsigned char *q;

    q = *pp;
    pkey = d2i_PUBKEY(NULL, &q, length);
    if (pkey == NULL)
        return NULL;
    key = EVP_PKEY_get1_EC_KEY(pkey);
    EVP_PKEY_free(pkey);
    if (key == NULL)
        return NULL;
    *pp = q;
    if (a != NULL) {
        EC_KEY_free(*a);
        *a = key;
    }
    return key;
}

int i2d_EC_PUBKEY(const EC_KEY *a, unsigned char **pp)
{
    EVP_PKEY *pktmp;
    int ret;

    if (a == NULL)
        return 0;
    if ((pktmp = EVP_PKEY_new()) == NULL) {
        ASN1err(ASN1_F_I2D_EC_PUBKEY, ERR_R_MALLOC_FAILURE);
        return -1;
    }
    (void)EVP_PKEY_assign_EC_KEY(pktmp, (EC_KEY *)a);
    ret = i2d_PUBKEY(pktmp, pp);
    pktmp->pkey.ptr = NULL;
    EVP_PKEY_free(pktmp);
    return ret;
}
#endif

int X509_PUBKEY_set0_param(X509_PUBKEY *pub, ASN1_OBJECT *aobj,
                           int ptype, void *pval,
                           unsigned char *penc, int penclen)
{
    if (!X509_ALGOR_set0(pub->algor, aobj, ptype, pval))
        return 0;
    if (penc) {
        OPENSSL_free(pub->public_key->data);
        pub->public_key->data = penc;
        pub->public_key->length = penclen;
        /* Set number of unused bits to zero */
        pub->public_key->flags &= ~(ASN1_STRING_FLAG_BITS_LEFT | 0x07);
        pub->public_key->flags |= ASN1_STRING_FLAG_BITS_LEFT;
    }
    return 1;
}

int X509_PUBKEY_get0_param(ASN1_OBJECT **ppkalg,
                           const unsigned char **pk, int *ppklen,
                           X509_ALGOR **pa, const X509_PUBKEY *pub)
{
    if (ppkalg)
        *ppkalg = pub->algor->algorithm;
    if (pk) {
        *pk = pub->public_key->data;
        *ppklen = pub->public_key->length;
    }
    if (pa)
        *pa = pub->algor;
    return 1;
}

ASN1_BIT_STRING *X509_get0_pubkey_bitstr(const X509 *x)
{
    if (x == NULL)
        return NULL;
    return x->cert_info.key->public_key;
}

/* Returns 1 for equal, 0, for non-equal, < 0 on error */
int X509_PUBKEY_eq(const X509_PUBKEY *a, const X509_PUBKEY *b)
{
    X509_ALGOR *algA, *algB;
    EVP_PKEY *pA, *pB;

    if (a == b)
        return 1;
    if (a == NULL || b == NULL)
        return 0;
    if (!X509_PUBKEY_get0_param(NULL, NULL, NULL, &algA, a) || algA == NULL
        || !X509_PUBKEY_get0_param(NULL, NULL, NULL, &algB, b) || algB == NULL)
        return -2;
    if (X509_ALGOR_cmp(algA, algB) != 0)
        return 0;
    if ((pA = X509_PUBKEY_get0(a)) == NULL
        || (pB = X509_PUBKEY_get0(b)) == NULL)
        return -2;
    return EVP_PKEY_eq(pA, pB);
}

int X509_PUBKEY_get0_libctx(OPENSSL_CTX **plibctx, const char **ppropq,
                            const X509_PUBKEY *key)
{
    if (plibctx)
        *plibctx = key->libctx;
    if (ppropq)
        *ppropq = key->propq;
    return 1;
}
