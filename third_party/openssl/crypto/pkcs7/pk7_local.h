/*
 * Copyright 2020 The OpenSSL Project Authors. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://www.openssl.org/source/license.html
 */

#include "crypto/pkcs7.h"

const PKCS7_CTX *pkcs7_get0_ctx(const PKCS7 *p7);
OPENSSL_CTX *pkcs7_ctx_get0_libctx(const PKCS7_CTX *ctx);
const char *pkcs7_ctx_get0_propq(const PKCS7_CTX *ctx);
