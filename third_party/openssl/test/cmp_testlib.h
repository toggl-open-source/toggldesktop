/*
 * Copyright 2007-2020 The OpenSSL Project Authors. All Rights Reserved.
 * Copyright Nokia 2007-2019
 * Copyright Siemens AG 2015-2019
 *
 * Licensed under the Apache License 2.0 (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://www.openssl.org/source/license.html
 */

#ifndef OSSL_TEST_CMP_TESTLIB_H
# define OSSL_TEST_CMP_TESTLIB_H

# include <openssl/cmp.h>
# include <openssl/pem.h>
# include <openssl/rand.h>
# include "crypto/x509.h" /* for x509_set0_libctx() and x509_dup_with_libctx() */

# include "../crypto/cmp/cmp_local.h"

# include "testutil.h"

# ifndef OPENSSL_NO_CMP
#  define CMP_TEST_REFVALUE_LENGTH 15 /* arbitrary value */
EVP_PKEY *load_pem_key(const char *file, OPENSSL_CTX *libctx);
X509 *load_pem_cert(const char *file, OPENSSL_CTX *libctx);
X509_REQ *load_csr(const char *file);
OSSL_CMP_MSG *load_pkimsg(const char *file);
int valid_asn1_encoding(const OSSL_CMP_MSG *msg);
int STACK_OF_X509_cmp(const STACK_OF(X509) *sk1, const STACK_OF(X509) *sk2);
int STACK_OF_X509_push1(STACK_OF(X509) *sk, X509 *cert);
int print_to_bio_out(const char *func, const char *file, int line,
                     OSSL_CMP_severity level, const char *msg);
# endif

#endif /* OSSL_TEST_CMP_TESTLIB_H */
