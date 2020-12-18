/*
 * Copyright 2019-2020 The OpenSSL Project Authors. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://www.openssl.org/source/license.html
 */

/*
 * DES low level APIs are deprecated for public use, but still ok for internal
 * use.
 */
#include "internal/deprecated.h"

#include "prov/ciphercommon.h"
#include "cipher_tdes.h"
#include <openssl/rand.h>
#include "prov/implementations.h"
#include "prov/providercommonerr.h"

/*
 * TODO(3.0) - ECB mode does not use an IV - but existing test code is setting
 * an IV. Fixing this could potentially make applications break.
 */
/* tdes_ede3_ecb_functions */
IMPLEMENT_tdes_cipher(ede3, EDE3, ecb, ECB, TDES_FLAGS, 64*3, 64, 64, block);
/* tdes_ede3_cbc_functions */
IMPLEMENT_tdes_cipher(ede3, EDE3, cbc, CBC, TDES_FLAGS, 64*3, 64, 64, block);
