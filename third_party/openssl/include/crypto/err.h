/*
 * Copyright 2016-2018 The OpenSSL Project Authors. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://www.openssl.org/source/license.html
 */

#ifndef OSSL_CRYPTO_ERR_H
# define OSSL_CRYPTO_ERR_H

int err_load_crypto_strings_int(void);
void err_cleanup(void);
int err_shelve_state(void **);
void err_unshelve_state(void *);

#endif
