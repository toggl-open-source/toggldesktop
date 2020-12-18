/*
 * Copyright 2006-2020 The OpenSSL Project Authors. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://www.openssl.org/source/license.html
 */

/*
 * ECDSA low level APIs are deprecated for public use, but still ok for
 * internal use.
 */
#include "internal/deprecated.h"

#include "internal/cryptlib.h"
#include <openssl/asn1t.h>
#include <openssl/ec.h>
#include <openssl/evp.h>
#include "crypto/evp.h"
#include "crypto/sm2.h"
#include "crypto/sm2err.h"
#include "crypto/ec.h"

/* EC pkey context structure */

typedef struct {
    /* message digest */
    const EVP_MD *md;
    /* Distinguishing Identifier, ISO/IEC 15946-3, FIPS 196 */
    uint8_t *id;
    size_t id_len;
    /* id_set indicates if the 'id' field is set (1) or not (0) */
    int id_set;
} SM2_PKEY_CTX;

static int pkey_sm2_init(EVP_PKEY_CTX *ctx)
{
    SM2_PKEY_CTX *smctx;

    if ((smctx = OPENSSL_zalloc(sizeof(*smctx))) == NULL) {
        SM2err(SM2_F_PKEY_SM2_INIT, ERR_R_MALLOC_FAILURE);
        return 0;
    }

    ctx->data = smctx;
    return 1;
}

static void pkey_sm2_cleanup(EVP_PKEY_CTX *ctx)
{
    SM2_PKEY_CTX *smctx = ctx->data;

    if (smctx != NULL) {
        OPENSSL_free(smctx->id);
        OPENSSL_free(smctx);
        ctx->data = NULL;
    }
}

static int pkey_sm2_copy(EVP_PKEY_CTX *dst, const EVP_PKEY_CTX *src)
{
    SM2_PKEY_CTX *dctx, *sctx;

    if (!pkey_sm2_init(dst))
        return 0;
    sctx = src->data;
    dctx = dst->data;
    if (sctx->id != NULL) {
        dctx->id = OPENSSL_malloc(sctx->id_len);
        if (dctx->id == NULL) {
            SM2err(SM2_F_PKEY_SM2_COPY, ERR_R_MALLOC_FAILURE);
            pkey_sm2_cleanup(dst);
            return 0;
        }
        memcpy(dctx->id, sctx->id, sctx->id_len);
    }
    dctx->id_len = sctx->id_len;
    dctx->id_set = sctx->id_set;
    dctx->md = sctx->md;

    return 1;
}

static int pkey_sm2_sign(EVP_PKEY_CTX *ctx, unsigned char *sig, size_t *siglen,
                         const unsigned char *tbs, size_t tbslen)
{
    int ret;
    unsigned int sltmp;
    EC_KEY *ec = ctx->pkey->pkey.ec;
    const int sig_sz = ECDSA_size(ctx->pkey->pkey.ec);

    if (sig_sz <= 0) {
        return 0;
    }

    if (sig == NULL) {
        *siglen = (size_t)sig_sz;
        return 1;
    }

    if (*siglen < (size_t)sig_sz) {
        SM2err(SM2_F_PKEY_SM2_SIGN, SM2_R_BUFFER_TOO_SMALL);
        return 0;
    }

    ret = sm2_sign(tbs, tbslen, sig, &sltmp, ec);

    if (ret <= 0)
        return ret;
    *siglen = (size_t)sltmp;
    return 1;
}

static int pkey_sm2_verify(EVP_PKEY_CTX *ctx,
                           const unsigned char *sig, size_t siglen,
                           const unsigned char *tbs, size_t tbslen)
{
    EC_KEY *ec = ctx->pkey->pkey.ec;

    return sm2_verify(tbs, tbslen, sig, siglen, ec);
}

static int pkey_sm2_encrypt(EVP_PKEY_CTX *ctx,
                            unsigned char *out, size_t *outlen,
                            const unsigned char *in, size_t inlen)
{
    int ret;
    EC_KEY *ec = ctx->pkey->pkey.ec;
    SM2_PKEY_CTX *dctx = ctx->data;
    const EVP_MD *md = (dctx->md == NULL) ? EVP_sm3() : dctx->md;
    OPENSSL_CTX *libctx = ec_key_get_libctx(ec);
    EVP_MD *fetched_md = NULL;

    if (out == NULL) {
        if (!sm2_ciphertext_size(ec, md, inlen, outlen))
            return -1;
        else
            return 1;
    }

    fetched_md = EVP_MD_fetch(libctx, EVP_MD_name(md), 0);
    if (fetched_md == NULL)
        return 0;
    ret = sm2_encrypt(ec, fetched_md, in, inlen, out, outlen);
    EVP_MD_free(fetched_md);
    return ret;
}

static int pkey_sm2_decrypt(EVP_PKEY_CTX *ctx,
                            unsigned char *out, size_t *outlen,
                            const unsigned char *in, size_t inlen)
{
    int ret;
    EC_KEY *ec = ctx->pkey->pkey.ec;
    SM2_PKEY_CTX *dctx = ctx->data;
    const EVP_MD *md = (dctx->md == NULL) ? EVP_sm3() : dctx->md;
    OPENSSL_CTX *libctx = ec_key_get_libctx(ec);
    EVP_MD *fetched_md = NULL;

    if (out == NULL) {
        if (!sm2_plaintext_size(ec, md, inlen, outlen))
            return -1;
        else
            return 1;
    }

    fetched_md = EVP_MD_fetch(libctx, EVP_MD_name(md), 0);
    if (fetched_md == NULL)
        return 0;
    ret = sm2_decrypt(ec, fetched_md, in, inlen, out, outlen);
    EVP_MD_free(fetched_md);
    return ret;
}

static int pkey_sm2_ctrl(EVP_PKEY_CTX *ctx, int type, int p1, void *p2)
{
    SM2_PKEY_CTX *smctx = ctx->data;
    uint8_t *tmp_id;

    switch (type) {
    case EVP_PKEY_CTRL_EC_PARAMGEN_CURVE_NID:
        /*
         * This control could be removed, which would signal it being
         * unsupported.  However, that means that when the caller uses
         * the correct curve, it may interpret the unsupported signal
         * as an error, so it's better to accept the control, check the
         * value and return a corresponding value.
         */
        if (p1 != NID_sm2) {
            SM2err(SM2_F_PKEY_SM2_CTRL, SM2_R_INVALID_CURVE);
            return 0;
        }
        return 1;

    case EVP_PKEY_CTRL_MD:
        smctx->md = p2;
        return 1;

    case EVP_PKEY_CTRL_GET_MD:
        *(const EVP_MD **)p2 = smctx->md;
        return 1;

    case EVP_PKEY_CTRL_SET1_ID:
        if (p1 > 0) {
            tmp_id = OPENSSL_malloc(p1);
            if (tmp_id == NULL) {
                SM2err(SM2_F_PKEY_SM2_CTRL, ERR_R_MALLOC_FAILURE);
                return 0;
            }
            memcpy(tmp_id, p2, p1);
            OPENSSL_free(smctx->id);
            smctx->id = tmp_id;
        } else {
            /* set null-ID */
            OPENSSL_free(smctx->id);
            smctx->id = NULL;
        }
        smctx->id_len = (size_t)p1;
        smctx->id_set = 1;
        return 1;

    case EVP_PKEY_CTRL_GET1_ID:
        memcpy(p2, smctx->id, smctx->id_len);
        return 1;

    case EVP_PKEY_CTRL_GET1_ID_LEN:
        *(size_t *)p2 = smctx->id_len;
        return 1;

    case EVP_PKEY_CTRL_DIGESTINIT:
        /* nothing to be inited, this is to suppress the error... */
        return 1;

    default:
        return -2;
    }
}

static int pkey_sm2_ctrl_str(EVP_PKEY_CTX *ctx,
                             const char *type, const char *value)
{
    uint8_t *hex_id;
    long hex_len = 0;
    int ret = 0;

    if (strcmp(type, "ec_paramgen_curve") == 0) {
        int nid = NID_undef;

        if (((nid = EC_curve_nist2nid(value)) == NID_undef)
            && ((nid = OBJ_sn2nid(value)) == NID_undef)
            && ((nid = OBJ_ln2nid(value)) == NID_undef)) {
            SM2err(SM2_F_PKEY_SM2_CTRL_STR, SM2_R_INVALID_CURVE);
            return 0;
        }
        return EVP_PKEY_CTX_set_ec_paramgen_curve_nid(ctx, nid);
    } else if (strcmp(type, "ec_param_enc") == 0) {
        int param_enc;

        if (strcmp(value, "explicit") == 0)
            param_enc = 0;
        else if (strcmp(value, "named_curve") == 0)
            param_enc = OPENSSL_EC_NAMED_CURVE;
        else
            return -2;
        return EVP_PKEY_CTX_set_ec_param_enc(ctx, param_enc);
    } else if (strcmp(type, "distid") == 0) {
        return pkey_sm2_ctrl(ctx, EVP_PKEY_CTRL_SET1_ID,
                             (int)strlen(value), (void *)value);
    } else if (strcmp(type, "hexdistid") == 0) {
        hex_id = OPENSSL_hexstr2buf((const char *)value, &hex_len);
        if (hex_id == NULL) {
            SM2err(SM2_F_PKEY_SM2_CTRL_STR, ERR_R_PASSED_INVALID_ARGUMENT);
            return 0;
        }
        ret = pkey_sm2_ctrl(ctx, EVP_PKEY_CTRL_SET1_ID, (int)hex_len,
                            (void *)hex_id);
        OPENSSL_free(hex_id);
        return ret;
    }

    return -2;
}

static int pkey_sm2_digest_custom(EVP_PKEY_CTX *ctx, EVP_MD_CTX *mctx)
{
    uint8_t z[EVP_MAX_MD_SIZE];
    SM2_PKEY_CTX *smctx = ctx->data;
    EC_KEY *ec = ctx->pkey->pkey.ec;
    const EVP_MD *md = EVP_MD_CTX_md(mctx);
    int mdlen = EVP_MD_size(md);

    if (!smctx->id_set) {
        /*
         * An ID value must be set. The specifications are not clear whether a
         * NULL is allowed. We only allow it if set explicitly for maximum
         * flexibility.
         */
        SM2err(SM2_F_PKEY_SM2_DIGEST_CUSTOM, SM2_R_ID_NOT_SET);
        return 0;
    }

    if (mdlen < 0) {
        SM2err(SM2_F_PKEY_SM2_DIGEST_CUSTOM, SM2_R_INVALID_DIGEST);
        return 0;
    }

    /* get hashed prefix 'z' of tbs message */
    if (!sm2_compute_z_digest(z, md, smctx->id, smctx->id_len, ec))
        return 0;

    return EVP_DigestUpdate(mctx, z, (size_t)mdlen);
}

static int pkey_sm2_paramgen(EVP_PKEY_CTX *ctx, EVP_PKEY *pkey)
{
    EC_KEY *ec = NULL;
    int ret;

    ec = EC_KEY_new_by_curve_name(NID_sm2);
    if (ec == NULL)
        return 0;
    if (!ossl_assert(ret = EVP_PKEY_assign_EC_KEY(pkey, ec)))
        EC_KEY_free(ec);
    return ret;
}

static int pkey_sm2_keygen(EVP_PKEY_CTX *ctx, EVP_PKEY *pkey)
{
    EC_KEY *ec = NULL;

    ec = EC_KEY_new_by_curve_name(NID_sm2);
    if (ec == NULL)
        return 0;
    if (!ossl_assert(EVP_PKEY_assign_EC_KEY(pkey, ec))) {
        EC_KEY_free(ec);
        return 0;
    }
    /* Note: if error is returned, we count on caller to free pkey->pkey.ec */
    if (ctx->pkey != NULL
        && !EVP_PKEY_copy_parameters(pkey, ctx->pkey))
        return 0;

    return EC_KEY_generate_key(ec);
}

static const EVP_PKEY_METHOD sm2_pkey_meth = {
    EVP_PKEY_SM2,
    0,
    pkey_sm2_init,
    pkey_sm2_copy,
    pkey_sm2_cleanup,

    0,
    pkey_sm2_paramgen,

    0,
    pkey_sm2_keygen,

    0,
    pkey_sm2_sign,

    0,
    pkey_sm2_verify,

    0, 0,

    0, 0, 0, 0,

    0,
    pkey_sm2_encrypt,

    0,
    pkey_sm2_decrypt,

    0,
    0,
    pkey_sm2_ctrl,
    pkey_sm2_ctrl_str,

    0, 0,

    0, 0, 0,

    pkey_sm2_digest_custom
};

const EVP_PKEY_METHOD *sm2_pkey_method(void)
{
    return &sm2_pkey_meth;
}
