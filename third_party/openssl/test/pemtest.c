/*
 * Copyright 2017-2018 The OpenSSL Project Authors. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://www.openssl.org/source/license.html
 */

#include <string.h>
#include <openssl/bio.h>
#include <openssl/pem.h>

#include "testutil.h"
#include "internal/nelem.h"

typedef struct {
    const char *raw;
    const char *encoded;
} TESTDATA;

static TESTDATA b64_pem_data[] = {
    { "hello world",
      "aGVsbG8gd29ybGQ=" },
    { "a very ooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooong input",
      "YSB2ZXJ5IG9vb29vb29vb29vb29vb29vb29vb29vb29vb29vb29vb29vb29vb29vb29vb29vb29vb29vb29vb29vb29vb29vb29vb29vb29vb29vb29vb29vb29vb29vb29uZyBpbnB1dA==" }
};

static const char *pemtype = "PEMTESTDATA";

static int test_b64(int idx)
{
    BIO *b = BIO_new(BIO_s_mem());
    char *name = NULL, *header = NULL;
    unsigned char *data = NULL;
    long len;
    int ret = 0;
    const char *raw = b64_pem_data[idx].raw;
    const char *encoded = b64_pem_data[idx].encoded;

    if (!TEST_ptr(b)
        || !TEST_true(BIO_printf(b, "-----BEGIN %s-----\n", pemtype))
        || !TEST_true(BIO_printf(b, "%s\n", encoded))
        || !TEST_true(BIO_printf(b, "-----END %s-----\n", pemtype))
        || !TEST_true(PEM_read_bio_ex(b, &name, &header, &data, &len,
                                      PEM_FLAG_ONLY_B64)))
        goto err;
    if (!TEST_int_eq(memcmp(pemtype, name, strlen(pemtype)), 0)
        || !TEST_int_eq(len, strlen(raw))
        || !TEST_int_eq(memcmp(data, raw, strlen(raw)), 0))
        goto err;
    ret = 1;
 err:
    BIO_free(b);
    OPENSSL_free(name);
    OPENSSL_free(header);
    OPENSSL_free(data);
    return ret;
}

static int test_invalid(void)
{
    BIO *b = BIO_new(BIO_s_mem());
    char *name = NULL, *header = NULL;
    unsigned char *data = NULL;
    long len;
    const char *encoded = b64_pem_data[0].encoded;

    if (!TEST_ptr(b)
        || !TEST_true(BIO_printf(b, "-----BEGIN %s-----\n", pemtype))
        || !TEST_true(BIO_printf(b, "%c%s\n", '\t', encoded))
        || !TEST_true(BIO_printf(b, "-----END %s-----\n", pemtype))
        /* Expected to fail due to non-base64 character */
        || TEST_true(PEM_read_bio_ex(b, &name, &header, &data, &len,
                                     PEM_FLAG_ONLY_B64))) {
        BIO_free(b);
        return 0;
    }
    BIO_free(b);
    OPENSSL_free(name);
    OPENSSL_free(header);
    OPENSSL_free(data);
    return 1;
}

int setup_tests(void)
{
    ADD_ALL_TESTS(test_b64, OSSL_NELEM(b64_pem_data));
    ADD_TEST(test_invalid);
    return 1;
}
