/*
 * Copyright 2019-2020 The OpenSSL Project Authors. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * https://www.openssl.org/source/license.html
 * or in the file LICENSE in the source distribution.
 */

/*
 * This program tests the use of OSSL_PARAM, currently in raw form.
 */

#include <string.h>
#include <openssl/bn.h>
#include <openssl/core.h>
#include <openssl/params.h>
#include "internal/nelem.h"
#include "testutil.h"

/*-
 * PROVIDER SECTION
 * ================
 *
 * Even though it's not necessarily ONLY providers doing this part,
 * they are naturally going to be the most common users of
 * set_params and get_params functions.
 */

/*
 * In real use cases, setters and getters would take an object with
 * which the parameters are associated.  This structure is a cheap
 * simulation.
 */
struct object_st {
    /*
     * Documented as a native integer, of the size given by sizeof(int).
     * Assumed data type OSSL_PARAM_INTEGER
     */
    int p1;
    /*
     * Documented as a native double, of the size given by sizeof(double).
     * Assumed data type OSSL_PARAM_REAL
     */
    double p2;
    /*
     * Documented as an arbitrarly large unsigned integer.
     * The data size must be large enough to accommodate.
     * Assumed data type OSSL_PARAM_UNSIGNED_INTEGER
     */
    BIGNUM *p3;
    /*
     * Documented as a C string.
     * The data size must be large enough to accommodate.
     * Assumed data type OSSL_PARAM_UTF8_STRING
     */
    char *p4;
    size_t p4_l;
    /*
     * Documented as a C string.
     * Assumed data type OSSL_PARAM_UTF8_STRING
     */
    char p5[256];
    size_t p5_l;
    /*
     * Documented as a pointer to a constant C string.
     * Assumed data type OSSL_PARAM_UTF8_PTR
     */
    const char *p6;
    size_t p6_l;
};

#define p1_init 42                              /* The ultimate answer */
#define p2_init 6.283                           /* Magic number */
/* Stolen from evp_data, BLAKE2s256 test */
#define p3_init                                 \
    "4142434445464748494a4b4c4d4e4f50"          \
    "5152535455565758595a616263646566"          \
    "6768696a6b6c6d6e6f70717273747576"          \
    "7778797a30313233343536373839"
#define p4_init "BLAKE2s256"                    /* Random string */
#define p5_init "Hellow World"                  /* Random string */
#define p6_init OPENSSL_FULL_VERSION_STR        /* Static string */

static void cleanup_object(void *vobj)
{
    struct object_st *obj = vobj;

    BN_free(obj->p3);
    obj->p3 = NULL;
    OPENSSL_free(obj->p4);
    obj->p4 = NULL;
    OPENSSL_free(obj);
}

static void *init_object(void)
{
    struct object_st *obj = OPENSSL_zalloc(sizeof(*obj));

    obj->p1 = p1_init;
    obj->p2 = p2_init;
    if (!TEST_true(BN_hex2bn(&obj->p3, p3_init)))
        goto fail;
    if (!TEST_ptr(obj->p4 = OPENSSL_strdup(p4_init)))
        goto fail;
    strcpy(obj->p5, p5_init);
    obj->p6 = p6_init;

    return obj;
 fail:
    cleanup_object(obj);
    obj = NULL;

    return NULL;
}

/*
 * RAW provider, which handles the parameters in a very raw manner,
 * with no fancy API and very minimal checking.  The application that
 * calls these to set or request parameters MUST get its OSSL_PARAM
 * array right.
 */

static int raw_set_params(void *vobj, const OSSL_PARAM *params)
{
    struct object_st *obj = vobj;

    for (; params->key != NULL; params++)
        if (strcmp(params->key, "p1") == 0) {
            obj->p1 = *(int *)params->data;
        } else if (strcmp(params->key, "p2") == 0) {
            obj->p2 = *(double *)params->data;
        } else if (strcmp(params->key, "p3") == 0) {
            BN_free(obj->p3);
            if (!TEST_ptr(obj->p3 = BN_native2bn(params->data,
                                                 params->data_size, NULL)))
                return 0;
        } else if (strcmp(params->key, "p4") == 0) {
            OPENSSL_free(obj->p4);
            if (!TEST_ptr(obj->p4 = OPENSSL_strndup(params->data,
                                                    params->data_size)))
                return 0;
        } else if (strcmp(params->key, "p5") == 0) {
            strncpy(obj->p5, params->data, params->data_size);
            obj->p5_l = strlen(obj->p5) + 1;
        } else if (strcmp(params->key, "p6") == 0) {
            obj->p6 = *(const char **)params->data;
            obj->p6_l = params->data_size;
        }

    return 1;
}

static int raw_get_params(void *vobj, OSSL_PARAM *params)
{
    struct object_st *obj = vobj;

    for (; params->key != NULL; params++)
        if (strcmp(params->key, "p1") == 0) {
            params->return_size = sizeof(obj->p1);
            *(int *)params->data = obj->p1;
        } else if (strcmp(params->key, "p2") == 0) {
            params->return_size = sizeof(obj->p2);
            *(double *)params->data = obj->p2;
        } else if (strcmp(params->key, "p3") == 0) {
            size_t bytes = BN_num_bytes(obj->p3);

            params->return_size = bytes;
            if (!TEST_size_t_ge(params->data_size, bytes))
                return 0;
            BN_bn2nativepad(obj->p3, params->data, bytes);
        } else if (strcmp(params->key, "p4") == 0) {
            size_t bytes = strlen(obj->p4) + 1;

            params->return_size = bytes;
            if (!TEST_size_t_ge(params->data_size, bytes))
                return 0;
            strcpy(params->data, obj->p4);
        } else if (strcmp(params->key, "p5") == 0) {
            size_t bytes = strlen(obj->p5) + 1;

            params->return_size = bytes;
            if (!TEST_size_t_ge(params->data_size, bytes))
                return 0;
            strcpy(params->data, obj->p5);
        } else if (strcmp(params->key, "p6") == 0) {
            /*
             * We COULD also use OPENSSL_FULL_VERSION_STR directly and
             * use sizeof(OPENSSL_FULL_VERSION_STR) instead of calling
             * strlen().
             * The caller wouldn't know the difference.
             */
            size_t bytes = strlen(obj->p6) + 1;

            params->return_size = bytes;
            *(const char **)params->data = obj->p6;
        }

    return 1;
}

/*
 * API provider, which handles the parameters using the API from params.h
 */

static int api_set_params(void *vobj, const OSSL_PARAM *params)
{
    struct object_st *obj = vobj;
    const OSSL_PARAM *p = NULL;

    if ((p = OSSL_PARAM_locate_const(params, "p1")) != NULL
        && !TEST_true(OSSL_PARAM_get_int(p, &obj->p1)))
        return 0;
    if ((p = OSSL_PARAM_locate_const(params, "p2")) != NULL
        && !TEST_true(OSSL_PARAM_get_double(p, &obj->p2)))
        return 0;
    if ((p = OSSL_PARAM_locate_const(params, "p3")) != NULL
        && !TEST_true(OSSL_PARAM_get_BN(p, &obj->p3)))
        return 0;
    if ((p = OSSL_PARAM_locate_const(params, "p4")) != NULL) {
        OPENSSL_free(obj->p4);
        obj->p4 = NULL;
        /* If the value pointer is NULL, we get it automatically allocated */
        if (!TEST_true(OSSL_PARAM_get_utf8_string(p, &obj->p4, 0)))
            return 0;
    }
    if ((p = OSSL_PARAM_locate_const(params, "p5")) != NULL) {
        char *p5_ptr = obj->p5;
        if (!TEST_true(OSSL_PARAM_get_utf8_string(p, &p5_ptr, sizeof(obj->p5))))
            return 0;
        obj->p5_l = strlen(obj->p5) + 1;
    }
    if ((p = OSSL_PARAM_locate_const(params, "p6")) != NULL) {
        if (!TEST_true(OSSL_PARAM_get_utf8_ptr(p, &obj->p6)))
            return 0;
        obj->p6_l = strlen(obj->p6) + 1;
    }

    return 1;
}

static int api_get_params(void *vobj, OSSL_PARAM *params)
{
    struct object_st *obj = vobj;
    OSSL_PARAM *p = NULL;

    if ((p = OSSL_PARAM_locate(params, "p1")) != NULL
        && !TEST_true(OSSL_PARAM_set_int(p, obj->p1)))
        return 0;
    if ((p = OSSL_PARAM_locate(params, "p2")) != NULL
        && !TEST_true(OSSL_PARAM_set_double(p, obj->p2)))
        return 0;
    if ((p = OSSL_PARAM_locate(params, "p3")) != NULL
        && !TEST_true(OSSL_PARAM_set_BN(p, obj->p3)))
        return 0;
    if ((p = OSSL_PARAM_locate(params, "p4")) != NULL
        && !TEST_true(OSSL_PARAM_set_utf8_string(p, obj->p4)))
        return 0;
    if ((p = OSSL_PARAM_locate(params, "p5")) != NULL
        && !TEST_true(OSSL_PARAM_set_utf8_string(p, obj->p5)))
        return 0;
    if ((p = OSSL_PARAM_locate(params, "p6")) != NULL
        && !TEST_true(OSSL_PARAM_set_utf8_ptr(p, obj->p6)))
        return 0;

    return 1;
}

/*
 * This structure only simulates a provider dispatch, the real deal is
 * a bit more code that's not necessary in these tests.
 */
struct provider_dispatch_st {
    int (*set_params)(void *obj, const OSSL_PARAM *params);
    int (*get_params)(void *obj, OSSL_PARAM *params);
};

/* "raw" provider */
static const struct provider_dispatch_st provider_raw = {
    raw_set_params, raw_get_params
};

/* "api" provider */
static const struct provider_dispatch_st provider_api = {
    api_set_params, api_get_params
};

/*-
 * APPLICATION SECTION
 * ===================
 */

/* In all our tests, these are variables that get manipulated as parameters
 *
 * These arrays consistently do nothing with the "p2" parameter, and
 * always include a "foo" parameter.  This is to check that the
 * set_params and get_params calls ignore the lack of parameters that
 * the application isn't interested in, as well as ignore parameters
 * they don't understand (the application may have one big bag of
 * parameters).
 */
static int app_p1;                    /* "p1" */
static double app_p2;                 /* "p2" is ignored */
static BIGNUM *app_p3 = NULL;         /* "p3" */
static unsigned char bignumbin[4096]; /* "p3" */
static char app_p4[256];              /* "p4" */
static char app_p5[256];              /* "p5" */
static const char *app_p6 = NULL;     /* "p6" */
static unsigned char foo[1];          /* "foo" */

#define app_p1_init 17           /* A random number */
#define app_p2_init 47.11        /* Another random number */
#define app_p3_init "deadbeef"   /* Classic */
#define app_p4_init "Hello"
#define app_p5_init "World"
#define app_p6_init "Cookie"
#define app_foo_init 'z'

static int cleanup_app_variables(void)
{
    BN_free(app_p3);
    app_p3 = NULL;
    return 1;
}

static int init_app_variables(void)
{
    int l = 0;

    cleanup_app_variables();

    app_p1 = app_p1_init;
    app_p2 = app_p2_init;
    if (!BN_hex2bn(&app_p3, app_p3_init)
        || (l = BN_bn2nativepad(app_p3, bignumbin, sizeof(bignumbin))) < 0)
        return 0;
    strcpy(app_p4, app_p4_init);
    strcpy(app_p5, app_p5_init);
    app_p6 = app_p6_init;
    foo[0] = app_foo_init;

    return 1;
}

/*
 * Here, we define test OSSL_PARAM arrays
 */

/* An array of OSSL_PARAM, specific in the most raw manner possible */
static OSSL_PARAM static_raw_params[] = {
    { "p1", OSSL_PARAM_INTEGER, &app_p1, sizeof(app_p1), 0 },
    { "p3", OSSL_PARAM_UNSIGNED_INTEGER, &bignumbin, sizeof(bignumbin), 0 },
    { "p4", OSSL_PARAM_UTF8_STRING, &app_p4, sizeof(app_p4), 0 },
    { "p5", OSSL_PARAM_UTF8_STRING, &app_p5, sizeof(app_p5), 0 },
    /* sizeof(app_p6_init), because we know that's what we're using */
    { "p6", OSSL_PARAM_UTF8_PTR, &app_p6, sizeof(app_p6_init), 0 },
    { "foo", OSSL_PARAM_OCTET_STRING, &foo, sizeof(foo), 0 },
    { NULL, 0, NULL, 0, 0 }
};

/* The same array of OSSL_PARAM, specified with the macros from params.h */
static OSSL_PARAM static_api_params[] = {
    OSSL_PARAM_int("p1", &app_p1),
    OSSL_PARAM_BN("p3", &bignumbin, sizeof(bignumbin)),
    OSSL_PARAM_DEFN("p4", OSSL_PARAM_UTF8_STRING, &app_p4, sizeof(app_p4)),
    OSSL_PARAM_DEFN("p5", OSSL_PARAM_UTF8_STRING, &app_p5, sizeof(app_p5)),
    /* sizeof(app_p6_init), because we know that's what we're using */
    OSSL_PARAM_DEFN("p6", OSSL_PARAM_UTF8_PTR, &app_p6, sizeof(app_p6_init)),
    OSSL_PARAM_DEFN("foo", OSSL_PARAM_OCTET_STRING, &foo, sizeof(foo)),
    OSSL_PARAM_END
};

/*
 * The same array again, but constructed at run-time
 * This exercises the OSSL_PARAM constructor functions
 */
static OSSL_PARAM *construct_api_params(void)
{
    size_t n = 0;
    static OSSL_PARAM params[10];

    params[n++] = OSSL_PARAM_construct_int("p1", &app_p1);
    params[n++] = OSSL_PARAM_construct_BN("p3", bignumbin, sizeof(bignumbin));
    params[n++] = OSSL_PARAM_construct_utf8_string("p4", app_p4,
                                                   sizeof(app_p4));
    params[n++] = OSSL_PARAM_construct_utf8_string("p5", app_p5,
                                                   sizeof(app_p5));
    /* sizeof(app_p6_init), because we know that's what we're using */
    params[n++] = OSSL_PARAM_construct_utf8_ptr("p6", (char **)&app_p6,
                                                sizeof(app_p6_init));
    params[n++] = OSSL_PARAM_construct_octet_string("foo", &foo, sizeof(foo));
    params[n++] = OSSL_PARAM_construct_end();

    return params;
}

struct param_owner_st {
    OSSL_PARAM *static_params;
    OSSL_PARAM *(*constructed_params)(void);
};

static const struct param_owner_st raw_params = {
    static_raw_params, NULL
};

static const struct param_owner_st api_params = {
    static_api_params, construct_api_params
};

/*-
 * TESTING
 * =======
 */

/*
 * Test cases to combine parameters with "provider side" functions
 */
static struct {
    const struct provider_dispatch_st *prov;
    const struct param_owner_st *app;
    const char *desc;
} test_cases[] = {
    /* Tests within specific methods */
    { &provider_raw, &raw_params, "raw provider vs raw params" },
    { &provider_api, &api_params, "api provider vs api params" },

    /* Mixed methods */
    { &provider_raw, &api_params, "raw provider vs api params" },
    { &provider_api, &raw_params, "api provider vs raw params" },
};

/* Generic tester of combinations of "providers" and params */
static int test_case_variant(OSSL_PARAM *params, const struct provider_dispatch_st *prov)
{
    BIGNUM *verify_p3 = NULL;
    void *obj = NULL;
    int errcnt = 0;
    OSSL_PARAM *p;

    /*
     * Initialize
     */
    if (!TEST_ptr(obj = init_object())
        || !TEST_true(BN_hex2bn(&verify_p3, p3_init))) {
        errcnt++;
        goto fin;
    }

    /*
     * Get parameters a first time, just to see that getting works and
     * gets us the values we expect.
     */
    init_app_variables();

    if (!TEST_true(prov->get_params(obj, params))
        || !TEST_int_eq(app_p1, p1_init)        /* "provider" value */
        || !TEST_double_eq(app_p2, app_p2_init) /* Should remain untouched */
        || !TEST_ptr(p = OSSL_PARAM_locate(params, "p3"))
        || !TEST_ptr(BN_native2bn(bignumbin, p->return_size, app_p3))
        || !TEST_BN_eq(app_p3, verify_p3)       /* "provider" value */
        || !TEST_str_eq(app_p4, p4_init)        /* "provider" value */
        || !TEST_ptr(p = OSSL_PARAM_locate(params, "p5"))
        || !TEST_size_t_eq(p->return_size, sizeof(p5_init)) /* "provider" value */
        || !TEST_str_eq(app_p5, p5_init)        /* "provider" value */
        || !TEST_ptr(p = OSSL_PARAM_locate(params, "p6"))
        || !TEST_size_t_eq(p->return_size, sizeof(p6_init)) /* "provider" value */
        || !TEST_str_eq(app_p6, p6_init)        /* "provider" value */
        || !TEST_char_eq(foo[0], app_foo_init)  /* Should remain untouched */
        || !TEST_ptr(p = OSSL_PARAM_locate(params, "foo")))
        errcnt++;

    /*
     * Set parameters, then sneak into the object itself and check
     * that its attributes got set (or ignored) properly.
     */
    init_app_variables();

    if (!TEST_true(prov->set_params(obj, params))) {
        errcnt++;
    } else {
        struct object_st *sneakpeek = obj;

        if (!TEST_int_eq(sneakpeek->p1, app_p1)         /* app value set */
            || !TEST_double_eq(sneakpeek->p2, p2_init)  /* Should remain untouched */
            || !TEST_BN_eq(sneakpeek->p3, app_p3)       /* app value set */
            || !TEST_str_eq(sneakpeek->p4, app_p4)      /* app value set */
            || !TEST_str_eq(sneakpeek->p5, app_p5)      /* app value set */
            || !TEST_str_eq(sneakpeek->p6, app_p6))     /* app value set */
            errcnt++;
    }

    /*
     * Get parameters again, checking that we get different values
     * than earlier where relevant.
     */
    BN_free(verify_p3);
    verify_p3 = NULL;

    if (!TEST_true(BN_hex2bn(&verify_p3, app_p3_init))) {
        errcnt++;
        goto fin;
    }

    if (!TEST_true(prov->get_params(obj, params))
        || !TEST_int_eq(app_p1, app_p1_init)    /* app value */
        || !TEST_double_eq(app_p2, app_p2_init) /* Should remain untouched */
        || !TEST_ptr(p = OSSL_PARAM_locate(params, "p3"))
        || !TEST_ptr(BN_native2bn(bignumbin, p->return_size, app_p3))
        || !TEST_BN_eq(app_p3, verify_p3)       /* app value */
        || !TEST_str_eq(app_p4, app_p4_init)    /* app value */
        || !TEST_ptr(p = OSSL_PARAM_locate(params, "p5"))
        || !TEST_size_t_eq(p->return_size,
                           sizeof(app_p5_init)) /* app value */
        || !TEST_str_eq(app_p5, app_p5_init)    /* app value */
        || !TEST_ptr(p = OSSL_PARAM_locate(params, "p6"))
        || !TEST_size_t_eq(p->return_size,
                           sizeof(app_p6_init)) /* app value */
        || !TEST_str_eq(app_p6, app_p6_init)    /* app value */
        || !TEST_char_eq(foo[0], app_foo_init)  /* Should remain untouched */
        || !TEST_ptr(p = OSSL_PARAM_locate(params, "foo")))
        errcnt++;

 fin:
    BN_free(verify_p3);
    verify_p3 = NULL;
    cleanup_app_variables();
    cleanup_object(obj);

    return errcnt == 0;
}

static int test_case(int i)
{
    TEST_info("Case: %s", test_cases[i].desc);

    return test_case_variant(test_cases[i].app->static_params,
                             test_cases[i].prov)
        && (test_cases[i].app->constructed_params == NULL
            || test_case_variant(test_cases[i].app->constructed_params(),
                                 test_cases[i].prov));
}

int setup_tests(void)
{
    ADD_ALL_TESTS(test_case, OSSL_NELEM(test_cases));
    return 1;
}
