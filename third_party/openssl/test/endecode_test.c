/*
 * Copyright 2020 The OpenSSL Project Authors. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://www.openssl.org/source/license.html
 */

#include <string.h>
#include <openssl/evp.h>
#include <openssl/pem.h>
#include <openssl/rsa.h>
#include <openssl/x509.h>
#include <openssl/core_names.h>
#include <openssl/params.h>
#include <openssl/param_build.h>
#include <openssl/encoder.h>
#include <openssl/decoder.h>

#include "internal/pem.h"        /* For PVK and "blob" PEM headers */
#include "internal/cryptlib.h"   /* ossl_assert */

#include "testutil.h"

#ifndef OPENSSL_NO_EC
static BN_CTX *bnctx = NULL;
static OSSL_PARAM_BLD *bld_prime_nc = NULL;
static OSSL_PARAM_BLD *bld_prime = NULL;
static OSSL_PARAM *ec_explicit_prime_params_nc = NULL;
static OSSL_PARAM *ec_explicit_prime_params_explicit = NULL;

# ifndef OPENSSL_NO_EC2M
static OSSL_PARAM_BLD *bld_tri_nc = NULL;
static OSSL_PARAM_BLD *bld_tri = NULL;
static OSSL_PARAM *ec_explicit_tri_params_nc = NULL;
static OSSL_PARAM *ec_explicit_tri_params_explicit = NULL;
# endif
#endif

static EVP_PKEY *make_template(const char *type, OSSL_PARAM *genparams)
{
    EVP_PKEY *pkey = NULL;
    EVP_PKEY_CTX *ctx = EVP_PKEY_CTX_new_from_name(NULL, type, NULL);

    /*
     * No real need to check the errors other than for the cascade
     * effect.  |pkey| will simply remain NULL if something goes wrong.
     */
    (void)(ctx != NULL
           && EVP_PKEY_paramgen_init(ctx) > 0
           && (genparams == NULL
               || EVP_PKEY_CTX_set_params(ctx, genparams) > 0)
           && EVP_PKEY_gen(ctx, &pkey) > 0);
    EVP_PKEY_CTX_free(ctx);

    return pkey;
}

static EVP_PKEY *make_key(const char *type, EVP_PKEY *template,
                          OSSL_PARAM *genparams)
{
    EVP_PKEY *pkey = NULL;
    EVP_PKEY_CTX *ctx =
        template != NULL
        ? EVP_PKEY_CTX_new(template, NULL)
        : EVP_PKEY_CTX_new_from_name(NULL, type, NULL);

    /*
     * No real need to check the errors other than for the cascade
     * effect.  |pkey| will simply remain NULL if something goes wrong.
     */
    (void)(ctx != NULL
           && EVP_PKEY_keygen_init(ctx) > 0
           && (genparams == NULL
               || EVP_PKEY_CTX_set_params(ctx, genparams) > 0)
           && EVP_PKEY_keygen(ctx, &pkey) > 0);
    EVP_PKEY_CTX_free(ctx);
    return pkey;
}

/* Main test driver */

/*
 * TODO(3.0) For better error output, changed the callbacks to take __FILE__
 * and __LINE__ as first two arguments, and have them use the lower case
 * functions, such as test_strn_eq(), rather than the uppercase macros
 * (TEST_strn2_eq(), for example).
 */

typedef int (encoder)(void **encoded, long *encoded_len,
                      void *object, const char *pass, const char *pcipher,
                      const char *encoder_propq);
typedef int (decoder)(void **object,
                      void *encoded, long encoded_len,
                      const char *pass);
typedef int (tester)(const void *data1, size_t data1_len,
                     const void *data2, size_t data2_len);
typedef int (checker)(const char *type, const void *data, size_t data_len);
typedef void (dumper)(const char *label, const void *data, size_t data_len);

static int test_encode_decode(const char *type, EVP_PKEY *pkey,
                              const char *pass, const char *pcipher,
                              encoder *encode_cb, decoder *decode_cb,
                              tester *test_cb, checker *check_cb,
                              dumper *dump_cb, const char *encoder_propq)
{
    void *encoded = NULL;
    long encoded_len = 0;
    EVP_PKEY *pkey2 = NULL;
    void *encoded2 = NULL;
    long encoded2_len = 0;
    int ok = 0;

    if (!encode_cb(&encoded, &encoded_len, pkey,
                      pass, pcipher, encoder_propq)
        || !check_cb(type, encoded, encoded_len)
        || !decode_cb((void **)&pkey2, encoded, encoded_len,
                           pass)
        || !TEST_int_eq(EVP_PKEY_eq(pkey, pkey2), 1))
        goto end;

    /*
     * Double check the encoding, but only for unprotected keys,
     * as protected keys have a random component, which makes the output
     * differ.
     */
    if ((pass == NULL && pcipher == NULL)
        && (!encode_cb(&encoded2, &encoded2_len, pkey2,
                          pass, pcipher, encoder_propq)
            || !test_cb(encoded, encoded_len,
                        encoded2, encoded2_len)))
        goto end;

    ok = 1;
 end:
    if (!ok) {
        if (encoded != NULL && encoded_len != 0)
            dump_cb("encoded result", encoded, encoded_len);
        if (encoded2 != NULL && encoded2_len != 0)
            dump_cb("re-encoded result", encoded2, encoded2_len);
    }

    OPENSSL_free(encoded);
    OPENSSL_free(encoded2);
    EVP_PKEY_free(pkey2);
    return ok;
}

/* Encoding and desencoding methods */

static int encode_EVP_PKEY_prov(void **encoded, long *encoded_len,
                                void *object,
                                const char *pass, const char *pcipher,
                                const char *encoder_propq)
{
    EVP_PKEY *pkey = object;
    OSSL_ENCODER_CTX *ectx = NULL;
    BIO *mem_ser = NULL;
    BUF_MEM *mem_buf = NULL;
    const unsigned char *upass = (const unsigned char *)pass;
    int ok = 0;

    if (!TEST_ptr(ectx = OSSL_ENCODER_CTX_new_by_EVP_PKEY(pkey, encoder_propq))
        || (pass != NULL
            && !TEST_true(OSSL_ENCODER_CTX_set_passphrase(ectx, upass,
                                                             strlen(pass))))
        || (pcipher != NULL
            && !TEST_true(OSSL_ENCODER_CTX_set_cipher(ectx, pcipher, NULL)))
        || !TEST_ptr(mem_ser = BIO_new(BIO_s_mem()))
        || !TEST_true(OSSL_ENCODER_to_bio(ectx, mem_ser))
        || !TEST_true(BIO_get_mem_ptr(mem_ser, &mem_buf) > 0)
        || !TEST_ptr(*encoded = mem_buf->data)
        || !TEST_long_gt(*encoded_len = mem_buf->length, 0))
        goto end;

    /* Detach the encoded output */
    mem_buf->data = NULL;
    mem_buf->length = 0;
    ok = 1;
 end:
    BIO_free(mem_ser);
    OSSL_ENCODER_CTX_free(ectx);
    return ok;
}

static int decode_EVP_PKEY_prov(void **object,
                                void *encoded, long encoded_len,
                                const char *pass)
{
    EVP_PKEY *pkey = NULL;
    OSSL_DECODER_CTX *dctx = NULL;
    BIO *mem_deser = NULL;
    const unsigned char *upass = (const unsigned char *)pass;
    int ok = 0;

    if (!TEST_ptr(dctx = OSSL_DECODER_CTX_new_by_EVP_PKEY(&pkey, NULL,
                                                               NULL, NULL))
        || (pass != NULL
            && !OSSL_DECODER_CTX_set_passphrase(dctx, upass,
                                                     strlen(pass)))
        || !TEST_ptr(mem_deser = BIO_new_mem_buf(encoded, encoded_len))
        || !TEST_true(OSSL_DECODER_from_bio(dctx, mem_deser)))
        goto end;
    ok = 1;
    *object = pkey;
 end:
    BIO_free(mem_deser);
    OSSL_DECODER_CTX_free(dctx);
    return ok;
}

static int encode_EVP_PKEY_legacy_PEM(void **encoded,
                                      long *encoded_len,
                                      void *object,
                                      const char *pass, const char *pcipher,
                                      ossl_unused const char *encoder_propq)
{
    EVP_PKEY *pkey = object;
    EVP_CIPHER *cipher = NULL;
    BIO *mem_ser = NULL;
    BUF_MEM *mem_buf = NULL;
    const unsigned char *upass = (const unsigned char *)pass;
    size_t passlen = 0;
    int ok = 0;

    if (pcipher != NULL && pass != NULL) {
        passlen = strlen(pass);
        if (!TEST_ptr(cipher = EVP_CIPHER_fetch(NULL, pcipher, NULL)))
            goto end;
    }
    if (!TEST_ptr(mem_ser = BIO_new(BIO_s_mem()))
        || !TEST_true(PEM_write_bio_PrivateKey_traditional(mem_ser, pkey,
                                                           cipher,
                                                           upass, passlen,
                                                           NULL, NULL))
        || !TEST_true(BIO_get_mem_ptr(mem_ser, &mem_buf) > 0)
        || !TEST_ptr(*encoded = mem_buf->data)
        || !TEST_long_gt(*encoded_len = mem_buf->length, 0))
        goto end;

    /* Detach the encoded output */
    mem_buf->data = NULL;
    mem_buf->length = 0;
    ok = 1;
 end:
    BIO_free(mem_ser);
    EVP_CIPHER_free(cipher);
    return ok;
}

#ifndef OPENSSL_NO_DSA
static int encode_EVP_PKEY_MSBLOB(void **encoded, long *encoded_len,
                                  void *object,
                                  ossl_unused const char *pass,
                                  ossl_unused const char *pcipher,
                                  ossl_unused const char *encoder_propq)
{
    EVP_PKEY *pkey = object;
    BIO *mem_ser = NULL;
    BUF_MEM *mem_buf = NULL;
    int ok = 0;

    if (!TEST_ptr(mem_ser = BIO_new(BIO_s_mem()))
        || !TEST_int_ge(i2b_PrivateKey_bio(mem_ser, pkey), 0)
        || !TEST_true(BIO_get_mem_ptr(mem_ser, &mem_buf) > 0)
        || !TEST_ptr(*encoded = mem_buf->data)
        || !TEST_long_gt(*encoded_len = mem_buf->length, 0))
        goto end;

    /* Detach the encoded output */
    mem_buf->data = NULL;
    mem_buf->length = 0;
    ok = 1;
 end:
    BIO_free(mem_ser);
    return ok;
}

static int encode_public_EVP_PKEY_MSBLOB(void **encoded, long *encoded_len,
                                         void *object,
                                         ossl_unused const char *pass,
                                         ossl_unused const char *pcipher,
                                         ossl_unused const char *encoder_propq)
{
    EVP_PKEY *pkey = object;
    BIO *mem_ser = NULL;
    BUF_MEM *mem_buf = NULL;
    int ok = 0;

    if (!TEST_ptr(mem_ser = BIO_new(BIO_s_mem()))
        || !TEST_int_ge(i2b_PublicKey_bio(mem_ser, pkey), 0)
        || !TEST_true(BIO_get_mem_ptr(mem_ser, &mem_buf) > 0)
        || !TEST_ptr(*encoded = mem_buf->data)
        || !TEST_long_gt(*encoded_len = mem_buf->length, 0))
        goto end;

    /* Detach the encoded output */
    mem_buf->data = NULL;
    mem_buf->length = 0;
    ok = 1;
 end:
    BIO_free(mem_ser);
    return ok;
}

# ifndef OPENSSL_NO_RC4
static pem_password_cb pass_pw;
static int pass_pw(char *buf, int size, int rwflag, void *userdata)
{
    OPENSSL_strlcpy(buf, userdata, size);
    return strlen(userdata);
}

static int encode_EVP_PKEY_PVK(void **encoded, long *encoded_len,
                               void *object,
                               const char *pass,
                               ossl_unused const char *pcipher,
                               ossl_unused const char *encoder_propq)
{
    EVP_PKEY *pkey = object;
    BIO *mem_ser = NULL;
    BUF_MEM *mem_buf = NULL;
    int enc = (pass != NULL);
    int ok = 0;

    if (!TEST_ptr(mem_ser = BIO_new(BIO_s_mem()))
        || !TEST_int_ge(i2b_PVK_bio(mem_ser, pkey, enc,
                                    pass_pw, (void *)pass), 0)
        || !TEST_true(BIO_get_mem_ptr(mem_ser, &mem_buf) > 0)
        || !TEST_ptr(*encoded = mem_buf->data)
        || !TEST_long_gt(*encoded_len = mem_buf->length, 0))
        goto end;

    /* Detach the encoded output */
    mem_buf->data = NULL;
    mem_buf->length = 0;
    ok = 1;
 end:
    BIO_free(mem_ser);
    return ok;
}
# endif
#endif

static int test_text(const void *data1, size_t data1_len,
                     const void *data2, size_t data2_len)
{
    return TEST_strn2_eq(data1, data1_len, data2, data2_len);
}

static int test_mem(const void *data1, size_t data1_len,
                    const void *data2, size_t data2_len)
{
    return TEST_mem_eq(data1, data1_len, data2, data2_len);
}

/* Test cases and their dumpers / checkers */

static void dump_der(const char *label, const void *data, size_t data_len)
{
    test_output_memory(label, data, data_len);
}

static void dump_pem(const char *label, const void *data, size_t data_len)
{
    test_output_string(label, data, data_len - 1);
}

static int check_unprotected_PKCS8_DER(const char *type,
                                       const void *data, size_t data_len)
{
    const unsigned char *datap = data;
    PKCS8_PRIV_KEY_INFO *p8inf =
        d2i_PKCS8_PRIV_KEY_INFO(NULL, &datap, data_len);
    int ok = 0;

    if (TEST_ptr(p8inf)) {
        EVP_PKEY *pkey = EVP_PKCS82PKEY(p8inf);

        ok = (TEST_ptr(pkey) && TEST_true(EVP_PKEY_is_a(pkey, type)));
        EVP_PKEY_free(pkey);
    }
    PKCS8_PRIV_KEY_INFO_free(p8inf);
    return ok;
}

static int test_unprotected_via_DER(const char *type, EVP_PKEY *key)
{
    return test_encode_decode(type, key, NULL, NULL,
                              encode_EVP_PKEY_prov, decode_EVP_PKEY_prov,
                              test_mem, check_unprotected_PKCS8_DER, dump_der,
                              OSSL_ENCODER_PrivateKey_TO_DER_PQ);
}

static int check_unprotected_PKCS8_PEM(const char *type,
                                       const void *data, size_t data_len)
{
    static const char pem_header[] = "-----BEGIN " PEM_STRING_PKCS8INF "-----";

    return TEST_strn_eq(data, pem_header, sizeof(pem_header) - 1);
}

static int test_unprotected_via_PEM(const char *type, EVP_PKEY *key)
{
    return test_encode_decode(type, key, NULL, NULL,
                              encode_EVP_PKEY_prov, decode_EVP_PKEY_prov,
                              test_text, check_unprotected_PKCS8_PEM, dump_pem,
                              OSSL_ENCODER_PrivateKey_TO_PEM_PQ);
}

static int check_unprotected_legacy_PEM(const char *type,
                                        const void *data, size_t data_len)
{
    static char pem_header[80];

    return
        TEST_int_gt(BIO_snprintf(pem_header, sizeof(pem_header),
                                 "-----BEGIN %s PRIVATE KEY-----", type), 0)
        && TEST_strn_eq(data, pem_header, strlen(pem_header));
}

static int test_unprotected_via_legacy_PEM(const char *type, EVP_PKEY *key)
{
    return test_encode_decode(type, key, NULL, NULL,
                              encode_EVP_PKEY_legacy_PEM, decode_EVP_PKEY_prov,
                              test_text, check_unprotected_legacy_PEM, dump_pem,
                              NULL);
}

#ifndef OPENSSL_NO_DSA
static int check_MSBLOB(const char *type, const void *data, size_t data_len)
{
    const unsigned char *datap = data;
    EVP_PKEY *pkey = b2i_PrivateKey(&datap, data_len);
    int ok = TEST_ptr(pkey);

    EVP_PKEY_free(pkey);
    return ok;
}

static int test_unprotected_via_MSBLOB(const char *type, EVP_PKEY *key)
{
    return test_encode_decode(type, key, NULL, NULL,
                              encode_EVP_PKEY_MSBLOB, decode_EVP_PKEY_prov,
                              test_mem, check_MSBLOB, dump_der,
                              NULL);
}

# ifndef OPENSSL_NO_RC4
static int check_PVK(const char *type, const void *data, size_t data_len)
{
    const unsigned char *in = data;
    unsigned int saltlen = 0, keylen = 0;
    int ok = ossl_do_PVK_header(&in, data_len, 0, &saltlen, &keylen);

    return ok;
}

static int test_unprotected_via_PVK(const char *type, EVP_PKEY *key)
{
    return test_encode_decode(type, key, NULL, NULL,
                              encode_EVP_PKEY_PVK, decode_EVP_PKEY_prov,
                              test_mem, check_PVK, dump_der,
                              NULL);
}
# endif
#endif

static const char *pass_cipher = "AES-256-CBC";
static const char *pass = "the holy handgrenade of antioch";

static int check_protected_PKCS8_DER(const char *type,
                                     const void *data, size_t data_len)
{
    const unsigned char *datap = data;
    X509_SIG *p8 = d2i_X509_SIG(NULL, &datap, data_len);
    int ok = TEST_ptr(p8);

    X509_SIG_free(p8);
    return ok;
}

static int test_protected_via_DER(const char *type, EVP_PKEY *key)
{
    return test_encode_decode(type, key, pass, pass_cipher,
                              encode_EVP_PKEY_prov, decode_EVP_PKEY_prov,
                              test_mem, check_protected_PKCS8_DER, dump_der,
                              OSSL_ENCODER_PrivateKey_TO_DER_PQ);
}

static int check_protected_PKCS8_PEM(const char *type,
                                     const void *data, size_t data_len)
{
    static const char pem_header[] = "-----BEGIN " PEM_STRING_PKCS8 "-----";

    return TEST_strn_eq(data, pem_header, sizeof(pem_header) - 1);
}

static int test_protected_via_PEM(const char *type, EVP_PKEY *key)
{
    return test_encode_decode(type, key, pass, pass_cipher,
                              encode_EVP_PKEY_prov, decode_EVP_PKEY_prov,
                              test_text, check_protected_PKCS8_PEM, dump_pem,
                              OSSL_ENCODER_PrivateKey_TO_PEM_PQ);
}

static int check_protected_legacy_PEM(const char *type,
                                      const void *data, size_t data_len)
{
    static char pem_header[80];

    return
        TEST_int_gt(BIO_snprintf(pem_header, sizeof(pem_header),
                                 "-----BEGIN %s PRIVATE KEY-----", type), 0)
        && TEST_strn_eq(data, pem_header, strlen(pem_header))
        && TEST_ptr(strstr(data, "\nDEK-Info: "));
}

static int test_protected_via_legacy_PEM(const char *type, EVP_PKEY *key)
{
    return test_encode_decode(type, key, pass, pass_cipher,
                              encode_EVP_PKEY_legacy_PEM, decode_EVP_PKEY_prov,
                              test_text, check_protected_legacy_PEM, dump_pem,
                              NULL);
}

#if !defined(OPENSSL_NO_DSA) && !defined(OPENSSL_NO_RC4)
static int test_protected_via_PVK(const char *type, EVP_PKEY *key)
{
    return test_encode_decode(type, key, pass, NULL,
                              encode_EVP_PKEY_PVK, decode_EVP_PKEY_prov,
                              test_mem, check_PVK, dump_der,
                              NULL);
}
#endif

static int check_public_DER(const char *type, const void *data, size_t data_len)
{
    const unsigned char *datap = data;
    EVP_PKEY *pkey = d2i_PUBKEY(NULL, &datap, data_len);
    int ok = (TEST_ptr(pkey) && TEST_true(EVP_PKEY_is_a(pkey, type)));

    EVP_PKEY_free(pkey);
    return ok;
}

static int test_public_via_DER(const char *type, EVP_PKEY *key)
{
    return test_encode_decode(type, key, NULL, NULL,
                              encode_EVP_PKEY_prov, decode_EVP_PKEY_prov,
                              test_mem, check_public_DER, dump_der,
                              OSSL_ENCODER_PUBKEY_TO_DER_PQ);
}

static int check_public_PEM(const char *type, const void *data, size_t data_len)
{
    static const char pem_header[] = "-----BEGIN " PEM_STRING_PUBLIC "-----";

    return
        TEST_strn_eq(data, pem_header, sizeof(pem_header) - 1);
}

static int test_public_via_PEM(const char *type, EVP_PKEY *key)
{
    return test_encode_decode(type, key, NULL, NULL,
                              encode_EVP_PKEY_prov, decode_EVP_PKEY_prov,
                              test_text, check_public_PEM, dump_pem,
                              OSSL_ENCODER_PUBKEY_TO_PEM_PQ);
}

#ifndef OPENSSL_NO_DSA
static int check_public_MSBLOB(const char *type,
                               const void *data, size_t data_len)
{
    const unsigned char *datap = data;
    EVP_PKEY *pkey = b2i_PublicKey(&datap, data_len);
    int ok = TEST_ptr(pkey);

    EVP_PKEY_free(pkey);
    return ok;
}

static int test_public_via_MSBLOB(const char *type, EVP_PKEY *key)
{
    return test_encode_decode(type, key, NULL, NULL,
                              encode_public_EVP_PKEY_MSBLOB,
                              decode_EVP_PKEY_prov,
                              test_mem, check_public_MSBLOB, dump_der,
                              NULL);
}
#endif

#define KEYS(KEYTYPE)                           \
    static EVP_PKEY *key_##KEYTYPE = NULL
#define MAKE_KEYS(KEYTYPE, KEYTYPEstr, params)                          \
    ok = ok                                                             \
        && TEST_ptr(key_##KEYTYPE = make_key(KEYTYPEstr, NULL, params))
#define FREE_KEYS(KEYTYPE)                                              \
    EVP_PKEY_free(key_##KEYTYPE);                                       \

#define DOMAIN_KEYS(KEYTYPE)                    \
    static EVP_PKEY *template_##KEYTYPE = NULL; \
    static EVP_PKEY *key_##KEYTYPE = NULL
#define MAKE_DOMAIN_KEYS(KEYTYPE, KEYTYPEstr, params)                   \
    ok = ok                                                             \
        && TEST_ptr(template_##KEYTYPE =                                \
                    make_template(KEYTYPEstr, params))                  \
        && TEST_ptr(key_##KEYTYPE =                                     \
                    make_key(KEYTYPEstr, template_##KEYTYPE, NULL))
#define FREE_DOMAIN_KEYS(KEYTYPE)                                       \
    EVP_PKEY_free(template_##KEYTYPE);                                  \
    EVP_PKEY_free(key_##KEYTYPE)

#define IMPLEMENT_TEST_SUITE(KEYTYPE, KEYTYPEstr)                       \
    static int test_unprotected_##KEYTYPE##_via_DER(void)               \
    {                                                                   \
        return test_unprotected_via_DER(KEYTYPEstr, key_##KEYTYPE);     \
    }                                                                   \
    static int test_unprotected_##KEYTYPE##_via_PEM(void)               \
    {                                                                   \
        return test_unprotected_via_PEM(KEYTYPEstr, key_##KEYTYPE);     \
    }                                                                   \
    static int test_protected_##KEYTYPE##_via_DER(void)                 \
    {                                                                   \
        return test_protected_via_DER(KEYTYPEstr, key_##KEYTYPE);       \
    }                                                                   \
    static int test_protected_##KEYTYPE##_via_PEM(void)                 \
    {                                                                   \
        return test_protected_via_PEM(KEYTYPEstr, key_##KEYTYPE);       \
    }                                                                   \
    static int test_public_##KEYTYPE##_via_DER(void)                    \
    {                                                                   \
        return test_public_via_DER(KEYTYPEstr, key_##KEYTYPE);          \
    }                                                                   \
    static int test_public_##KEYTYPE##_via_PEM(void)                    \
    {                                                                   \
        return test_public_via_PEM(KEYTYPEstr, key_##KEYTYPE);          \
    }

#define ADD_TEST_SUITE(KEYTYPE)                                 \
    ADD_TEST(test_unprotected_##KEYTYPE##_via_DER);             \
    ADD_TEST(test_unprotected_##KEYTYPE##_via_PEM);             \
    ADD_TEST(test_protected_##KEYTYPE##_via_DER);               \
    ADD_TEST(test_protected_##KEYTYPE##_via_PEM);               \
    ADD_TEST(test_public_##KEYTYPE##_via_DER);                  \
    ADD_TEST(test_public_##KEYTYPE##_via_PEM)

#define IMPLEMENT_TEST_SUITE_LEGACY(KEYTYPE, KEYTYPEstr)                \
    static int test_unprotected_##KEYTYPE##_via_legacy_PEM(void)        \
    {                                                                   \
        return                                                          \
            test_unprotected_via_legacy_PEM(KEYTYPEstr, key_##KEYTYPE); \
    }                                                                   \
    static int test_protected_##KEYTYPE##_via_legacy_PEM(void)          \
    {                                                                   \
        return                                                          \
            test_protected_via_legacy_PEM(KEYTYPEstr, key_##KEYTYPE);   \
    }

#define ADD_TEST_SUITE_LEGACY(KEYTYPE)                                 \
    ADD_TEST(test_unprotected_##KEYTYPE##_via_legacy_PEM);             \
    ADD_TEST(test_protected_##KEYTYPE##_via_legacy_PEM)

#ifndef OPENSSL_NO_DSA
# define IMPLEMENT_TEST_SUITE_MSBLOB(KEYTYPE, KEYTYPEstr)               \
    static int test_unprotected_##KEYTYPE##_via_MSBLOB(void)            \
    {                                                                   \
        return test_unprotected_via_MSBLOB(KEYTYPEstr, key_##KEYTYPE);  \
    }                                                                   \
    static int test_public_##KEYTYPE##_via_MSBLOB(void)                 \
    {                                                                   \
        return test_public_via_MSBLOB(KEYTYPEstr, key_##KEYTYPE);       \
    }

# define ADD_TEST_SUITE_MSBLOB(KEYTYPE)                         \
    ADD_TEST(test_unprotected_##KEYTYPE##_via_MSBLOB);          \
    ADD_TEST(test_public_##KEYTYPE##_via_MSBLOB)

# ifndef OPENSSL_NO_RC4
#  define IMPLEMENT_TEST_SUITE_PVK(KEYTYPE, KEYTYPEstr)                 \
    static int test_unprotected_##KEYTYPE##_via_PVK(void)               \
    {                                                                   \
        return test_unprotected_via_PVK(KEYTYPEstr, key_##KEYTYPE);     \
    }                                                                   \
    static int test_protected_##KEYTYPE##_via_PVK(void)                 \
    {                                                                   \
        return test_protected_via_PVK(KEYTYPEstr, key_##KEYTYPE);       \
    }

#  define ADD_TEST_SUITE_PVK(KEYTYPE)                           \
    ADD_TEST(test_unprotected_##KEYTYPE##_via_PVK);             \
    ADD_TEST(test_protected_##KEYTYPE##_via_PVK)
# endif
#endif

#ifndef OPENSSL_NO_DH
DOMAIN_KEYS(DH);
IMPLEMENT_TEST_SUITE(DH, "DH")
DOMAIN_KEYS(DHX);
IMPLEMENT_TEST_SUITE(DHX, "X9.42 DH")
/*
 * DH has no support for PEM_write_bio_PrivateKey_traditional(),
 * so no legacy tests.
 */
#endif
#ifndef OPENSSL_NO_DSA
DOMAIN_KEYS(DSA);
IMPLEMENT_TEST_SUITE(DSA, "DSA")
IMPLEMENT_TEST_SUITE_LEGACY(DSA, "DSA")
IMPLEMENT_TEST_SUITE_MSBLOB(DSA, "DSA")
# ifndef OPENSSL_NO_RC4
IMPLEMENT_TEST_SUITE_PVK(DSA, "DSA")
# endif
#endif
#ifndef OPENSSL_NO_EC
DOMAIN_KEYS(EC);
IMPLEMENT_TEST_SUITE(EC, "EC")
IMPLEMENT_TEST_SUITE_LEGACY(EC, "EC")
DOMAIN_KEYS(ECExplicitPrimeNamedCurve);
IMPLEMENT_TEST_SUITE(ECExplicitPrimeNamedCurve, "EC")
IMPLEMENT_TEST_SUITE_LEGACY(ECExplicitPrimeNamedCurve, "EC")
DOMAIN_KEYS(ECExplicitPrime2G);
IMPLEMENT_TEST_SUITE(ECExplicitPrime2G, "EC")
IMPLEMENT_TEST_SUITE_LEGACY(ECExplicitPrime2G, "EC")
# ifndef OPENSSL_NO_EC2M
DOMAIN_KEYS(ECExplicitTriNamedCurve);
IMPLEMENT_TEST_SUITE(ECExplicitTriNamedCurve, "EC")
IMPLEMENT_TEST_SUITE_LEGACY(ECExplicitTriNamedCurve, "EC")
DOMAIN_KEYS(ECExplicitTri2G);
IMPLEMENT_TEST_SUITE(ECExplicitTri2G, "EC")
IMPLEMENT_TEST_SUITE_LEGACY(ECExplicitTri2G, "EC")
# endif
KEYS(ED25519);
IMPLEMENT_TEST_SUITE(ED25519, "ED25519")
KEYS(ED448);
IMPLEMENT_TEST_SUITE(ED448, "ED448")
KEYS(X25519);
IMPLEMENT_TEST_SUITE(X25519, "X25519")
KEYS(X448);
IMPLEMENT_TEST_SUITE(X448, "X448")
/*
 * ED25519, ED448, X25519 and X448 have no support for
 * PEM_write_bio_PrivateKey_traditional(), so no legacy tests.
 */
#endif
KEYS(RSA);
IMPLEMENT_TEST_SUITE(RSA, "RSA")
IMPLEMENT_TEST_SUITE_LEGACY(RSA, "RSA")
KEYS(RSA_PSS);
IMPLEMENT_TEST_SUITE(RSA_PSS, "RSA-PSS")
/*
 * RSA-PSS has no support for PEM_write_bio_PrivateKey_traditional(),
 * so no legacy tests.
 */
#ifndef OPENSSL_NO_DSA
IMPLEMENT_TEST_SUITE_MSBLOB(RSA, "RSA")
# ifndef OPENSSL_NO_RC4
IMPLEMENT_TEST_SUITE_PVK(RSA, "RSA")
# endif
#endif

#ifndef OPENSSL_NO_EC
/* Explicit parameters that match a named curve */
static int do_create_ec_explicit_prime_params(OSSL_PARAM_BLD *bld,
                                              const unsigned char *gen,
                                              size_t gen_len)
{
    BIGNUM *a, *b, *prime, *order;

    /* Curve prime256v1 */
    static const unsigned char prime_data[] = {
        0x00, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00,
        0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff
    };
    static const unsigned char a_data[] = {
        0x00, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00,
        0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xfc
    };
    static const unsigned char b_data[] = {
        0x5a, 0xc6, 0x35, 0xd8, 0xaa, 0x3a, 0x93, 0xe7,
        0xb3, 0xeb, 0xbd, 0x55, 0x76, 0x98, 0x86, 0xbc,
        0x65, 0x1d, 0x06, 0xb0, 0xcc, 0x53, 0xb0, 0xf6,
        0x3b, 0xce, 0x3c, 0x3e, 0x27, 0xd2, 0x60, 0x4b
    };
    static const unsigned char seed[] = {
        0xc4, 0x9d, 0x36, 0x08, 0x86, 0xe7, 0x04, 0x93,
        0x6a, 0x66, 0x78, 0xe1, 0x13, 0x9d, 0x26, 0xb7,
        0x81, 0x9f, 0x7e, 0x90
    };
    static const unsigned char order_data[] = {
        0x00, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00,
        0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xbc, 0xe6, 0xfa, 0xad, 0xa7, 0x17, 0x9e,
        0x84, 0xf3, 0xb9, 0xca, 0xc2, 0xfc, 0x63, 0x25, 0x51
    };
    return TEST_ptr(a = BN_CTX_get(bnctx))
           && TEST_ptr(b = BN_CTX_get(bnctx))
           && TEST_ptr(prime = BN_CTX_get(bnctx))
           && TEST_ptr(order = BN_CTX_get(bnctx))
           && TEST_ptr(BN_bin2bn(prime_data, sizeof(prime_data), prime))
           && TEST_ptr(BN_bin2bn(a_data, sizeof(a_data), a))
           && TEST_ptr(BN_bin2bn(b_data, sizeof(b_data), b))
           && TEST_ptr(BN_bin2bn(order_data, sizeof(order_data), order))
           && TEST_true(OSSL_PARAM_BLD_push_utf8_string(bld,
                            OSSL_PKEY_PARAM_EC_FIELD_TYPE, SN_X9_62_prime_field,
                            0))
           && TEST_true(OSSL_PARAM_BLD_push_BN(bld, OSSL_PKEY_PARAM_EC_P, prime))
           && TEST_true(OSSL_PARAM_BLD_push_BN(bld, OSSL_PKEY_PARAM_EC_A, a))
           && TEST_true(OSSL_PARAM_BLD_push_BN(bld, OSSL_PKEY_PARAM_EC_B, b))
           && TEST_true(OSSL_PARAM_BLD_push_BN(bld,
                            OSSL_PKEY_PARAM_EC_ORDER, order))
           && TEST_true(OSSL_PARAM_BLD_push_octet_string(bld,
                            OSSL_PKEY_PARAM_EC_GENERATOR, gen, gen_len))
           && TEST_true(OSSL_PARAM_BLD_push_octet_string(bld,
                            OSSL_PKEY_PARAM_EC_SEED, seed, sizeof(seed)))
           && TEST_true(OSSL_PARAM_BLD_push_BN(bld, OSSL_PKEY_PARAM_EC_COFACTOR,
                                               BN_value_one()));
}

static int create_ec_explicit_prime_params_namedcurve(OSSL_PARAM_BLD *bld)
{
    static const unsigned char prime256v1_gen[] = {
        0x04,
        0x6b, 0x17, 0xd1, 0xf2, 0xe1, 0x2c, 0x42, 0x47,
        0xf8, 0xbc, 0xe6, 0xe5, 0x63, 0xa4, 0x40, 0xf2,
        0x77, 0x03, 0x7d, 0x81, 0x2d, 0xeb, 0x33, 0xa0,
        0xf4, 0xa1, 0x39, 0x45, 0xd8, 0x98, 0xc2, 0x96,
        0x4f, 0xe3, 0x42, 0xe2, 0xfe, 0x1a, 0x7f, 0x9b,
        0x8e, 0xe7, 0xeb, 0x4a, 0x7c, 0x0f, 0x9e, 0x16,
        0x2b, 0xce, 0x33, 0x57, 0x6b, 0x31, 0x5e, 0xce,
        0xcb, 0xb6, 0x40, 0x68, 0x37, 0xbf, 0x51, 0xf5
    };
    return do_create_ec_explicit_prime_params(bld, prime256v1_gen,
                                              sizeof(prime256v1_gen));
}

static int create_ec_explicit_prime_params(OSSL_PARAM_BLD *bld)
{
    /* 2G */
    static const unsigned char prime256v1_gen2[] = {
        0x04,
        0xe4, 0x97, 0x08, 0xbe, 0x7d, 0xfa, 0xa2, 0x9a,
        0xa3, 0x12, 0x6f, 0xe4, 0xe7, 0xd0, 0x25, 0xe3,
        0x4a, 0xc1, 0x03, 0x15, 0x8c, 0xd9, 0x33, 0xc6,
        0x97, 0x42, 0xf5, 0xdc, 0x97, 0xb9, 0xd7, 0x31,
        0xe9, 0x7d, 0x74, 0x3d, 0x67, 0x6a, 0x3b, 0x21,
        0x08, 0x9c, 0x31, 0x73, 0xf8, 0xc1, 0x27, 0xc9,
        0xd2, 0xa0, 0xa0, 0x83, 0x66, 0xe0, 0xc9, 0xda,
        0xa8, 0xc6, 0x56, 0x2b, 0x94, 0xb1, 0xae, 0x55
    };
    return do_create_ec_explicit_prime_params(bld, prime256v1_gen2,
                                              sizeof(prime256v1_gen2));
}

# ifndef OPENSSL_NO_EC2M
static int do_create_ec_explicit_trinomial_params(OSSL_PARAM_BLD *bld,
                                                  const unsigned char *gen,
                                                  size_t gen_len)
{
    BIGNUM *a, *b, *poly, *order, *cofactor;
    /* sect233k1 characteristic-two-field tpBasis */
    static const unsigned char poly_data[] = {
        0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x01,
    };
    static const unsigned char a_data[] = {
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00
    };
    static const unsigned char b_data[] = {
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x01
    };
    static const unsigned char order_data[] = {
        0x00, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x06, 0x9D, 0x5B, 0xB9, 0x15, 0xBC, 0xD4, 0x6E, 0xFB,
        0x1A, 0xD5, 0xF1, 0x73, 0xAB, 0xDF
    };
    static const unsigned char cofactor_data[]= {
        0x4
    };
    return TEST_ptr(a = BN_CTX_get(bnctx))
           && TEST_ptr(b = BN_CTX_get(bnctx))
           && TEST_ptr(poly = BN_CTX_get(bnctx))
           && TEST_ptr(order = BN_CTX_get(bnctx))
           && TEST_ptr(cofactor = BN_CTX_get(bnctx))
           && TEST_ptr(BN_bin2bn(poly_data, sizeof(poly_data), poly))
           && TEST_ptr(BN_bin2bn(a_data, sizeof(a_data), a))
           && TEST_ptr(BN_bin2bn(b_data, sizeof(b_data), b))
           && TEST_ptr(BN_bin2bn(order_data, sizeof(order_data), order))
           && TEST_ptr(BN_bin2bn(cofactor_data, sizeof(cofactor_data), cofactor))
           && TEST_true(OSSL_PARAM_BLD_push_utf8_string(bld,
                            OSSL_PKEY_PARAM_EC_FIELD_TYPE,
                            SN_X9_62_characteristic_two_field, 0))
           && TEST_true(OSSL_PARAM_BLD_push_BN(bld, OSSL_PKEY_PARAM_EC_P, poly))
           && TEST_true(OSSL_PARAM_BLD_push_BN(bld, OSSL_PKEY_PARAM_EC_A, a))
           && TEST_true(OSSL_PARAM_BLD_push_BN(bld, OSSL_PKEY_PARAM_EC_B, b))
           && TEST_true(OSSL_PARAM_BLD_push_BN(bld,
                            OSSL_PKEY_PARAM_EC_ORDER, order))
           && TEST_true(OSSL_PARAM_BLD_push_octet_string(bld,
                            OSSL_PKEY_PARAM_EC_GENERATOR, gen, gen_len))
           && TEST_true(OSSL_PARAM_BLD_push_BN(bld, OSSL_PKEY_PARAM_EC_COFACTOR,
                                               cofactor));
}

static int create_ec_explicit_trinomial_params_namedcurve(OSSL_PARAM_BLD *bld)
{
    static const unsigned char gen[] = {
        0x04,
        0x01, 0x72, 0x32, 0xBA, 0x85, 0x3A, 0x7E, 0x73, 0x1A, 0xF1, 0x29, 0xF2,
        0x2F, 0xF4, 0x14, 0x95, 0x63, 0xA4, 0x19, 0xC2, 0x6B, 0xF5, 0x0A, 0x4C,
        0x9D, 0x6E, 0xEF, 0xAD, 0x61, 0x26,
        0x01, 0xDB, 0x53, 0x7D, 0xEC, 0xE8, 0x19, 0xB7, 0xF7, 0x0F, 0x55, 0x5A,
        0x67, 0xC4, 0x27, 0xA8, 0xCD, 0x9B, 0xF1, 0x8A, 0xEB, 0x9B, 0x56, 0xE0,
        0xC1, 0x10, 0x56, 0xFA, 0xE6, 0xA3
    };
    return do_create_ec_explicit_trinomial_params(bld, gen, sizeof(gen));
}

static int create_ec_explicit_trinomial_params(OSSL_PARAM_BLD *bld)
{
    static const unsigned char gen2[] = {
        0x04,
        0x00, 0xd7, 0xba, 0xd0, 0x26, 0x6c, 0x31, 0x6a, 0x78, 0x76, 0x01, 0xd1,
        0x32, 0x4b, 0x8f, 0x30, 0x29, 0x2d, 0x78, 0x30, 0xca, 0x43, 0xaa, 0xf0,
        0xa2, 0x5a, 0xd4, 0x0f, 0xb3, 0xf4,
        0x00, 0x85, 0x4b, 0x1b, 0x8d, 0x50, 0x10, 0xa5, 0x1c, 0x80, 0xf7, 0x86,
        0x40, 0x62, 0x4c, 0x87, 0xd1, 0x26, 0x7a, 0x9c, 0x5c, 0xe9, 0x82, 0x29,
        0xd1, 0x67, 0x70, 0x41, 0xea, 0xcb
    };
    return do_create_ec_explicit_trinomial_params(bld, gen2, sizeof(gen2));
}
# endif /* OPENSSL_NO_EC2M */
#endif /* OPENSSL_NO_EC */

int setup_tests(void)
{
    int ok = 1;

#ifndef OPENSSL_NO_DSA
    static size_t qbits = 160;  /* PVK only tolerates 160 Q bits */
    static size_t pbits = 1024; /* With 160 Q bits, we MUST use 1024 P bits */
    OSSL_PARAM DSA_params[] = {
        OSSL_PARAM_size_t("pbits", &pbits),
        OSSL_PARAM_size_t("qbits", &qbits),
        OSSL_PARAM_END
    };
#endif

#ifndef OPENSSL_NO_EC
    static char groupname[] = "prime256v1";
    OSSL_PARAM EC_params[] = {
        OSSL_PARAM_utf8_string("group", groupname, sizeof(groupname) - 1),
        OSSL_PARAM_END
    };
#endif

    /* 7 is the default magic number */
    static unsigned int rsapss_min_saltlen = 7;
    OSSL_PARAM RSA_PSS_params[] = {
        OSSL_PARAM_uint("saltlen", &rsapss_min_saltlen),
        OSSL_PARAM_END
    };

#ifndef OPENSSL_NO_EC
    if (!TEST_ptr(bnctx = BN_CTX_new_ex(NULL))
        || !TEST_ptr(bld_prime_nc = OSSL_PARAM_BLD_new())
        || !TEST_ptr(bld_prime = OSSL_PARAM_BLD_new())
        || !create_ec_explicit_prime_params_namedcurve(bld_prime_nc)
        || !create_ec_explicit_prime_params(bld_prime)
        || !TEST_ptr(ec_explicit_prime_params_nc = OSSL_PARAM_BLD_to_param(bld_prime_nc))
        || !TEST_ptr(ec_explicit_prime_params_explicit = OSSL_PARAM_BLD_to_param(bld_prime))
# ifndef OPENSSL_NO_EC2M
        || !TEST_ptr(bld_tri_nc = OSSL_PARAM_BLD_new())
        || !TEST_ptr(bld_tri = OSSL_PARAM_BLD_new())
        || !create_ec_explicit_trinomial_params_namedcurve(bld_tri_nc)
        || !create_ec_explicit_trinomial_params(bld_tri)
        || !TEST_ptr(ec_explicit_tri_params_nc = OSSL_PARAM_BLD_to_param(bld_tri_nc))
        || !TEST_ptr(ec_explicit_tri_params_explicit = OSSL_PARAM_BLD_to_param(bld_tri))
# endif
        )
        return 0;
#endif

    TEST_info("Generating keys...");

#ifndef OPENSSL_NO_DH
    MAKE_DOMAIN_KEYS(DH, "DH", NULL);
    MAKE_DOMAIN_KEYS(DHX, "X9.42 DH", NULL);
#endif
#ifndef OPENSSL_NO_DSA
    MAKE_DOMAIN_KEYS(DSA, "DSA", DSA_params);
#endif
#ifndef OPENSSL_NO_EC
    MAKE_DOMAIN_KEYS(EC, "EC", EC_params);
    MAKE_DOMAIN_KEYS(ECExplicitPrimeNamedCurve, "EC", ec_explicit_prime_params_nc);
    MAKE_DOMAIN_KEYS(ECExplicitPrime2G, "EC", ec_explicit_prime_params_explicit);
# ifndef OPENSSL_NO_EC2M
    MAKE_DOMAIN_KEYS(ECExplicitTriNamedCurve, "EC", ec_explicit_tri_params_nc);
    MAKE_DOMAIN_KEYS(ECExplicitTri2G, "EC", ec_explicit_tri_params_explicit);
# endif
    MAKE_KEYS(ED25519, "ED25519", NULL);
    MAKE_KEYS(ED448, "ED448", NULL);
    MAKE_KEYS(X25519, "X25519", NULL);
    MAKE_KEYS(X448, "X448", NULL);
#endif
    MAKE_KEYS(RSA, "RSA", NULL);
    MAKE_KEYS(RSA_PSS, "RSA-PSS", RSA_PSS_params);
    TEST_info("Generating key... done");

    if (ok) {
#ifndef OPENSSL_NO_DH
        ADD_TEST_SUITE(DH);
        ADD_TEST_SUITE(DHX);
        /*
         * DH has no support for PEM_write_bio_PrivateKey_traditional(),
         * so no legacy tests.
         */
#endif
#ifndef OPENSSL_NO_DSA
        ADD_TEST_SUITE(DSA);
        ADD_TEST_SUITE_LEGACY(DSA);
        ADD_TEST_SUITE_MSBLOB(DSA);
# ifndef OPENSSL_NO_RC4
        ADD_TEST_SUITE_PVK(DSA);
# endif
#endif
#ifndef OPENSSL_NO_EC
        ADD_TEST_SUITE(EC);
        ADD_TEST_SUITE_LEGACY(EC);
        ADD_TEST_SUITE(ECExplicitPrimeNamedCurve);
        ADD_TEST_SUITE_LEGACY(ECExplicitPrimeNamedCurve);
        ADD_TEST_SUITE(ECExplicitPrime2G);
        ADD_TEST_SUITE_LEGACY(ECExplicitPrime2G);
# ifndef OPENSSL_NO_EC2M
        ADD_TEST_SUITE(ECExplicitTriNamedCurve);
        ADD_TEST_SUITE_LEGACY(ECExplicitTriNamedCurve);
        ADD_TEST_SUITE(ECExplicitTri2G);
        ADD_TEST_SUITE_LEGACY(ECExplicitTri2G);
# endif
        ADD_TEST_SUITE(ED25519);
        ADD_TEST_SUITE(ED448);
        ADD_TEST_SUITE(X25519);
        ADD_TEST_SUITE(X448);
        /*
         * ED25519, ED448, X25519 and X448 have no support for
         * PEM_write_bio_PrivateKey_traditional(), so no legacy tests.
         */
#endif
        ADD_TEST_SUITE(RSA);
        ADD_TEST_SUITE_LEGACY(RSA);
        ADD_TEST_SUITE(RSA_PSS);
        /*
         * RSA-PSS has no support for PEM_write_bio_PrivateKey_traditional(),
         * so no legacy tests.
         */
#ifndef OPENSSL_NO_DSA
        ADD_TEST_SUITE_MSBLOB(RSA);
# ifndef OPENSSL_NO_RC4
        ADD_TEST_SUITE_PVK(RSA);
# endif
#endif
    }

    return 1;
}

void cleanup_tests(void)
{
#ifndef OPENSSL_NO_EC
    OSSL_PARAM_BLD_free_params(ec_explicit_prime_params_nc);
    OSSL_PARAM_BLD_free_params(ec_explicit_prime_params_explicit);
    OSSL_PARAM_BLD_free(bld_prime_nc);
    OSSL_PARAM_BLD_free(bld_prime);
# ifndef OPENSSL_NO_EC2M
    OSSL_PARAM_BLD_free_params(ec_explicit_tri_params_nc);
    OSSL_PARAM_BLD_free_params(ec_explicit_tri_params_explicit);
    OSSL_PARAM_BLD_free(bld_tri_nc);
    OSSL_PARAM_BLD_free(bld_tri);
# endif
    BN_CTX_free(bnctx);
#endif /* OPENSSL_NO_EC */

#ifndef OPENSSL_NO_DH
    FREE_DOMAIN_KEYS(DH);
    FREE_DOMAIN_KEYS(DHX);
#endif
#ifndef OPENSSL_NO_DSA
    FREE_DOMAIN_KEYS(DSA);
#endif
#ifndef OPENSSL_NO_EC
    FREE_DOMAIN_KEYS(EC);
    FREE_DOMAIN_KEYS(ECExplicitPrimeNamedCurve);
    FREE_DOMAIN_KEYS(ECExplicitPrime2G);
# ifndef OPENSSL_NO_EC2M
    FREE_DOMAIN_KEYS(ECExplicitTriNamedCurve);
    FREE_DOMAIN_KEYS(ECExplicitTri2G);
# endif
    FREE_KEYS(ED25519);
    FREE_KEYS(ED448);
    FREE_KEYS(X25519);
    FREE_KEYS(X448);
#endif
    FREE_KEYS(RSA);
    FREE_KEYS(RSA_PSS);
}
