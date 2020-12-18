/*
 * Copyright 1995-2020 The OpenSSL Project Authors. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://www.openssl.org/source/license.html
 */

#ifndef OPENSSL_RAND_H
# define OPENSSL_RAND_H
# pragma once

# include <openssl/macros.h>
# ifndef OPENSSL_NO_DEPRECATED_3_0
#  define HEADER_RAND_H
# endif

# include <stdlib.h>
# include <openssl/types.h>
# include <openssl/e_os2.h>
# include <openssl/randerr.h>
# include <openssl/evp.h>

#ifdef  __cplusplus
extern "C" {
#endif

/*
 * Default security strength (in the sense of [NIST SP 800-90Ar1])
 *
 * NIST SP 800-90Ar1 supports the strength of the DRBG being smaller than that
 * of the cipher by collecting less entropy. The current DRBG implementation
 * does not take RAND_DRBG_STRENGTH into account and sets the strength of the
 * DRBG to that of the cipher.
 */
# define RAND_DRBG_STRENGTH             256

struct rand_meth_st {
    int (*seed) (const void *buf, int num);
    int (*bytes) (unsigned char *buf, int num);
    void (*cleanup) (void);
    int (*add) (const void *buf, int num, double randomness);
    int (*pseudorand) (unsigned char *buf, int num);
    int (*status) (void);
};

int RAND_set_rand_method(const RAND_METHOD *meth);
const RAND_METHOD *RAND_get_rand_method(void);
# ifndef OPENSSL_NO_ENGINE
int RAND_set_rand_engine(ENGINE *engine);
# endif

RAND_METHOD *RAND_OpenSSL(void);

# ifndef OPENSSL_NO_DEPRECATED_1_1_0
#   define RAND_cleanup() while(0) continue
# endif
int RAND_bytes(unsigned char *buf, int num);
int RAND_priv_bytes(unsigned char *buf, int num);

/* Equivalent of RAND_priv_bytes() but additionally taking an OPENSSL_CTX */
int RAND_priv_bytes_ex(OPENSSL_CTX *ctx, unsigned char *buf, int num);

/* Equivalent of RAND_bytes() but additionally taking an OPENSSL_CTX */
int RAND_bytes_ex(OPENSSL_CTX *ctx, unsigned char *buf, int num);

DEPRECATEDIN_1_1_0(int RAND_pseudo_bytes(unsigned char *buf, int num))

EVP_RAND_CTX *RAND_get0_primary(OPENSSL_CTX *ctx);
EVP_RAND_CTX *RAND_get0_public(OPENSSL_CTX *ctx);
EVP_RAND_CTX *RAND_get0_private(OPENSSL_CTX *ctx);

void RAND_seed(const void *buf, int num);
void RAND_keep_random_devices_open(int keep);

# if defined(__ANDROID__) && defined(__NDK_FPABI__)
__NDK_FPABI__   /* __attribute__((pcs("aapcs"))) on ARM */
# endif
void RAND_add(const void *buf, int num, double randomness);
int RAND_load_file(const char *file, long max_bytes);
int RAND_write_file(const char *file);
const char *RAND_file_name(char *file, size_t num);
int RAND_status(void);

# ifndef OPENSSL_NO_EGD
int RAND_query_egd_bytes(const char *path, unsigned char *buf, int bytes);
int RAND_egd(const char *path);
int RAND_egd_bytes(const char *path, int bytes);
# endif

int RAND_poll(void);

# if defined(_WIN32) && (defined(BASETYPES) || defined(_WINDEF_H))
/* application has to include <windows.h> in order to use these */
DEPRECATEDIN_1_1_0(void RAND_screen(void))
DEPRECATEDIN_1_1_0(int RAND_event(UINT, WPARAM, LPARAM))
# endif

#ifdef  __cplusplus
}
#endif

#endif
