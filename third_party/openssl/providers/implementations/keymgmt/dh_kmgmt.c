/*
 * Copyright 2019-2020 The OpenSSL Project Authors. All Rights Reserved.
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

#include <string.h> /* strcmp */
#include <openssl/core_dispatch.h>
#include <openssl/core_names.h>
#include <openssl/bn.h>
#include <openssl/err.h>
#include "prov/implementations.h"
#include "prov/providercommon.h"
#include "prov/provider_ctx.h"
#include "crypto/dh.h"
#include "internal/sizes.h"
#include "internal/nelem.h"
#include "internal/param_build_set.h"

static OSSL_FUNC_keymgmt_new_fn dh_newdata;
static OSSL_FUNC_keymgmt_free_fn dh_freedata;
static OSSL_FUNC_keymgmt_gen_init_fn dh_gen_init;
static OSSL_FUNC_keymgmt_gen_init_fn dhx_gen_init;
static OSSL_FUNC_keymgmt_gen_set_template_fn dh_gen_set_template;
static OSSL_FUNC_keymgmt_gen_set_params_fn dh_gen_set_params;
static OSSL_FUNC_keymgmt_gen_settable_params_fn dh_gen_settable_params;
static OSSL_FUNC_keymgmt_gen_fn dh_gen;
static OSSL_FUNC_keymgmt_gen_cleanup_fn dh_gen_cleanup;
static OSSL_FUNC_keymgmt_load_fn dh_load;
static OSSL_FUNC_keymgmt_get_params_fn dh_get_params;
static OSSL_FUNC_keymgmt_gettable_params_fn dh_gettable_params;
static OSSL_FUNC_keymgmt_set_params_fn dh_set_params;
static OSSL_FUNC_keymgmt_settable_params_fn dh_settable_params;
static OSSL_FUNC_keymgmt_has_fn dh_has;
static OSSL_FUNC_keymgmt_match_fn dh_match;
static OSSL_FUNC_keymgmt_validate_fn dh_validate;
static OSSL_FUNC_keymgmt_import_fn dh_import;
static OSSL_FUNC_keymgmt_import_types_fn dh_import_types;
static OSSL_FUNC_keymgmt_export_fn dh_export;
static OSSL_FUNC_keymgmt_export_types_fn dh_export_types;

#define DH_POSSIBLE_SELECTIONS                                                 \
    (OSSL_KEYMGMT_SELECT_KEYPAIR | OSSL_KEYMGMT_SELECT_DOMAIN_PARAMETERS)

struct dh_gen_ctx {
    OPENSSL_CTX *libctx;

    FFC_PARAMS *ffc_params;
    int selection;
    /* All these parameters are used for parameter generation only */
    /* If there is a group name then the remaining parameters are not needed */
    int group_nid;
    size_t pbits;
    size_t qbits;
    unsigned char *seed; /* optional FIPS186-4 param for testing */
    size_t seedlen;
    int gindex; /* optional  FIPS186-4 generator index (ignored if -1) */
    int gen_type; /* see dhtype2id */
    int generator; /* Used by DH_PARAMGEN_TYPE_GENERATOR in non fips mode only */
    int pcounter;
    int hindex;
    int priv_len;

    const char *mdname;
    const char *mdprops;
    OSSL_CALLBACK *cb;
    void *cbarg;
    int dh_type;
};

typedef struct dh_name2id_st{
    const char *name;
    int id;
} DH_GENTYPE_NAME2ID;

static const DH_GENTYPE_NAME2ID dhtype2id[]=
{
    { "default", DH_PARAMGEN_TYPE_FIPS_186_4 },
    { "fips186_4", DH_PARAMGEN_TYPE_FIPS_186_4 },
    { "fips186_2", DH_PARAMGEN_TYPE_FIPS_186_2 },
    { "group", DH_PARAMGEN_TYPE_GROUP },
    { "generator", DH_PARAMGEN_TYPE_GENERATOR }
};

const char *dh_gen_type_id2name(int id)
{
    size_t i;

    for (i = 0; i < OSSL_NELEM(dhtype2id); ++i) {
        if (dhtype2id[i].id == id)
            return dhtype2id[i].name;
    }
    return NULL;
}

static int dh_gen_type_name2id(const char *name)
{
    size_t i;

    for (i = 0; i < OSSL_NELEM(dhtype2id); ++i) {
        if (strcmp(dhtype2id[i].name, name) == 0)
            return dhtype2id[i].id;
    }
    return -1;
}

static int dh_key_todata(DH *dh, OSSL_PARAM_BLD *bld, OSSL_PARAM params[])
{
    const BIGNUM *priv = NULL, *pub = NULL;

    if (dh == NULL)
        return 0;

    DH_get0_key(dh, &pub, &priv);
    if (priv != NULL
        && !ossl_param_build_set_bn(bld, params, OSSL_PKEY_PARAM_PRIV_KEY, priv))
        return 0;
    if (pub != NULL
        && !ossl_param_build_set_bn(bld, params, OSSL_PKEY_PARAM_PUB_KEY, pub))
        return 0;

    return 1;
}

static void *dh_newdata(void *provctx)
{
    DH *dh = NULL;

    if (ossl_prov_is_running()) {
        dh = dh_new_with_libctx(PROV_LIBRARY_CONTEXT_OF(provctx));
        if (dh != NULL) {
            DH_clear_flags(dh, DH_FLAG_TYPE_MASK);
            DH_set_flags(dh, DH_FLAG_TYPE_DH);
        }
    }
    return dh;
}

static void *dhx_newdata(void *provctx)
{
    DH *dh = NULL;

    dh = dh_new_with_libctx(PROV_LIBRARY_CONTEXT_OF(provctx));
    if (dh != NULL) {
        DH_clear_flags(dh, DH_FLAG_TYPE_MASK);
        DH_set_flags(dh, DH_FLAG_TYPE_DHX);
    }
    return dh;
}

static void dh_freedata(void *keydata)
{
    DH_free(keydata);
}

static int dh_has(void *keydata, int selection)
{
    DH *dh = keydata;
    int ok = 0;

    if (ossl_prov_is_running() && dh != NULL) {
        if ((selection & DH_POSSIBLE_SELECTIONS) != 0)
            ok = 1;

        if ((selection & OSSL_KEYMGMT_SELECT_PUBLIC_KEY) != 0)
            ok = ok && (DH_get0_pub_key(dh) != NULL);
        if ((selection & OSSL_KEYMGMT_SELECT_PRIVATE_KEY) != 0)
            ok = ok && (DH_get0_priv_key(dh) != NULL);
        if ((selection & OSSL_KEYMGMT_SELECT_DOMAIN_PARAMETERS) != 0)
            ok = ok && (DH_get0_p(dh) != NULL && DH_get0_g(dh) != NULL);
    }
    return ok;
}

static int dh_match(const void *keydata1, const void *keydata2, int selection)
{
    const DH *dh1 = keydata1;
    const DH *dh2 = keydata2;
    int ok = 1;

    if (!ossl_prov_is_running())
        return 0;

    if ((selection & OSSL_KEYMGMT_SELECT_PUBLIC_KEY) != 0)
        ok = ok && BN_cmp(DH_get0_pub_key(dh1), DH_get0_pub_key(dh2)) == 0;
    if ((selection & OSSL_KEYMGMT_SELECT_PRIVATE_KEY) != 0)
        ok = ok && BN_cmp(DH_get0_priv_key(dh1), DH_get0_priv_key(dh2)) == 0;
    if ((selection & OSSL_KEYMGMT_SELECT_DOMAIN_PARAMETERS) != 0) {
        FFC_PARAMS *dhparams1 = dh_get0_params((DH *)dh1);
        FFC_PARAMS *dhparams2 = dh_get0_params((DH *)dh2);

        ok = ok && ffc_params_cmp(dhparams1, dhparams2, 1);
    }
    return ok;
}

static int dh_import(void *keydata, int selection, const OSSL_PARAM params[])
{
    DH *dh = keydata;
    int ok = 1;

    if (!ossl_prov_is_running() || dh == NULL)
        return 0;

    if ((selection & DH_POSSIBLE_SELECTIONS) == 0)
        return 0;

    if ((selection & OSSL_KEYMGMT_SELECT_ALL_PARAMETERS) != 0)
        ok = ok && dh_ffc_params_fromdata(dh, params);

    if ((selection & OSSL_KEYMGMT_SELECT_KEYPAIR) != 0)
        ok = ok && dh_key_fromdata(dh, params);

    return ok;
}

static int dh_export(void *keydata, int selection, OSSL_CALLBACK *param_cb,
                     void *cbarg)
{
    DH *dh = keydata;
    OSSL_PARAM_BLD *tmpl = NULL;
    OSSL_PARAM *params = NULL;
    int ok = 1;

    if (!ossl_prov_is_running() || dh == NULL)
        return 0;

    tmpl = OSSL_PARAM_BLD_new();
    if (tmpl == NULL)
        return 0;

    if ((selection & OSSL_KEYMGMT_SELECT_ALL_PARAMETERS) != 0)
        ok = ok && ffc_params_todata(dh_get0_params(dh), tmpl, NULL);
    if ((selection & OSSL_KEYMGMT_SELECT_KEYPAIR) != 0)
        ok = ok && dh_key_todata(dh, tmpl, NULL);

    if (!ok
        || (params = OSSL_PARAM_BLD_to_param(tmpl)) == NULL) {
        ok = 0;
        goto err;
    }
    ok = param_cb(params, cbarg);
    OSSL_PARAM_BLD_free_params(params);
err:
    OSSL_PARAM_BLD_free(tmpl);
    return ok;
}

/* IMEXPORT = IMPORT + EXPORT */

# define DH_IMEXPORTABLE_PARAMETERS                                            \
    OSSL_PARAM_BN(OSSL_PKEY_PARAM_FFC_P, NULL, 0),                             \
    OSSL_PARAM_BN(OSSL_PKEY_PARAM_FFC_Q, NULL, 0),                             \
    OSSL_PARAM_BN(OSSL_PKEY_PARAM_FFC_G, NULL, 0),                             \
    OSSL_PARAM_BN(OSSL_PKEY_PARAM_FFC_COFACTOR, NULL, 0),                      \
    OSSL_PARAM_int(OSSL_PKEY_PARAM_FFC_GINDEX, NULL),                          \
    OSSL_PARAM_int(OSSL_PKEY_PARAM_FFC_PCOUNTER, NULL),                        \
    OSSL_PARAM_int(OSSL_PKEY_PARAM_FFC_H, NULL),                               \
    OSSL_PARAM_octet_string(OSSL_PKEY_PARAM_FFC_SEED, NULL, 0),                \
    OSSL_PARAM_utf8_string(OSSL_PKEY_PARAM_GROUP_NAME, NULL, 0)
# define DH_IMEXPORTABLE_PUBLIC_KEY                                            \
    OSSL_PARAM_BN(OSSL_PKEY_PARAM_PUB_KEY, NULL, 0)
# define DH_IMEXPORTABLE_PRIVATE_KEY                                           \
    OSSL_PARAM_BN(OSSL_PKEY_PARAM_PRIV_KEY, NULL, 0)
static const OSSL_PARAM dh_all_types[] = {
    DH_IMEXPORTABLE_PARAMETERS,
    DH_IMEXPORTABLE_PUBLIC_KEY,
    DH_IMEXPORTABLE_PRIVATE_KEY,
    OSSL_PARAM_END
};
static const OSSL_PARAM dh_parameter_types[] = {
    DH_IMEXPORTABLE_PARAMETERS,
    OSSL_PARAM_END
};
static const OSSL_PARAM dh_key_types[] = {
    DH_IMEXPORTABLE_PUBLIC_KEY,
    DH_IMEXPORTABLE_PRIVATE_KEY,
    OSSL_PARAM_END
};
static const OSSL_PARAM *dh_types[] = {
    NULL,                        /* Index 0 = none of them */
    dh_parameter_types,          /* Index 1 = parameter types */
    dh_key_types,                /* Index 2 = key types */
    dh_all_types                 /* Index 3 = 1 + 2 */
};

static const OSSL_PARAM *dh_imexport_types(int selection)
{
    int type_select = 0;

    if ((selection & OSSL_KEYMGMT_SELECT_ALL_PARAMETERS) != 0)
        type_select += 1;
    if ((selection & OSSL_KEYMGMT_SELECT_KEYPAIR) != 0)
        type_select += 2;
    return dh_types[type_select];
}

static const OSSL_PARAM *dh_import_types(int selection)
{
    return dh_imexport_types(selection);
}

static const OSSL_PARAM *dh_export_types(int selection)
{
    return dh_imexport_types(selection);
}

static ossl_inline int dh_get_params(void *key, OSSL_PARAM params[])
{
    DH *dh = key;
    OSSL_PARAM *p;

    if ((p = OSSL_PARAM_locate(params, OSSL_PKEY_PARAM_BITS)) != NULL
        && !OSSL_PARAM_set_int(p, DH_bits(dh)))
        return 0;
    if ((p = OSSL_PARAM_locate(params, OSSL_PKEY_PARAM_SECURITY_BITS)) != NULL
        && !OSSL_PARAM_set_int(p, DH_security_bits(dh)))
        return 0;
    if ((p = OSSL_PARAM_locate(params, OSSL_PKEY_PARAM_MAX_SIZE)) != NULL
        && !OSSL_PARAM_set_int(p, DH_size(dh)))
        return 0;
    if ((p = OSSL_PARAM_locate(params, OSSL_PKEY_PARAM_TLS_ENCODED_PT)) != NULL) {
        if (p->data_type != OSSL_PARAM_OCTET_STRING)
            return 0;
        p->return_size = dh_key2buf(dh, (unsigned char **)&p->data,
                                    p->data_size, 0);
        if (p->return_size == 0)
            return 0;
    }

    return ffc_params_todata(dh_get0_params(dh), NULL, params)
           && dh_key_todata(dh, NULL, params);
}

static const OSSL_PARAM dh_params[] = {
    OSSL_PARAM_int(OSSL_PKEY_PARAM_BITS, NULL),
    OSSL_PARAM_int(OSSL_PKEY_PARAM_SECURITY_BITS, NULL),
    OSSL_PARAM_int(OSSL_PKEY_PARAM_MAX_SIZE, NULL),
    OSSL_PARAM_octet_string(OSSL_PKEY_PARAM_TLS_ENCODED_PT, NULL, 0),
    DH_IMEXPORTABLE_PARAMETERS,
    DH_IMEXPORTABLE_PUBLIC_KEY,
    DH_IMEXPORTABLE_PRIVATE_KEY,
    OSSL_PARAM_END
};

static const OSSL_PARAM *dh_gettable_params(void *provctx)
{
    return dh_params;
}

static const OSSL_PARAM dh_known_settable_params[] = {
    OSSL_PARAM_octet_string(OSSL_PKEY_PARAM_TLS_ENCODED_PT, NULL, 0),
    OSSL_PARAM_END
};

static const OSSL_PARAM *dh_settable_params(void *provctx)
{
    return dh_known_settable_params;
}

static int dh_set_params(void *key, const OSSL_PARAM params[])
{
    DH *dh = key;
    const OSSL_PARAM *p;

    p = OSSL_PARAM_locate_const(params, OSSL_PKEY_PARAM_TLS_ENCODED_PT);
    if (p != NULL
            && (p->data_type != OSSL_PARAM_OCTET_STRING
                || !dh_buf2key(dh, p->data, p->data_size)))
        return 0;

    return 1;
}

static int dh_validate_public(DH *dh)
{
    const BIGNUM *pub_key = NULL;

    DH_get0_key(dh, &pub_key, NULL);
    if (pub_key == NULL)
        return 0;
    return DH_check_pub_key_ex(dh, pub_key);
}

static int dh_validate_private(DH *dh)
{
    int status = 0;
    const BIGNUM *priv_key = NULL;

    DH_get0_key(dh, NULL, &priv_key);
    if (priv_key == NULL)
        return 0;
    return dh_check_priv_key(dh, priv_key, &status);;
}

static int dh_validate(void *keydata, int selection)
{
    DH *dh = keydata;
    int ok = 0;

    if (!ossl_prov_is_running())
        return 0;

    if ((selection & DH_POSSIBLE_SELECTIONS) != 0)
        ok = 1;

    if ((selection & OSSL_KEYMGMT_SELECT_DOMAIN_PARAMETERS) != 0)
        ok = ok && DH_check_params_ex(dh);

    if ((selection & OSSL_KEYMGMT_SELECT_PUBLIC_KEY) != 0)
        ok = ok && dh_validate_public(dh);

    if ((selection & OSSL_KEYMGMT_SELECT_PRIVATE_KEY) != 0)
        ok = ok && dh_validate_private(dh);

    if ((selection & OSSL_KEYMGMT_SELECT_KEYPAIR)
            == OSSL_KEYMGMT_SELECT_KEYPAIR)
        ok = ok && dh_check_pairwise(dh);
    return ok;
}

static void *dh_gen_init_base(void *provctx, int selection, int type)
{
    OPENSSL_CTX *libctx = PROV_LIBRARY_CONTEXT_OF(provctx);
    struct dh_gen_ctx *gctx = NULL;

    if (!ossl_prov_is_running())
        return NULL;

    if ((selection & (OSSL_KEYMGMT_SELECT_KEYPAIR
                      | OSSL_KEYMGMT_SELECT_DOMAIN_PARAMETERS)) == 0)
        return NULL;

    if ((gctx = OPENSSL_zalloc(sizeof(*gctx))) != NULL) {
        gctx->selection = selection;
        gctx->libctx = libctx;
        gctx->pbits = 2048;
        gctx->qbits = 224;
        gctx->mdname = NULL;
        gctx->gen_type = DH_PARAMGEN_TYPE_FIPS_186_4;
        gctx->gindex = -1;
        gctx->hindex = 0;
        gctx->pcounter = -1;
        gctx->generator = DH_GENERATOR_2;
        gctx->dh_type = type;
    }
    return gctx;
}

static void *dh_gen_init(void *provctx, int selection)
{
    return dh_gen_init_base(provctx, selection, DH_FLAG_TYPE_DH);
}

static void *dhx_gen_init(void *provctx, int selection)
{
   return dh_gen_init_base(provctx, selection, DH_FLAG_TYPE_DHX);
}

static int dh_gen_set_template(void *genctx, void *templ)
{
    struct dh_gen_ctx *gctx = genctx;
    DH *dh = templ;

    if (!ossl_prov_is_running() || gctx == NULL || dh == NULL)
        return 0;
    gctx->ffc_params = dh_get0_params(dh);
    return 1;
}

static int dh_set_gen_seed(struct dh_gen_ctx *gctx, unsigned char *seed,
                           size_t seedlen)
{
    OPENSSL_clear_free(gctx->seed, gctx->seedlen);
    gctx->seed = NULL;
    gctx->seedlen = 0;
    if (seed != NULL && seedlen > 0) {
        gctx->seed = OPENSSL_memdup(seed, seedlen);
        if (gctx->seed == NULL)
            return 0;
        gctx->seedlen = seedlen;
    }
    return 1;
}

static int dh_gen_set_params(void *genctx, const OSSL_PARAM params[])
{
    struct dh_gen_ctx *gctx = genctx;
    const OSSL_PARAM *p;

    if (gctx == NULL)
        return 0;

    p = OSSL_PARAM_locate_const(params, OSSL_PKEY_PARAM_FFC_TYPE);
    if (p != NULL) {
        if (p->data_type != OSSL_PARAM_UTF8_STRING
            || ((gctx->gen_type = dh_gen_type_name2id(p->data)) == -1)) {
            ERR_raise(ERR_LIB_PROV, ERR_R_PASSED_INVALID_ARGUMENT);
            return 0;
        }
    }
    p = OSSL_PARAM_locate_const(params, OSSL_PKEY_PARAM_GROUP_NAME);
    if (p != NULL) {
        if (p->data_type != OSSL_PARAM_UTF8_STRING
           || ((gctx->group_nid = ffc_named_group_to_uid(p->data)) == NID_undef)) {
            ERR_raise(ERR_LIB_PROV, ERR_R_PASSED_INVALID_ARGUMENT);
            return 0;
        }
        gctx->gen_type = DH_PARAMGEN_TYPE_GROUP;
    }
    p = OSSL_PARAM_locate_const(params, OSSL_PKEY_PARAM_DH_GENERATOR);
    if (p != NULL && !OSSL_PARAM_get_int(p, &gctx->generator))
        return 0;
    p = OSSL_PARAM_locate_const(params, OSSL_PKEY_PARAM_FFC_GINDEX);
    if (p != NULL && !OSSL_PARAM_get_int(p, &gctx->gindex))
        return 0;
    p = OSSL_PARAM_locate_const(params, OSSL_PKEY_PARAM_FFC_PCOUNTER);
    if (p != NULL && !OSSL_PARAM_get_int(p, &gctx->pcounter))
        return 0;
    p = OSSL_PARAM_locate_const(params, OSSL_PKEY_PARAM_FFC_H);
    if (p != NULL && !OSSL_PARAM_get_int(p, &gctx->hindex))
        return 0;
    p = OSSL_PARAM_locate_const(params, OSSL_PKEY_PARAM_FFC_SEED);
    if (p != NULL
        && (p->data_type != OSSL_PARAM_OCTET_STRING
            || !dh_set_gen_seed(gctx, p->data, p->data_size)))
            return 0;

    if ((p = OSSL_PARAM_locate_const(params, OSSL_PKEY_PARAM_FFC_PBITS)) != NULL
        && !OSSL_PARAM_get_size_t(p, &gctx->pbits))
        return 0;
    if ((p = OSSL_PARAM_locate_const(params, OSSL_PKEY_PARAM_FFC_QBITS)) != NULL
        && !OSSL_PARAM_get_size_t(p, &gctx->qbits))
        return 0;
    p = OSSL_PARAM_locate_const(params, OSSL_PKEY_PARAM_FFC_DIGEST);
    if (p != NULL) {
        if (p->data_type != OSSL_PARAM_UTF8_STRING)
            return 0;
        gctx->mdname = p->data;
    }
    p = OSSL_PARAM_locate_const(params, OSSL_PKEY_PARAM_FFC_DIGEST_PROPS);
    if (p != NULL) {
        if (p->data_type != OSSL_PARAM_UTF8_STRING)
            return 0;
        gctx->mdprops = p->data;
    }
    p = OSSL_PARAM_locate_const(params, OSSL_PKEY_PARAM_DH_PRIV_LEN);
    if (p != NULL && !OSSL_PARAM_get_int(p, &gctx->priv_len))
        return 0;
    return 1;
}

static const OSSL_PARAM *dh_gen_settable_params(void *provctx)
{
    static OSSL_PARAM settable[] = {
        OSSL_PARAM_utf8_string(OSSL_PKEY_PARAM_GROUP_NAME, NULL, 0),
        OSSL_PARAM_int(OSSL_PKEY_PARAM_DH_PRIV_LEN, NULL),
        OSSL_PARAM_int(OSSL_PKEY_PARAM_DH_GENERATOR, NULL),
        OSSL_PARAM_utf8_string(OSSL_PKEY_PARAM_FFC_TYPE, NULL, 0),
        OSSL_PARAM_size_t(OSSL_PKEY_PARAM_FFC_PBITS, NULL),
        OSSL_PARAM_size_t(OSSL_PKEY_PARAM_FFC_QBITS, NULL),
        OSSL_PARAM_utf8_string(OSSL_PKEY_PARAM_FFC_DIGEST, NULL, 0),
        OSSL_PARAM_utf8_string(OSSL_PKEY_PARAM_FFC_DIGEST_PROPS, NULL, 0),
        OSSL_PARAM_int(OSSL_PKEY_PARAM_FFC_GINDEX, NULL),
        OSSL_PARAM_octet_string(OSSL_PKEY_PARAM_FFC_SEED, NULL, 0),
        OSSL_PARAM_int(OSSL_PKEY_PARAM_FFC_PCOUNTER, NULL),
        OSSL_PARAM_int(OSSL_PKEY_PARAM_FFC_H, NULL),
        OSSL_PARAM_END
    };
    return settable;
}

static int dh_gencb(int p, int n, BN_GENCB *cb)
{
    struct dh_gen_ctx *gctx = BN_GENCB_get_arg(cb);
    OSSL_PARAM params[] = { OSSL_PARAM_END, OSSL_PARAM_END, OSSL_PARAM_END };

    params[0] = OSSL_PARAM_construct_int(OSSL_GEN_PARAM_POTENTIAL, &p);
    params[1] = OSSL_PARAM_construct_int(OSSL_GEN_PARAM_ITERATION, &n);

    return gctx->cb(params, gctx->cbarg);
}

static void *dh_gen(void *genctx, OSSL_CALLBACK *osslcb, void *cbarg)
{
    int ret = 0;
    struct dh_gen_ctx *gctx = genctx;
    DH *dh = NULL;
    BN_GENCB *gencb = NULL;
    FFC_PARAMS *ffc;

    if (!ossl_prov_is_running() || gctx == NULL)
        return NULL;

    /* For parameter generation - If there is a group name just create it */
    if (gctx->gen_type == DH_PARAMGEN_TYPE_GROUP) {
        /* Select a named group if there is not one already */
        if (gctx->group_nid == NID_undef)
            gctx->group_nid = dh_get_named_group_uid_from_size(gctx->pbits);
        if (gctx->group_nid == NID_undef)
            return NULL;
        dh = dh_new_by_nid_with_libctx(gctx->libctx, gctx->group_nid);
        if (dh == NULL)
            return NULL;
        ffc = dh_get0_params(dh);
    } else {
        dh = dh_new_with_libctx(gctx->libctx);
        if (dh == NULL)
            return NULL;
        ffc = dh_get0_params(dh);

        /* Copy the template value if one was passed */
        if (gctx->ffc_params != NULL
            && !ffc_params_copy(ffc, gctx->ffc_params))
            goto end;

        if (!ffc_params_set_seed(ffc, gctx->seed, gctx->seedlen))
            goto end;
        if (gctx->gindex != -1) {
            ffc_params_set_gindex(ffc, gctx->gindex);
            if (gctx->pcounter != -1)
                ffc_params_set_pcounter(ffc, gctx->pcounter);
        } else if (gctx->hindex != 0) {
            ffc_params_set_h(ffc, gctx->hindex);
        }
        if (gctx->mdname != NULL) {
            if (!ffc_set_digest(ffc, gctx->mdname, gctx->mdprops))
                goto end;
        }
        gctx->cb = osslcb;
        gctx->cbarg = cbarg;
        gencb = BN_GENCB_new();
        if (gencb != NULL)
            BN_GENCB_set(gencb, dh_gencb, genctx);

        if ((gctx->selection & OSSL_KEYMGMT_SELECT_DOMAIN_PARAMETERS) != 0) {
            /*
             * NOTE: The old safe prime generator code is not used in fips mode,
             * (i.e internally it ignores the generator and chooses a named
             * group based on pbits.
             */
            if (gctx->gen_type == DH_PARAMGEN_TYPE_GENERATOR)
                ret = DH_generate_parameters_ex(dh, gctx->pbits,
                                                gctx->generator, gencb);
            else
                ret = dh_generate_ffc_parameters(dh, gctx->gen_type, gctx->pbits,
                                                 gctx->qbits, gencb);
            if (ret <= 0)
                goto end;
        }
    }

    if ((gctx->selection & OSSL_KEYMGMT_SELECT_KEYPAIR) != 0) {
        if (ffc->p == NULL || ffc->g == NULL)
            goto end;
        if (gctx->priv_len > 0)
            DH_set_length(dh, (long)gctx->priv_len);
        ffc_params_enable_flags(ffc, FFC_PARAM_FLAG_VALIDATE_LEGACY,
                                gctx->gen_type == DH_PARAMGEN_TYPE_FIPS_186_2);
        if (DH_generate_key(dh) <= 0)
            goto end;
    }
    DH_clear_flags(dh, DH_FLAG_TYPE_MASK);
    DH_set_flags(dh, gctx->dh_type);

    ret = 1;
end:
    if (ret <= 0) {
        DH_free(dh);
        dh = NULL;
    }
    BN_GENCB_free(gencb);
    return dh;
}

static void dh_gen_cleanup(void *genctx)
{
    struct dh_gen_ctx *gctx = genctx;

    if (gctx == NULL)
        return;

    OPENSSL_clear_free(gctx->seed, gctx->seedlen);
    OPENSSL_free(gctx);
}

void *dh_load(const void *reference, size_t reference_sz)
{
    DH *dh = NULL;

    if (ossl_prov_is_running() && reference_sz == sizeof(dh)) {
        /* The contents of the reference is the address to our object */
        dh = *(DH **)reference;
        /* We grabbed, so we detach it */
        *(DH **)reference = NULL;
        return dh;
    }
    return NULL;
}

const OSSL_DISPATCH dh_keymgmt_functions[] = {
    { OSSL_FUNC_KEYMGMT_NEW, (void (*)(void))dh_newdata },
    { OSSL_FUNC_KEYMGMT_GEN_INIT, (void (*)(void))dh_gen_init },
    { OSSL_FUNC_KEYMGMT_GEN_SET_TEMPLATE, (void (*)(void))dh_gen_set_template },
    { OSSL_FUNC_KEYMGMT_GEN_SET_PARAMS, (void (*)(void))dh_gen_set_params },
    { OSSL_FUNC_KEYMGMT_GEN_SETTABLE_PARAMS,
      (void (*)(void))dh_gen_settable_params },
    { OSSL_FUNC_KEYMGMT_GEN, (void (*)(void))dh_gen },
    { OSSL_FUNC_KEYMGMT_GEN_CLEANUP, (void (*)(void))dh_gen_cleanup },
    { OSSL_FUNC_KEYMGMT_LOAD, (void (*)(void))dh_load },
    { OSSL_FUNC_KEYMGMT_FREE, (void (*)(void))dh_freedata },
    { OSSL_FUNC_KEYMGMT_GET_PARAMS, (void (*) (void))dh_get_params },
    { OSSL_FUNC_KEYMGMT_GETTABLE_PARAMS, (void (*) (void))dh_gettable_params },
    { OSSL_FUNC_KEYMGMT_SET_PARAMS, (void (*) (void))dh_set_params },
    { OSSL_FUNC_KEYMGMT_SETTABLE_PARAMS, (void (*) (void))dh_settable_params },
    { OSSL_FUNC_KEYMGMT_HAS, (void (*)(void))dh_has },
    { OSSL_FUNC_KEYMGMT_MATCH, (void (*)(void))dh_match },
    { OSSL_FUNC_KEYMGMT_VALIDATE, (void (*)(void))dh_validate },
    { OSSL_FUNC_KEYMGMT_IMPORT, (void (*)(void))dh_import },
    { OSSL_FUNC_KEYMGMT_IMPORT_TYPES, (void (*)(void))dh_import_types },
    { OSSL_FUNC_KEYMGMT_EXPORT, (void (*)(void))dh_export },
    { OSSL_FUNC_KEYMGMT_EXPORT_TYPES, (void (*)(void))dh_export_types },
    { 0, NULL }
};

/* For any DH key, we use the "DH" algorithms regardless of sub-type. */
static const char *dhx_query_operation_name(int operation_id)
{
    return "DH";
}

const OSSL_DISPATCH dhx_keymgmt_functions[] = {
    { OSSL_FUNC_KEYMGMT_NEW, (void (*)(void))dhx_newdata },
    { OSSL_FUNC_KEYMGMT_GEN_INIT, (void (*)(void))dhx_gen_init },
    { OSSL_FUNC_KEYMGMT_GEN_SET_TEMPLATE, (void (*)(void))dh_gen_set_template },
    { OSSL_FUNC_KEYMGMT_GEN_SET_PARAMS, (void (*)(void))dh_gen_set_params },
    { OSSL_FUNC_KEYMGMT_GEN_SETTABLE_PARAMS,
      (void (*)(void))dh_gen_settable_params },
    { OSSL_FUNC_KEYMGMT_GEN, (void (*)(void))dh_gen },
    { OSSL_FUNC_KEYMGMT_GEN_CLEANUP, (void (*)(void))dh_gen_cleanup },
    { OSSL_FUNC_KEYMGMT_LOAD, (void (*)(void))dh_load },
    { OSSL_FUNC_KEYMGMT_FREE, (void (*)(void))dh_freedata },
    { OSSL_FUNC_KEYMGMT_GET_PARAMS, (void (*) (void))dh_get_params },
    { OSSL_FUNC_KEYMGMT_GETTABLE_PARAMS, (void (*) (void))dh_gettable_params },
    { OSSL_FUNC_KEYMGMT_SET_PARAMS, (void (*) (void))dh_set_params },
    { OSSL_FUNC_KEYMGMT_SETTABLE_PARAMS, (void (*) (void))dh_settable_params },
    { OSSL_FUNC_KEYMGMT_HAS, (void (*)(void))dh_has },
    { OSSL_FUNC_KEYMGMT_MATCH, (void (*)(void))dh_match },
    { OSSL_FUNC_KEYMGMT_VALIDATE, (void (*)(void))dh_validate },
    { OSSL_FUNC_KEYMGMT_IMPORT, (void (*)(void))dh_import },
    { OSSL_FUNC_KEYMGMT_IMPORT_TYPES, (void (*)(void))dh_import_types },
    { OSSL_FUNC_KEYMGMT_EXPORT, (void (*)(void))dh_export },
    { OSSL_FUNC_KEYMGMT_EXPORT_TYPES, (void (*)(void))dh_export_types },
    { OSSL_FUNC_KEYMGMT_QUERY_OPERATION_NAME,
      (void (*)(void))dhx_query_operation_name },
    { 0, NULL }
};
