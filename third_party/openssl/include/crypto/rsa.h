/*
 * Copyright 2019-2020 The OpenSSL Project Authors. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://www.openssl.org/source/license.html
 */

#ifndef OSSL_INTERNAL_RSA_H
# define OSSL_INTERNAL_RSA_H

# include <openssl/core.h>
# include <openssl/rsa.h>

typedef struct rsa_pss_params_30_st {
    int hash_algorithm_nid;
    struct {
        int algorithm_nid;       /* Currently always NID_mgf1 */
        int hash_algorithm_nid;
    } mask_gen;
    int salt_len;
    int trailer_field;
} RSA_PSS_PARAMS_30;

RSA_PSS_PARAMS_30 *rsa_get0_pss_params_30(RSA *r);
int rsa_pss_params_30_set_defaults(RSA_PSS_PARAMS_30 *rsa_pss_params);
int rsa_pss_params_30_copy(RSA_PSS_PARAMS_30 *to,
                           const RSA_PSS_PARAMS_30 *from);
int rsa_pss_params_30_is_unrestricted(const RSA_PSS_PARAMS_30 *rsa_pss_params);
int rsa_pss_params_30_set_hashalg(RSA_PSS_PARAMS_30 *rsa_pss_params,
                                  int hashalg_nid);
int rsa_pss_params_30_set_maskgenalg(RSA_PSS_PARAMS_30 *rsa_pss_params,
                                     int maskgenalg_nid);
int rsa_pss_params_30_set_maskgenhashalg(RSA_PSS_PARAMS_30 *rsa_pss_params,
                                         int maskgenhashalg_nid);
int rsa_pss_params_30_set_saltlen(RSA_PSS_PARAMS_30 *rsa_pss_params,
                                  int saltlen);
int rsa_pss_params_30_set_trailerfield(RSA_PSS_PARAMS_30 *rsa_pss_params,
                                       int trailerfield);
int rsa_pss_params_30_hashalg(const RSA_PSS_PARAMS_30 *rsa_pss_params);
int rsa_pss_params_30_maskgenalg(const RSA_PSS_PARAMS_30 *rsa_pss_params);
int rsa_pss_params_30_maskgenhashalg(const RSA_PSS_PARAMS_30 *rsa_pss_params);
int rsa_pss_params_30_saltlen(const RSA_PSS_PARAMS_30 *rsa_pss_params);
int rsa_pss_params_30_trailerfield(const RSA_PSS_PARAMS_30 *rsa_pss_params);

const char *rsa_mgf_nid2name(int mgf);
int rsa_oaeppss_md2nid(const EVP_MD *md);
const char *rsa_oaeppss_nid2name(int md);

RSA *rsa_new_with_ctx(OPENSSL_CTX *libctx);
OPENSSL_CTX *rsa_get0_libctx(RSA *r);

int rsa_set0_all_params(RSA *r, const STACK_OF(BIGNUM) *primes,
                        const STACK_OF(BIGNUM) *exps,
                        const STACK_OF(BIGNUM) *coeffs);
int rsa_get0_all_params(RSA *r, STACK_OF(BIGNUM_const) *primes,
                        STACK_OF(BIGNUM_const) *exps,
                        STACK_OF(BIGNUM_const) *coeffs);

int rsa_todata(RSA *rsa, OSSL_PARAM_BLD *bld, OSSL_PARAM params[]);
int rsa_fromdata(RSA *rsa, const OSSL_PARAM params[]);
int rsa_pss_params_30_todata(const RSA_PSS_PARAMS_30 *pss, const char *propq,
                             OSSL_PARAM_BLD *bld, OSSL_PARAM params[]);
int rsa_pss_params_30_fromdata(RSA_PSS_PARAMS_30 *pss_params,
                               const OSSL_PARAM params[], OPENSSL_CTX *libctx);

int rsa_padding_check_PKCS1_type_2_TLS(OPENSSL_CTX *ctx, unsigned char *to,
                                       size_t tlen, const unsigned char *from,
                                       size_t flen, int client_version,
                                       int alt_version);
int rsa_padding_add_PKCS1_OAEP_mgf1_with_libctx(OPENSSL_CTX *libctx,
                                                unsigned char *to, int tlen,
                                                const unsigned char *from,
                                                int flen,
                                                const unsigned char *param,
                                                int plen, const EVP_MD *md,
                                                const EVP_MD *mgf1md);

int rsa_validate_public(const RSA *key);
int rsa_validate_private(const RSA *key);
int rsa_validate_pairwise(const RSA *key);

int int_rsa_verify(int dtype, const unsigned char *m,
                   unsigned int m_len, unsigned char *rm,
                   size_t *prm_len, const unsigned char *sigbuf,
                   size_t siglen, RSA *rsa);

const unsigned char *rsa_digestinfo_encoding(int md_nid, size_t *len);

extern const char *rsa_mp_factor_names[];
extern const char *rsa_mp_exp_names[];
extern const char *rsa_mp_coeff_names[];

# if defined(FIPS_MODULE) && !defined(OPENSSL_NO_ACVP_TESTS)
int rsa_acvp_test_gen_params_new(OSSL_PARAM **dst, const OSSL_PARAM src[]);
void rsa_acvp_test_gen_params_free(OSSL_PARAM *dst);

int rsa_acvp_test_set_params(RSA *r, const OSSL_PARAM params[]);
int rsa_acvp_test_get_params(RSA *r, OSSL_PARAM params[]);
typedef struct rsa_acvp_test_st RSA_ACVP_TEST;
void rsa_acvp_test_free(RSA_ACVP_TEST *t);
# else
# define RSA_ACVP_TEST void
# endif

#endif
