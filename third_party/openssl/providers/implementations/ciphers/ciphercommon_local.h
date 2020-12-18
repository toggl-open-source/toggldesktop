/*
 * Copyright 2019-2020 The OpenSSL Project Authors. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://www.openssl.org/source/license.html
 */

#include "prov/ciphercommon.h"

void padblock(unsigned char *buf, size_t *buflen, size_t blocksize);
int unpadblock(unsigned char *buf, size_t *buflen, size_t blocksize);
int tlsunpadblock(OPENSSL_CTX *libctx, unsigned int tlsversion,
                  unsigned char *buf, size_t *buflen, size_t blocksize,
                  unsigned char **mac, int *alloced, size_t macsize, int aead);
