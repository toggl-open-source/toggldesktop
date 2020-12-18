/*
 * Copyright 2016-2020 The OpenSSL Project Authors. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://www.openssl.org/source/license.html
 */

#ifndef OSSL_CRYPTO_STORE_H
# define OSSL_CRYPTO_STORE_H

# include <openssl/bio.h>
# include <openssl/store.h>
# include <openssl/ui.h>

void ossl_store_cleanup_int(void);

#endif
