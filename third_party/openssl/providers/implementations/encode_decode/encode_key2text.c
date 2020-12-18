/*
 * Copyright 2020 The OpenSSL Project Authors. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://www.openssl.org/source/license.html
 */

/*
 * Low level APIs are deprecated for public use, but still ok for internal use.
 */
#include "internal/deprecated.h"

#include <ctype.h>

#include <openssl/core.h>
#include <openssl/core_dispatch.h>
#include <openssl/core_names.h>
#include <openssl/bn.h>
#include <openssl/err.h>
#include <openssl/safestack.h>
#include "internal/ffc.h"
#include "crypto/bn.h"           /* bn_get_words() */
#include "crypto/dh.h"           /* dh_get0_params() */
#include "crypto/dsa.h"          /* dsa_get0_params() */
#include "crypto/ec.h"           /* ec_key_get_libctx */
#include "crypto/ecx.h"          /* ECX_KEY, etc... */
#include "crypto/rsa.h"          /* RSA_PSS_PARAMS_30, etc... */
#include "prov/bio.h"
#include "prov/implementations.h"
#include "prov/providercommonerr.h"
#include "endecoder_local.h"

DEFINE_SPECIAL_STACK_OF_CONST(BIGNUM_const, BIGNUM)

# ifdef SIXTY_FOUR_BIT_LONG
#  define BN_FMTu "%lu"
#  define BN_FMTx "%lx"
# endif

# ifdef SIXTY_FOUR_BIT
#  define BN_FMTu "%llu"
#  define BN_FMTx "%llx"
# endif

# ifdef THIRTY_TWO_BIT
#  define BN_FMTu "%u"
#  define BN_FMTx "%x"
# endif

static int print_labeled_bignum(BIO *out, const char *label, const BIGNUM *bn)
{
    int ret = 0, use_sep = 0;
    char *hex_str = NULL, *p;
    const char spaces[] = "    ";
    const char *post_label_spc = " ";

    const char *neg = "";
    int bytes;

    if (bn == NULL)
        return 0;
    if (label == NULL) {
        label = "";
        post_label_spc = "";
    }

    if (BN_is_zero(bn))
        return BIO_printf(out, "%s%s0\n", label, post_label_spc);

    if (BN_num_bytes(bn) <= BN_BYTES) {
        BN_ULONG *words = bn_get_words(bn);

        if (BN_is_negative(bn))
            neg = "-";

        return BIO_printf(out, "%s%s%s" BN_FMTu " (%s0x" BN_FMTx ")\n",
                          label, post_label_spc, neg, words[0], neg, words[0]);
    }

    hex_str = BN_bn2hex(bn);
    p = hex_str;
    if (*p == '-') {
        ++p;
        neg = " (Negative)";
    }
    if (BIO_printf(out, "%s%s\n", label, neg) <= 0)
        goto err;

    /* Keep track of how many bytes we have printed out so far */
    bytes = 0;

    if (BIO_printf(out, "%s", spaces) <= 0)
        goto err;

    /* Add a leading 00 if the top bit is set */
    if (*p >= '8') {
        if (BIO_printf(out, "%02x", 0) <= 0)
            goto err;
        ++bytes;
        use_sep = 1;
    }
    while (*p != '\0') {
        /* Do a newline after every 15 hex bytes + add the space indent */
        if ((bytes % 15) == 0 && bytes > 0) {
            if (BIO_printf(out, ":\n%s", spaces) <= 0)
                goto err;
            use_sep = 0; /* The first byte on the next line doesnt have a : */
        }
        if (BIO_printf(out, "%s%c%c", use_sep ? ":" : "",
                       tolower(p[0]), tolower(p[1])) <= 0)
            goto err;
        ++bytes;
        p += 2;
        use_sep = 1;
    }
    if (BIO_printf(out, "\n") <= 0)
        goto err;
    ret = 1;
err:
    OPENSSL_free(hex_str);
    return ret;
}

/* Number of octets per line */
#define LABELED_BUF_PRINT_WIDTH    15

static int print_labeled_buf(BIO *out, const char *label,
                             const unsigned char *buf, size_t buflen)
{
    size_t i;

    if (BIO_printf(out, "%s\n", label) <= 0)
        return 0;

    for (i = 0; i < buflen; i++) {
        if ((i % LABELED_BUF_PRINT_WIDTH) == 0) {
            if (i > 0 && BIO_printf(out, "\n") <= 0)
                return 0;
            if (BIO_printf(out, "    ") <= 0)
                return 0;
        }

        if (BIO_printf(out, "%02x%s", buf[i],
                                 (i == buflen - 1) ? "" : ":") <= 0)
            return 0;
    }
    if (BIO_printf(out, "\n") <= 0)
        return 0;

    return 1;
}

#if !defined(OPENSSL_NO_DH) || !defined(OPENSSL_NO_DSA)
static int ffc_params_to_text(BIO *out, const FFC_PARAMS *ffc)
{
    if (ffc->nid != NID_undef) {
#ifndef OPENSSL_NO_DH
        const char *name = ffc_named_group_from_uid(ffc->nid);

        if (name == NULL)
            goto err;
        if (BIO_printf(out, "GROUP: %s\n", name) <= 0)
            goto err;
        return 1;
#else
        /* How could this be? We should not have a nid in a no-dh build. */
        goto err;
#endif
    }

    if (!print_labeled_bignum(out, "P:   ", ffc->p))
        goto err;
    if (ffc->q != NULL) {
        if (!print_labeled_bignum(out, "Q:   ", ffc->q))
            goto err;
    }
    if (!print_labeled_bignum(out, "G:   ", ffc->g))
        goto err;
    if (ffc->j != NULL) {
        if (!print_labeled_bignum(out, "J:   ", ffc->j))
            goto err;
    }
    if (ffc->seed != NULL) {
        if (!print_labeled_buf(out, "SEED:", ffc->seed, ffc->seedlen))
            goto err;
    }
    if (ffc->gindex != -1) {
        if (BIO_printf(out, "gindex: %d\n", ffc->gindex) <= 0)
            goto err;
    }
    if (ffc->pcounter != -1) {
        if (BIO_printf(out, "pcounter: %d\n", ffc->pcounter) <= 0)
            goto err;
    }
    if (ffc->h != 0) {
        if (BIO_printf(out, "h: %d\n", ffc->h) <= 0)
            goto err;
    }
    return 1;
err:
    return 0;
}
#endif

/* ---------------------------------------------------------------------- */

#ifndef OPENSSL_NO_DH
# define dh_param_selection     OSSL_KEYMGMT_SELECT_DOMAIN_PARAMETERS
# define dh_pub_selection       (OSSL_KEYMGMT_SELECT_PUBLIC_KEY \
                                 | dh_param_selection)
# define dh_priv_selection      (OSSL_KEYMGMT_SELECT_KEYPAIR \
                                 | dh_param_selection)

static int dh_to_text(BIO *out, const void *key, int selection)
{
    const DH *dh = key;
    const char *type_label = NULL;
    const BIGNUM *priv_key = NULL, *pub_key = NULL;
    const FFC_PARAMS *params = NULL;
    const BIGNUM *p = NULL;

    if (out == NULL || dh == NULL) {
        ERR_raise(ERR_LIB_PROV, ERR_R_PASSED_NULL_PARAMETER);
        return 0;
    }

    if ((selection & OSSL_KEYMGMT_SELECT_PRIVATE_KEY) != 0)
        type_label = "DH Private-Key";
    else if ((selection & OSSL_KEYMGMT_SELECT_PUBLIC_KEY) != 0)
        type_label = "DH Public-Key";
    else if ((selection & OSSL_KEYMGMT_SELECT_DOMAIN_PARAMETERS) != 0)
        type_label = "DH Parameters";

    if ((selection & OSSL_KEYMGMT_SELECT_PRIVATE_KEY) != 0) {
        priv_key = DH_get0_priv_key(dh);
        if (priv_key == NULL) {
            ERR_raise(ERR_LIB_PROV, PROV_R_NOT_A_PRIVATE_KEY);
            return 0;
        }
    }
    if ((selection & OSSL_KEYMGMT_SELECT_PUBLIC_KEY) != 0) {
        pub_key = DH_get0_pub_key(dh);
        if (pub_key == NULL) {
            ERR_raise(ERR_LIB_PROV, PROV_R_NOT_A_PUBLIC_KEY);
            return 0;
        }
    }
    if ((selection & OSSL_KEYMGMT_SELECT_DOMAIN_PARAMETERS) != 0) {
        params = dh_get0_params((DH *)dh);
        if (params == NULL) {
            ERR_raise(ERR_LIB_PROV, PROV_R_NOT_PARAMETERS);
            return 0;
        }
    }

    p = DH_get0_p(dh);
    if (p == NULL) {
        ERR_raise(ERR_LIB_PROV, PROV_R_INVALID_KEY);
        return 0;
    }

    if (BIO_printf(out, "%s: (%d bit)\n", type_label, BN_num_bits(p)) <= 0)
        return 0;
    if (priv_key != NULL
        && !print_labeled_bignum(out, "private-key:", priv_key))
        return 0;
    if (pub_key != NULL
        && !print_labeled_bignum(out, "public-key:", pub_key))
        return 0;
    if (params != NULL
        && !ffc_params_to_text(out, params))
        return 0;

    return 1;
}
#endif

/* ---------------------------------------------------------------------- */

#ifndef OPENSSL_NO_DSA
# define dsa_param_selection    OSSL_KEYMGMT_SELECT_DOMAIN_PARAMETERS
# define dsa_pub_selection      (OSSL_KEYMGMT_SELECT_PUBLIC_KEY \
                                 | dsa_param_selection)
# define dsa_priv_selection     (OSSL_KEYMGMT_SELECT_KEYPAIR \
                                 | dsa_param_selection)

static int dsa_to_text(BIO *out, const void *key, int selection)
{
    const DSA *dsa = key;
    const char *type_label = NULL;
    const BIGNUM *priv_key = NULL, *pub_key = NULL;
    const FFC_PARAMS *params = NULL;
    const BIGNUM *p = NULL;

    if (out == NULL || dsa == NULL) {
        ERR_raise(ERR_LIB_PROV, ERR_R_PASSED_NULL_PARAMETER);
        return 0;
    }

    if ((selection & OSSL_KEYMGMT_SELECT_PRIVATE_KEY) != 0)
        type_label = "Private-Key";
    else if ((selection & OSSL_KEYMGMT_SELECT_PUBLIC_KEY) != 0)
        type_label = "Public-Key";
    else if ((selection & OSSL_KEYMGMT_SELECT_DOMAIN_PARAMETERS) != 0)
        type_label = "DSA-Parameters";

    if ((selection & OSSL_KEYMGMT_SELECT_PRIVATE_KEY) != 0) {
        priv_key = DSA_get0_priv_key(dsa);
        if (priv_key == NULL) {
            ERR_raise(ERR_LIB_PROV, PROV_R_NOT_A_PRIVATE_KEY);
            return 0;
        }
    }
    if ((selection & OSSL_KEYMGMT_SELECT_PUBLIC_KEY) != 0) {
        pub_key = DSA_get0_pub_key(dsa);
        if (pub_key == NULL) {
            ERR_raise(ERR_LIB_PROV, PROV_R_NOT_A_PUBLIC_KEY);
            return 0;
        }
    }
    if ((selection & OSSL_KEYMGMT_SELECT_DOMAIN_PARAMETERS) != 0) {
        params = dsa_get0_params((DSA *)dsa);
        if (params == NULL) {
            ERR_raise(ERR_LIB_PROV, PROV_R_NOT_PARAMETERS);
            return 0;
        }
    }

    p = DSA_get0_p(dsa);
    if (p == NULL) {
        ERR_raise(ERR_LIB_PROV, PROV_R_INVALID_KEY);
        return 0;
    }

    if (BIO_printf(out, "%s: (%d bit)\n", type_label, BN_num_bits(p)) <= 0)
        return 0;
    if (priv_key != NULL
        && !print_labeled_bignum(out, "priv:", priv_key))
        return 0;
    if (pub_key != NULL
        && !print_labeled_bignum(out, "pub: ", pub_key))
        return 0;
    if (params != NULL
        && !ffc_params_to_text(out, params))
        return 0;

    return 1;
}
#endif

/* ---------------------------------------------------------------------- */

#ifndef OPENSSL_NO_EC
# define ec_param_selection     OSSL_KEYMGMT_SELECT_ALL_PARAMETERS
# define ec_pub_selection       (OSSL_KEYMGMT_SELECT_PUBLIC_KEY \
                                 | ec_param_selection)
# define ec_priv_selection      (OSSL_KEYMGMT_SELECT_KEYPAIR \
                                 | ec_param_selection)

static int ec_param_explicit_curve_to_text(BIO *out, const EC_GROUP *group,
                                           BN_CTX *ctx)
{
    const char *plabel = "Prime:";
    BIGNUM *p = NULL, *a = NULL, *b = NULL;

    p = BN_CTX_get(ctx);
    a = BN_CTX_get(ctx);
    b = BN_CTX_get(ctx);
    if (b == NULL
        || !EC_GROUP_get_curve(group, p, a, b, ctx))
        return 0;

    if (EC_GROUP_get_field_type(group) == NID_X9_62_characteristic_two_field) {
        int basis_type = EC_GROUP_get_basis_type(group);

        /* print the 'short name' of the base type OID */
        if (basis_type == NID_undef
            || BIO_printf(out, "Basis Type: %s\n", OBJ_nid2sn(basis_type)) <= 0)
            return 0;
        plabel = "Polynomial:";
    }
    return print_labeled_bignum(out, plabel, p)
        && print_labeled_bignum(out, "A:   ", a)
        && print_labeled_bignum(out, "B:   ", b);
}

static int ec_param_explicit_gen_to_text(BIO *out, const EC_GROUP *group,
                                         BN_CTX *ctx)
{
    const EC_POINT *point = NULL;
    BIGNUM *gen = NULL;
    const char *glabel = NULL;
    point_conversion_form_t form;

    form = EC_GROUP_get_point_conversion_form(group);
    point = EC_GROUP_get0_generator(group);
    gen = BN_CTX_get(ctx);

    if (gen == NULL
        || point == NULL
        || EC_POINT_point2bn(group, point, form, gen, ctx) == NULL)
        return 0;

    if (gen != NULL) {
        switch (form) {
        case POINT_CONVERSION_COMPRESSED:
           glabel = "Generator (compressed):";
           break;
        case POINT_CONVERSION_UNCOMPRESSED:
            glabel = "Generator (uncompressed):";
            break;
        case POINT_CONVERSION_HYBRID:
            glabel = "Generator (hybrid):";
            break;
        default:
            return 0;
        }
        return print_labeled_bignum(out, glabel, gen);
    }
    return 1;
}

/* Print explicit parameters */
static int ec_param_explicit_to_text(BIO *out, const EC_GROUP *group,
                                     OPENSSL_CTX *libctx)
{
    int ret = 0, tmp_nid;
    BN_CTX *ctx = NULL;
    const BIGNUM *order = NULL, *cofactor = NULL;
    const unsigned char *seed;
    size_t seed_len = 0;

    ctx = BN_CTX_new_ex(libctx);
    if (ctx == NULL)
        return 0;
    BN_CTX_start(ctx);

    tmp_nid = EC_GROUP_get_field_type(group);
    order = EC_GROUP_get0_order(group);
    if (order == NULL)
        goto err;

    seed = EC_GROUP_get0_seed(group);
    if (seed != NULL)
        seed_len = EC_GROUP_get_seed_len(group);
    cofactor = EC_GROUP_get0_cofactor(group);

    /* print the 'short name' of the field type */
    if (BIO_printf(out, "Field Type: %s\n", OBJ_nid2sn(tmp_nid)) <= 0
        || !ec_param_explicit_curve_to_text(out, group, ctx)
        || !ec_param_explicit_gen_to_text(out, group, ctx)
        || !print_labeled_bignum(out, "Order: ", order)
        || (cofactor != NULL
            && !print_labeled_bignum(out, "Cofactor: ", cofactor))
        || (seed != NULL
            && !print_labeled_buf(out, "Seed:", seed, seed_len)))
        goto err;
    ret = 1;
err:
    BN_CTX_end(ctx);
    BN_CTX_free(ctx);
    return ret;
}

static int ec_param_to_text(BIO *out, const EC_GROUP *group,
                            OPENSSL_CTX *libctx)
{
    if (EC_GROUP_get_asn1_flag(group) & OPENSSL_EC_NAMED_CURVE) {
        const char *curve_name;
        int curve_nid = EC_GROUP_get_curve_name(group);

        /* Explicit parameters */
        if (curve_nid == NID_undef)
            return 0;

        if (BIO_printf(out, "%s: %s\n", "ASN1 OID", OBJ_nid2sn(curve_nid)) <= 0)
            return 0;

        curve_name = EC_curve_nid2nist(curve_nid);
        return (curve_name == NULL
                || BIO_printf(out, "%s: %s\n", "NIST CURVE", curve_name) > 0);
    } else {
        return ec_param_explicit_to_text(out, group, libctx);
    }
}

static int ec_to_text(BIO *out, const void *key, int selection)
{
    const EC_KEY *ec = key;
    const char *type_label = NULL;
    unsigned char *priv = NULL, *pub = NULL;
    size_t priv_len = 0, pub_len = 0;
    const EC_GROUP *group;
    int ret = 0;

    if (out == NULL || ec == NULL) {
        ERR_raise(ERR_LIB_PROV, ERR_R_PASSED_NULL_PARAMETER);
        return 0;
    }

    if ((group = EC_KEY_get0_group(ec)) == NULL) {
        ERR_raise(ERR_LIB_PROV, PROV_R_INVALID_KEY);
        return 0;
    }

    if ((selection & OSSL_KEYMGMT_SELECT_PRIVATE_KEY) != 0)
        type_label = "Private-Key";
    else if ((selection & OSSL_KEYMGMT_SELECT_PUBLIC_KEY) != 0)
        type_label = "Public-Key";
    else if ((selection & OSSL_KEYMGMT_SELECT_DOMAIN_PARAMETERS) != 0)
        type_label = "EC-Parameters";

    if ((selection & OSSL_KEYMGMT_SELECT_PRIVATE_KEY) != 0) {
        const BIGNUM *priv_key = EC_KEY_get0_private_key(ec);

        if (priv_key == NULL) {
            ERR_raise(ERR_LIB_PROV, PROV_R_NOT_A_PRIVATE_KEY);
            goto err;
        }
        priv_len = EC_KEY_priv2buf(ec, &priv);
        if (priv_len == 0)
            goto err;
    }
    if ((selection & OSSL_KEYMGMT_SELECT_PUBLIC_KEY) != 0) {
        const EC_POINT *pub_pt = EC_KEY_get0_public_key(ec);

        if (pub_pt == NULL) {
            ERR_raise(ERR_LIB_PROV, PROV_R_NOT_A_PUBLIC_KEY);
            goto err;
        }

        pub_len = EC_KEY_key2buf(ec, EC_KEY_get_conv_form(ec), &pub, NULL);
        if (pub_len == 0)
            goto err;
    }

    if (BIO_printf(out, "%s: (%d bit)\n", type_label,
                   EC_GROUP_order_bits(group)) <= 0)
        goto err;
    if (priv != NULL
        && !print_labeled_buf(out, "priv:", priv, priv_len))
        goto err;
    if (pub != NULL
        && !print_labeled_buf(out, "pub:", pub, pub_len))
        goto err;
    if ((selection & OSSL_KEYMGMT_SELECT_DOMAIN_PARAMETERS) != 0)
        ret = ec_param_to_text(out, group, ec_key_get_libctx(ec));
err:
    OPENSSL_clear_free(priv, priv_len);
    OPENSSL_free(pub);
    return ret;
}
#endif

/* ---------------------------------------------------------------------- */

#ifndef OPENSSL_NO_EC
# define ecx_pub_selection      OSSL_KEYMGMT_SELECT_PUBLIC_KEY
# define ecx_priv_selection     OSSL_KEYMGMT_SELECT_KEYPAIR

static int ecx_to_text(BIO *out, const void *key, int selection)
{
    const ECX_KEY *ecx = key;
    const char *type_label = NULL;

    if (out == NULL || ecx == NULL) {
        ERR_raise(ERR_LIB_PROV, ERR_R_PASSED_NULL_PARAMETER);
        return 0;
    }

    if ((selection & OSSL_KEYMGMT_SELECT_PRIVATE_KEY) != 0) {
        if (ecx->privkey == NULL) {
            ERR_raise(ERR_LIB_PROV, PROV_R_NOT_A_PRIVATE_KEY);
            return 0;
        }

        switch (ecx->type) {
        case ECX_KEY_TYPE_X25519:
            type_label = "X25519 Private-Key";
            break;
        case ECX_KEY_TYPE_X448:
            type_label = "X448 Private-Key";
            break;
        case ECX_KEY_TYPE_ED25519:
            type_label = "ED25519 Private-Key";
            break;
        case ECX_KEY_TYPE_ED448:
            type_label = "ED448 Private-Key";
            break;
        }
    } else if ((selection & OSSL_KEYMGMT_SELECT_PUBLIC_KEY) != 0) {
        /* ecx->pubkey is an array, not a pointer... */
        if (!ecx->haspubkey) {
            ERR_raise(ERR_LIB_PROV, PROV_R_NOT_A_PUBLIC_KEY);
            return 0;
        }

        switch (ecx->type) {
        case ECX_KEY_TYPE_X25519:
            type_label = "X25519 Public-Key";
            break;
        case ECX_KEY_TYPE_X448:
            type_label = "X448 Public-Key";
            break;
        case ECX_KEY_TYPE_ED25519:
            type_label = "ED25519 Public-Key";
            break;
        case ECX_KEY_TYPE_ED448:
            type_label = "ED448 Public-Key";
            break;
        }
    }

    if (BIO_printf(out, "%s:\n", type_label) <= 0)
        return 0;
    if ((selection & OSSL_KEYMGMT_SELECT_PRIVATE_KEY) != 0
        && !print_labeled_buf(out, "priv:", ecx->privkey, ecx->keylen))
        return 0;
    if ((selection & OSSL_KEYMGMT_SELECT_PUBLIC_KEY) != 0
        && !print_labeled_buf(out, "pub:", ecx->pubkey, ecx->keylen))
        return 0;

    return 1;
}
#endif

/* ---------------------------------------------------------------------- */

#define rsa_param_selection     OSSL_KEYMGMT_SELECT_OTHER_PARAMETERS
#define rsa_pub_selection       (OSSL_KEYMGMT_SELECT_PUBLIC_KEY \
                                 | rsa_param_selection)
#define rsa_priv_selection      (OSSL_KEYMGMT_SELECT_KEYPAIR \
                                 | rsa_param_selection)

static int rsa_to_text(BIO *out, const void *key, int selection)
{
    const RSA *rsa = key;
    const char *type_label = "RSA key";
    const char *modulus_label;
    const char *exponent_label;
    const BIGNUM *rsa_d = NULL, *rsa_n = NULL, *rsa_e = NULL;
    STACK_OF(BIGNUM_const) *factors = NULL;
    STACK_OF(BIGNUM_const) *exps = NULL;
    STACK_OF(BIGNUM_const) *coeffs = NULL;
    int primes;
    const RSA_PSS_PARAMS_30 *pss_params = rsa_get0_pss_params_30((RSA *)rsa);
    int ret = 0;

    if (out == NULL || rsa == NULL) {
        ERR_raise(ERR_LIB_PROV, ERR_R_PASSED_NULL_PARAMETER);
        goto err;
    }

    factors = sk_BIGNUM_const_new_null();
    exps = sk_BIGNUM_const_new_null();
    coeffs = sk_BIGNUM_const_new_null();

    if (factors == NULL || exps == NULL || coeffs == NULL) {
        ERR_raise(ERR_LIB_PROV, ERR_R_MALLOC_FAILURE);
        goto err;
    }

    if ((selection & OSSL_KEYMGMT_SELECT_PRIVATE_KEY) != 0) {
        type_label = "Private-Key";
        modulus_label = "modulus:";
        exponent_label = "publicExponent:";
    } else if ((selection & OSSL_KEYMGMT_SELECT_PUBLIC_KEY) != 0) {
        type_label = "Public-Key";
        modulus_label = "Modulus:";
        exponent_label = "Exponent:";
    }

    RSA_get0_key(rsa, &rsa_n, &rsa_e, &rsa_d);
    rsa_get0_all_params((RSA *)rsa, factors, exps, coeffs);
    primes = sk_BIGNUM_const_num(factors);

    if ((selection & OSSL_KEYMGMT_SELECT_PRIVATE_KEY) != 0) {
        if (BIO_printf(out, "%s: (%d bit, %d primes)\n",
                       type_label, BN_num_bits(rsa_n), primes) <= 0)
            goto err;
    } else {
        if (BIO_printf(out, "%s: (%d bit)\n",
                       type_label, BN_num_bits(rsa_n)) <= 0)
            goto err;
    }

    if (!print_labeled_bignum(out, modulus_label, rsa_n))
        goto err;
    if (!print_labeled_bignum(out, exponent_label, rsa_e))
        goto err;
    if ((selection & OSSL_KEYMGMT_SELECT_PRIVATE_KEY) != 0) {
        int i;

        if (!print_labeled_bignum(out, "privateExponent:", rsa_d))
            goto err;
        if (!print_labeled_bignum(out, "prime1:",
                                  sk_BIGNUM_const_value(factors, 0)))
            goto err;
        if (!print_labeled_bignum(out, "prime2:",
                                  sk_BIGNUM_const_value(factors, 1)))
            goto err;
        if (!print_labeled_bignum(out, "exponent1:",
                                  sk_BIGNUM_const_value(exps, 0)))
            goto err;
        if (!print_labeled_bignum(out, "exponent2:",
                                  sk_BIGNUM_const_value(exps, 1)))
            goto err;
        if (!print_labeled_bignum(out, "coefficient:",
                                  sk_BIGNUM_const_value(coeffs, 0)))
            goto err;
        for (i = 2; i < sk_BIGNUM_const_num(factors); i++) {
            if (BIO_printf(out, "prime%d:", i + 1) <= 0)
                goto err;
            if (!print_labeled_bignum(out, NULL,
                                      sk_BIGNUM_const_value(factors, i)))
                goto err;
            if (BIO_printf(out, "exponent%d:", i + 1) <= 0)
                goto err;
            if (!print_labeled_bignum(out, NULL,
                                      sk_BIGNUM_const_value(exps, i)))
                goto err;
            if (BIO_printf(out, "coefficient%d:", i + 1) <= 0)
                goto err;
            if (!print_labeled_bignum(out, NULL,
                                      sk_BIGNUM_const_value(coeffs, i - 1)))
                goto err;
        }
    }

    if ((selection & OSSL_KEYMGMT_SELECT_OTHER_PARAMETERS) != 0) {
        switch (RSA_test_flags(rsa, RSA_FLAG_TYPE_MASK)) {
        case RSA_FLAG_TYPE_RSA:
            if (!rsa_pss_params_30_is_unrestricted(pss_params)) {
                if (BIO_printf(out, "(INVALID PSS PARAMETERS)\n") <= 0)
                    goto err;
            }
            break;
        case RSA_FLAG_TYPE_RSASSAPSS:
            if (rsa_pss_params_30_is_unrestricted(pss_params)) {
                if (BIO_printf(out, "No PSS parameter restrictions\n") <= 0)
                    goto err;
            } else {
                int hashalg_nid = rsa_pss_params_30_hashalg(pss_params);
                int maskgenalg_nid = rsa_pss_params_30_maskgenalg(pss_params);
                int maskgenhashalg_nid =
                    rsa_pss_params_30_maskgenhashalg(pss_params);
                int saltlen = rsa_pss_params_30_saltlen(pss_params);
                int trailerfield = rsa_pss_params_30_trailerfield(pss_params);

                if (BIO_printf(out, "PSS parameter restrictions:\n") <= 0)
                    goto err;
                if (BIO_printf(out, "  Hash Algorithm: %s%s\n",
                               rsa_oaeppss_nid2name(hashalg_nid),
                               (hashalg_nid == NID_sha1
                                ? " (default)" : "")) <= 0)
                    goto err;
                if (BIO_printf(out, "  Mask Algorithm: %s with %s%s\n",
                               rsa_mgf_nid2name(maskgenalg_nid),
                               rsa_oaeppss_nid2name(maskgenhashalg_nid),
                               (maskgenalg_nid == NID_mgf1
                                && maskgenhashalg_nid == NID_sha1
                                ? " (default)" : "")) <= 0)
                    goto err;
                if (BIO_printf(out, "  Minimum Salt Length: %d%s\n",
                               saltlen,
                               (saltlen == 20 ? " (default)" : "")) <= 0)
                    goto err;
                /*
                 * TODO(3.0) Should we show the ASN.1 trailerField value, or
                 * the actual trailerfield byte (i.e. 0xBC for 1)?
                 * crypto/rsa/rsa_ameth.c isn't very clear on that, as it
                 * does display 0xBC when the default applies, but the ASN.1
                 * trailerField value otherwise...
                 */
                if (BIO_printf(out, "  Trailer Field: 0x%x%s\n",
                               trailerfield,
                               (trailerfield == 1 ? " (default)" : "")) <= 0)
                    goto err;
            }
            break;
        }
    }

    ret = 1;
 err:
    sk_BIGNUM_const_free(factors);
    sk_BIGNUM_const_free(exps);
    sk_BIGNUM_const_free(coeffs);
    return ret;
}

/* ---------------------------------------------------------------------- */

static void *key2text_newctx(void *provctx)
{
    return provctx;
}

static void key2text_freectx(ossl_unused void *vctx)
{
}

static int key2text_encode(void *vctx, const void *key, int selection,
                           OSSL_CORE_BIO *cout,
                           int (*key2text)(BIO *out, const void *key,
                                           int selection),
                           OSSL_PASSPHRASE_CALLBACK *cb, void *cbarg)
{
    BIO *out = bio_new_from_core_bio(vctx, cout);
    int ret;

    if (out == NULL)
        return 0;

    ret = key2text(out, key, selection);
    BIO_free(out);

    return ret;
}

#define MAKE_TEXT_ENCODER_KIND(impl, kind, type)                        \
    static OSSL_FUNC_encoder_encode_data_fn                             \
    impl##_##kind##2text_encode_d;                                      \
    static OSSL_FUNC_encoder_encode_object_fn                           \
    impl##_##kind##2text_encode_o;                                      \
    static int                                                          \
    impl##_##kind##2text_encode_d(void *ctx, const OSSL_PARAM params[], \
                                  OSSL_CORE_BIO *cout,                  \
                                  OSSL_PASSPHRASE_CALLBACK *cb,         \
                                  void *cbarg)                          \
    {                                                                   \
        int selection = type##_##kind##_selection;                      \
        void *key = ossl_prov_import_key(impl##_keymgmt_functions,      \
                                         ctx, selection, params);       \
        int ret;                                                        \
                                                                        \
        if (key == NULL)                                                \
            return 0;                                                   \
                                                                        \
        ret = impl##_##kind##2text_encode_o(ctx, key, cout, cb, cbarg); \
        ossl_prov_free_key(impl##_keymgmt_functions, key);              \
        return ret;                                                     \
    }                                                                   \
    static int                                                          \
    impl##_##kind##2text_encode_o(void *vctx, const void *key,          \
                                  OSSL_CORE_BIO *cout,                  \
                                  OSSL_PASSPHRASE_CALLBACK *cb,         \
                                  void *cbarg)                          \
    {                                                                   \
        int selection = type##_##kind##_selection;                      \
                                                                        \
        return key2text_encode(vctx, key, selection, cout,              \
                               type##_to_text, cb, cbarg);              \
    }                                                                   \
    const OSSL_DISPATCH impl##_##kind##_to_text_encoder_functions[] = { \
        { OSSL_FUNC_ENCODER_NEWCTX,                                     \
          (void (*)(void))key2text_newctx },                            \
        { OSSL_FUNC_ENCODER_FREECTX,                                    \
          (void (*)(void))key2text_freectx },                           \
        { OSSL_FUNC_ENCODER_ENCODE_DATA,                                \
          (void (*)(void))impl##_##kind##2text_encode_d },              \
        { OSSL_FUNC_ENCODER_ENCODE_OBJECT,                              \
          (void (*)(void))impl##_##kind##2text_encode_o },              \
        { 0, NULL }                                                     \
    }

#define MAKE_TEXT_ENCODER(impl, type)                                   \
    MAKE_TEXT_ENCODER_KIND(impl, param, type);                          \
    MAKE_TEXT_ENCODER_KIND(impl, pub, type);                            \
    MAKE_TEXT_ENCODER_KIND(impl, priv, type)

#define MAKE_TEXT_ENCODER_NOPARAM(impl, type)                           \
    MAKE_TEXT_ENCODER_KIND(impl, pub, type);                            \
    MAKE_TEXT_ENCODER_KIND(impl, priv, type)

#ifndef OPENSSL_NO_DH
MAKE_TEXT_ENCODER(dh, dh);
#endif
#ifndef OPENSSL_NO_DSA
MAKE_TEXT_ENCODER(dsa, dsa);
#endif
#ifndef OPENSSL_NO_EC
MAKE_TEXT_ENCODER(ec, ec);
MAKE_TEXT_ENCODER_NOPARAM(ed25519, ecx);
MAKE_TEXT_ENCODER_NOPARAM(ed448, ecx);
MAKE_TEXT_ENCODER_NOPARAM(x25519, ecx);
MAKE_TEXT_ENCODER_NOPARAM(x448, ecx);
#endif
MAKE_TEXT_ENCODER_NOPARAM(rsa, rsa);
