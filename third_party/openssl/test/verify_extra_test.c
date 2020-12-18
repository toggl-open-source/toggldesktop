/*
 * Copyright 2015-2020 The OpenSSL Project Authors. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://www.openssl.org/source/license.html
 */

#include <stdio.h>
#include <string.h>
#include <openssl/crypto.h>
#include <openssl/bio.h>
#include <openssl/x509.h>
#include <openssl/pem.h>
#include <openssl/err.h>
#include "testutil.h"

static const char *root_f;
static const char *roots_f;
static const char *untrusted_f;
static const char *bad_f;
static const char *req_f;

static X509 *load_cert_from_file(const char *filename)
{
    X509 *cert = NULL;
    BIO *bio;

    bio = BIO_new_file(filename, "r");
    if (bio != NULL)
        cert = PEM_read_bio_X509(bio, NULL, 0, NULL);
    BIO_free(bio);
    return cert;
}

static STACK_OF(X509) *load_certs_from_file(const char *filename)
{
    STACK_OF(X509) *certs;
    BIO *bio;
    X509 *x;

    bio = BIO_new_file(filename, "r");

    if (bio == NULL) {
        return NULL;
    }

    certs = sk_X509_new_null();
    if (certs == NULL) {
        BIO_free(bio);
        return NULL;
    }

    ERR_set_mark();
    do {
        x = PEM_read_bio_X509(bio, NULL, 0, NULL);
        if (x != NULL && !sk_X509_push(certs, x)) {
            sk_X509_pop_free(certs, X509_free);
            BIO_free(bio);
            return NULL;
        } else if (x == NULL) {
            /*
             * We probably just ran out of certs, so ignore any errors
             * generated
             */
            ERR_pop_to_mark();
        }
    } while (x != NULL);

    BIO_free(bio);

    return certs;
}

/*
 * Test for CVE-2015-1793 (Alternate Chains Certificate Forgery)
 *
 * Chain is as follows:
 *
 * rootCA (self-signed)
 *   |
 * interCA
 *   |
 * subinterCA       subinterCA (self-signed)
 *   |                   |
 * leaf ------------------
 *   |
 * bad
 *
 * rootCA, interCA, subinterCA, subinterCA (ss) all have CA=TRUE
 * leaf and bad have CA=FALSE
 *
 * subinterCA and subinterCA (ss) have the same subject name and keys
 *
 * interCA (but not rootCA) and subinterCA (ss) are in the trusted store
 * (roots.pem)
 * leaf and subinterCA are in the untrusted list (untrusted.pem)
 * bad is the certificate being verified (bad.pem)
 *
 * Versions vulnerable to CVE-2015-1793 will fail to detect that leaf has
 * CA=FALSE, and will therefore incorrectly verify bad
 *
 */
static int test_alt_chains_cert_forgery(void)
{
    int ret = 0;
    int i;
    X509 *x = NULL;
    STACK_OF(X509) *untrusted = NULL;
    X509_STORE_CTX *sctx = NULL;
    X509_STORE *store = NULL;
    X509_LOOKUP *lookup = NULL;

    store = X509_STORE_new();
    if (store == NULL)
        goto err;

    lookup = X509_STORE_add_lookup(store, X509_LOOKUP_file());
    if (lookup == NULL)
        goto err;
    if (!X509_LOOKUP_load_file(lookup, roots_f, X509_FILETYPE_PEM))
        goto err;

    untrusted = load_certs_from_file(untrusted_f);

    if ((x = load_cert_from_file(bad_f)) == NULL)
        goto err;

    sctx = X509_STORE_CTX_new();
    if (sctx == NULL)
        goto err;

    if (!X509_STORE_CTX_init(sctx, store, x, untrusted))
        goto err;

    i = X509_verify_cert(sctx);

    if (i == 0 && X509_STORE_CTX_get_error(sctx) == X509_V_ERR_INVALID_CA) {
        /* This is the result we were expecting: Test passed */
        ret = 1;
    }
 err:
    X509_STORE_CTX_free(sctx);
    X509_free(x);
    sk_X509_pop_free(untrusted, X509_free);
    X509_STORE_free(store);
    return ret;
}

static int test_store_ctx(void)
{
    X509_STORE_CTX *sctx = NULL;
    X509 *x = NULL;
    int testresult = 0, ret;

    x = load_cert_from_file(bad_f);
    if (x == NULL)
        goto err;

    sctx = X509_STORE_CTX_new();
    if (sctx == NULL)
        goto err;

    if (!X509_STORE_CTX_init(sctx, NULL, x, NULL))
        goto err;

    /* Verifying a cert where we have no trusted certs should fail */
    ret = X509_verify_cert(sctx);

    if (ret == 0) {
        /* This is the result we were expecting: Test passed */
        testresult = 1;
    }

 err:
    X509_STORE_CTX_free(sctx);
    X509_free(x);
    return testresult;
}

OPT_TEST_DECLARE_USAGE("roots.pem untrusted.pem bad.pem\n")

static int test_distinguishing_id(void)
{
    X509 *x = NULL;
    int ret = 0;
    ASN1_OCTET_STRING *v = NULL, *v2 = NULL;
    char *distid = "this is an ID";

    x = load_cert_from_file(bad_f);
    if (x == NULL)
        goto err;

    v = ASN1_OCTET_STRING_new();
    if (v == NULL)
        goto err;

    if (!ASN1_OCTET_STRING_set(v, (unsigned char *)distid,
                               (int)strlen(distid))) {
        ASN1_OCTET_STRING_free(v);
        goto err;
    }

    X509_set0_distinguishing_id(x, v);

    v2 = X509_get0_distinguishing_id(x);
    if (!TEST_ptr(v2)
            || !TEST_int_eq(ASN1_OCTET_STRING_cmp(v, v2), 0))
        goto err;

    ret = 1;
 err:
    X509_free(x);
    return ret;
}

static int test_req_distinguishing_id(void)
{
    X509_REQ *x = NULL;
    BIO *bio = NULL;
    int ret = 0;
    ASN1_OCTET_STRING *v = NULL, *v2 = NULL;
    char *distid = "this is an ID";

    bio = BIO_new_file(req_f, "r");
    if (bio == NULL)
        goto err;

    x = PEM_read_bio_X509_REQ(bio, NULL, 0, NULL);
    if (x == NULL)
        goto err;

    v = ASN1_OCTET_STRING_new();
    if (v == NULL)
        goto err;

    if (!ASN1_OCTET_STRING_set(v, (unsigned char *)distid,
                               (int)strlen(distid))) {
        ASN1_OCTET_STRING_free(v);
        goto err;
    }

    X509_REQ_set0_distinguishing_id(x, v);

    v2 = X509_REQ_get0_distinguishing_id(x);
    if (!TEST_ptr(v2)
            || !TEST_int_eq(ASN1_OCTET_STRING_cmp(v, v2), 0))
        goto err;

    ret = 1;
 err:
    X509_REQ_free(x);
    BIO_free(bio);
    return ret;
}

static int test_self_signed(const char *filename, int expected)
{
    X509 *cert;
    int ret;

    cert = load_cert_from_file(filename); /* may result in NULL */
    ret = TEST_int_eq(X509_self_signed(cert, 1), expected);
    X509_free(cert);
    return ret;
}

static int test_self_signed_good(void)
{
    return test_self_signed(root_f, 1);
}

static int test_self_signed_bad(void)
{
    return test_self_signed(bad_f, 0);
}

static int test_self_signed_error(void)
{
    return test_self_signed("nonexistent file name", -1);
}

int setup_tests(void)
{
    if (!test_skip_common_options()) {
        TEST_error("Error parsing test options\n");
        return 0;
    }

    if (!TEST_ptr(root_f = test_get_argument(0))
            || !TEST_ptr(roots_f = test_get_argument(1))
            || !TEST_ptr(untrusted_f = test_get_argument(2))
            || !TEST_ptr(bad_f = test_get_argument(3))
            || !TEST_ptr(req_f = test_get_argument(4)))
        return 0;

    ADD_TEST(test_alt_chains_cert_forgery);
    ADD_TEST(test_store_ctx);
    ADD_TEST(test_distinguishing_id);
    ADD_TEST(test_req_distinguishing_id);
    ADD_TEST(test_self_signed_good);
    ADD_TEST(test_self_signed_bad);
    ADD_TEST(test_self_signed_error);
    return 1;
}
