/*
 * Copyright 2019 The OpenSSL Project Authors. All Rights Reserved.
 * Copyright (c) 2019, Oracle and/or its affiliates.  All rights reserved.
 *
 * Licensed under the Apache License 2.0 (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://www.openssl.org/source/license.html
 */

#include <string.h>
#include <openssl/crypto.h>
#include <openssl/lhash.h>
#include "crypto/lhash.h"
#include "property_local.h"

/*
 * Property strings are a consolidation of all strings seen by the property
 * subsystem.  There are two name spaces to keep property names separate from
 * property values (numeric values are not expected to be cached however).
 * They allow a rapid conversion from a string to a unique index and any
 * subsequent string comparison can be done via an integer compare.
 *
 * This implementation uses OpenSSL's standard hash table.  There are more
 * space and time efficient algorithms if this becomes a bottleneck.
 */

typedef struct {
    const char *s;
    OSSL_PROPERTY_IDX idx;
    char body[1];
} PROPERTY_STRING;

DEFINE_LHASH_OF(PROPERTY_STRING);
typedef LHASH_OF(PROPERTY_STRING) PROP_TABLE;

typedef struct {
    PROP_TABLE *prop_names;
    PROP_TABLE *prop_values;
    OSSL_PROPERTY_IDX prop_name_idx;
    OSSL_PROPERTY_IDX prop_value_idx;
} PROPERTY_STRING_DATA;

static unsigned long property_hash(const PROPERTY_STRING *a)
{
    return OPENSSL_LH_strhash(a->s);
}

static int property_cmp(const PROPERTY_STRING *a, const PROPERTY_STRING *b)
{
    return strcmp(a->s, b->s);
}

static void property_free(PROPERTY_STRING *ps)
{
    OPENSSL_free(ps);
}

static void property_table_free(PROP_TABLE **pt)
{
    PROP_TABLE *t = *pt;

    if (t != NULL) {
        lh_PROPERTY_STRING_doall(t, &property_free);
        lh_PROPERTY_STRING_free(t);
        *pt = NULL;
    }
}

static void property_string_data_free(void *vpropdata)
{
    PROPERTY_STRING_DATA *propdata = vpropdata;

    if (propdata == NULL)
        return;

    property_table_free(&propdata->prop_names);
    property_table_free(&propdata->prop_values);
    propdata->prop_name_idx = propdata->prop_value_idx = 0;

    OPENSSL_free(propdata);
}

static void *property_string_data_new(OPENSSL_CTX *ctx) {
    PROPERTY_STRING_DATA *propdata = OPENSSL_zalloc(sizeof(*propdata));

    if (propdata == NULL)
        return NULL;

    propdata->prop_names = lh_PROPERTY_STRING_new(&property_hash,
                                                  &property_cmp);
    if (propdata->prop_names == NULL)
        goto err;

    propdata->prop_values = lh_PROPERTY_STRING_new(&property_hash,
                                                   &property_cmp);
    if (propdata->prop_values == NULL)
        goto err;

    return propdata;

err:
    property_string_data_free(propdata);
    return NULL;
}

static const OPENSSL_CTX_METHOD property_string_data_method = {
    property_string_data_new,
    property_string_data_free,
};

static PROPERTY_STRING *new_property_string(const char *s,
                                            OSSL_PROPERTY_IDX *pidx)
{
    const size_t l = strlen(s);
    PROPERTY_STRING *ps = OPENSSL_malloc(sizeof(*ps) + l);

    if (ps != NULL) {
        memcpy(ps->body, s, l + 1);
        ps->s = ps->body;
        ps->idx = ++*pidx;
        if (ps->idx == 0) {
            OPENSSL_free(ps);
            return NULL;
        }
    }
    return ps;
}

static OSSL_PROPERTY_IDX ossl_property_string(PROP_TABLE *t,
                                              OSSL_PROPERTY_IDX *pidx,
                                              const char *s)
{
    PROPERTY_STRING p, *ps, *ps_new;

    p.s = s;
    ps = lh_PROPERTY_STRING_retrieve(t, &p);
    if (ps == NULL && pidx != NULL)
        if ((ps_new = new_property_string(s, pidx)) != NULL) {
            lh_PROPERTY_STRING_insert(t, ps_new);
            if (lh_PROPERTY_STRING_error(t)) {
                property_free(ps_new);
                return 0;
            }
            ps = ps_new;
        }
    return ps != NULL ? ps->idx : 0;
}

OSSL_PROPERTY_IDX ossl_property_name(OPENSSL_CTX *ctx, const char *s,
                                     int create)
{
    PROPERTY_STRING_DATA *propdata
        = openssl_ctx_get_data(ctx, OPENSSL_CTX_PROPERTY_STRING_INDEX,
                               &property_string_data_method);

    if (propdata == NULL)
        return 0;
    return ossl_property_string(propdata->prop_names,
                                create ? &propdata->prop_name_idx : NULL,
                                s);
}

OSSL_PROPERTY_IDX ossl_property_value(OPENSSL_CTX *ctx, const char *s,
                                      int create)
{
    PROPERTY_STRING_DATA *propdata
        = openssl_ctx_get_data(ctx, OPENSSL_CTX_PROPERTY_STRING_INDEX,
                               &property_string_data_method);

    if (propdata == NULL)
        return 0;
    return ossl_property_string(propdata->prop_values,
                                create ? &propdata->prop_value_idx : NULL,
                                s);
}
