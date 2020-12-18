/*
 * Copyright 2019-2020 The OpenSSL Project Authors. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://www.openssl.org/source/license.html
 */

#ifndef OSSL_INTERNAL_FFC_H
# define OSSL_INTERNAL_FFC_H

# include <openssl/core.h>
# include <openssl/bn.h>
# include <openssl/evp.h>
# include <openssl/dh.h> /* Uses Error codes from DH */
# include <openssl/params.h>
# include <openssl/param_build.h>
# include "internal/sizes.h"

/* Default value for gindex when canonical generation of g is not used */
# define FFC_UNVERIFIABLE_GINDEX -1

/* The different types of FFC keys */
# define FFC_PARAM_TYPE_DSA  0
# define FFC_PARAM_TYPE_DH   1

/*
 * The mode used by functions that share code for both generation and
 * verification. See ffc_params_FIPS186_4_gen_verify().
 */
#define FFC_PARAM_MODE_VERIFY   0
#define FFC_PARAM_MODE_GENERATE 1

/* Return codes for generation and validation of FFC parameters */
#define FFC_PARAM_RET_STATUS_FAILED         0
#define FFC_PARAM_RET_STATUS_SUCCESS        1
/* Returned if validating and g is only partially verifiable */
#define FFC_PARAM_RET_STATUS_UNVERIFIABLE_G 2

/* Validation flags */
# define FFC_PARAM_FLAG_VALIDATE_PQ    0x01
# define FFC_PARAM_FLAG_VALIDATE_G     0x02
# define FFC_PARAM_FLAG_VALIDATE_ALL                                           \
    (FFC_PARAM_FLAG_VALIDATE_PQ | FFC_PARAM_FLAG_VALIDATE_G)
#define FFC_PARAM_FLAG_VALIDATE_LEGACY 0x04

/*
 * NB: These values must align with the equivalently named macros in
 * openssl/dh.h. We cannot use those macros here in case DH has been disabled.
 */
# define FFC_CHECK_P_NOT_PRIME                0x00001
# define FFC_CHECK_P_NOT_SAFE_PRIME           0x00002
# define FFC_CHECK_UNKNOWN_GENERATOR          0x00004
# define FFC_CHECK_NOT_SUITABLE_GENERATOR     0x00008
# define FFC_CHECK_Q_NOT_PRIME                0x00010
# define FFC_CHECK_INVALID_Q_VALUE            0x00020
# define FFC_CHECK_INVALID_J_VALUE            0x00040

# define FFC_CHECK_BAD_LN_PAIR                0x00080
# define FFC_CHECK_INVALID_SEED_SIZE          0x00100
# define FFC_CHECK_MISSING_SEED_OR_COUNTER    0x00200
# define FFC_CHECK_INVALID_G                  0x00400
# define FFC_CHECK_INVALID_PQ                 0x00800
# define FFC_CHECK_INVALID_COUNTER            0x01000
# define FFC_CHECK_P_MISMATCH                 0x02000
# define FFC_CHECK_Q_MISMATCH                 0x04000
# define FFC_CHECK_G_MISMATCH                 0x08000
# define FFC_CHECK_COUNTER_MISMATCH           0x10000

/* Validation Return codes */
# define FFC_ERROR_PUBKEY_TOO_SMALL       0x01
# define FFC_ERROR_PUBKEY_TOO_LARGE       0x02
# define FFC_ERROR_PUBKEY_INVALID         0x04
# define FFC_ERROR_NOT_SUITABLE_GENERATOR 0x08
# define FFC_ERROR_PRIVKEY_TOO_SMALL      0x10
# define FFC_ERROR_PRIVKEY_TOO_LARGE      0x20

/*
 * Finite field cryptography (FFC) domain parameters are used by DH and DSA.
 * Refer to FIPS186_4 Appendix A & B.
 */
typedef struct ffc_params_st {
    /* Primes */
    BIGNUM *p;
    BIGNUM *q;
    /* Generator */
    BIGNUM *g;
    /* DH X9.42 Optional Subgroup factor j >= 2 where p = j * q + 1 */
    BIGNUM *j;

    /* Required for FIPS186_4 validation of p, q and optionally canonical g */
    unsigned char *seed;
    /* If this value is zero the hash size is used as the seed length */
    size_t seedlen;
    /* Required for FIPS186_4 validation of p and q */
    int pcounter;
    int nid; /* The identity of a named group */

    /*
     * Required for FIPS186_4 generation & validation of canonical g.
     * It uses unverifiable g if this value is -1.
     */
    int gindex;
    int h; /* loop counter for unverifiable g */

    unsigned int flags; /* See FFC_PARAM_FLAG_VALIDATE_ALL */
    /*
     * The digest to use for generation or validation. If this value is NULL,
     * then the digest is chosen using the value of N.
     */
    const char *mdname;
    const char *mdprops;
} FFC_PARAMS;

void ffc_params_init(FFC_PARAMS *params);
void ffc_params_cleanup(FFC_PARAMS *params);
void ffc_params_set0_pqg(FFC_PARAMS *params, BIGNUM *p, BIGNUM *q, BIGNUM *g);
void ffc_params_get0_pqg(const FFC_PARAMS *params, const BIGNUM **p,
                         const BIGNUM **q, const BIGNUM **g);
void ffc_params_set0_j(FFC_PARAMS *d, BIGNUM *j);
int ffc_params_set_seed(FFC_PARAMS *params,
                        const unsigned char *seed, size_t seedlen);
void ffc_params_set_gindex(FFC_PARAMS *params, int index);
void ffc_params_set_pcounter(FFC_PARAMS *params, int index);
void ffc_params_set_h(FFC_PARAMS *params, int index);
void ffc_params_set_flags(FFC_PARAMS *params, unsigned int flags);
void ffc_params_enable_flags(FFC_PARAMS *params, unsigned int flags, int enable);
int ffc_set_digest(FFC_PARAMS *params, const char *alg, const char *props);

int ffc_params_set_validate_params(FFC_PARAMS *params,
                                   const unsigned char *seed, size_t seedlen,
                                   int counter);
void ffc_params_get_validate_params(const FFC_PARAMS *params,
                                    unsigned char **seed, size_t *seedlen,
                                    int *pcounter);

int ffc_params_copy(FFC_PARAMS *dst, const FFC_PARAMS *src);
int ffc_params_cmp(const FFC_PARAMS *a, const FFC_PARAMS *b, int ignore_q);

#ifndef FIPS_MODULE
int ffc_params_print(BIO *bp, const FFC_PARAMS *ffc, int indent);
#endif /* FIPS_MODULE */


int ffc_params_FIPS186_4_generate(OPENSSL_CTX *libctx, FFC_PARAMS *params,
                                  int type, size_t L, size_t N,
                                  int *res, BN_GENCB *cb);
int ffc_params_FIPS186_2_generate(OPENSSL_CTX *libctx, FFC_PARAMS *params,
                                  int type, size_t L, size_t N,
                                  int *res, BN_GENCB *cb);

int ffc_params_FIPS186_4_gen_verify(OPENSSL_CTX *libctx, FFC_PARAMS *params,
                                    int mode, int type, size_t L, size_t N,
                                    int *res, BN_GENCB *cb);
int ffc_params_FIPS186_2_gen_verify(OPENSSL_CTX *libctx, FFC_PARAMS *params,
                                    int mode, int type, size_t L, size_t N,
                                    int *res, BN_GENCB *cb);

int ffc_params_simple_validate(OPENSSL_CTX *libctx, FFC_PARAMS *params, int type);
int ffc_params_FIPS186_4_validate(OPENSSL_CTX *libctx, const FFC_PARAMS *params,
                                  int type, int *res, BN_GENCB *cb);
int ffc_params_FIPS186_2_validate(OPENSSL_CTX *libctx, const FFC_PARAMS *params,
                                  int type, int *res, BN_GENCB *cb);

int ffc_generate_private_key(BN_CTX *ctx, const FFC_PARAMS *params,
                             int N, int s, BIGNUM *priv);

int ffc_params_validate_unverifiable_g(BN_CTX *ctx, BN_MONT_CTX *mont,
                                       const BIGNUM *p, const BIGNUM *q,
                                       const BIGNUM *g, BIGNUM *tmp, int *ret);

int ffc_validate_public_key(const FFC_PARAMS *params, const BIGNUM *pub_key,
                            int *ret);
int ffc_validate_public_key_partial(const FFC_PARAMS *params,
                                    const BIGNUM *pub_key, int *ret);
int ffc_validate_private_key(const BIGNUM *upper, const BIGNUM *priv_key,
                             int *ret);

int ffc_params_todata(const FFC_PARAMS *ffc, OSSL_PARAM_BLD *tmpl,
                      OSSL_PARAM params[]);
int ffc_params_fromdata(FFC_PARAMS *ffc, const OSSL_PARAM params[]);
int ffc_set_group_pqg(FFC_PARAMS *ffc, const char *group_name);
int ffc_named_group_to_uid(const char *name);
const char *ffc_named_group_from_uid(int nid);
int ffc_set_group_pqg(FFC_PARAMS *ffc, const char *group_name);
const char *ffc_params_flags_to_name(int flags);
int ffc_params_flags_from_name(const char *name);

#endif /* OSSL_INTERNAL_FFC_H */
