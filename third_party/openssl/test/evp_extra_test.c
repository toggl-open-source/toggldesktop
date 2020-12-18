/*
 * Copyright 2015-2020 The OpenSSL Project Authors. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://www.openssl.org/source/license.html
 */

/* We need to use some deprecated APIs */
#define OPENSSL_SUPPRESS_DEPRECATED

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openssl/bio.h>
#include <openssl/conf.h>
#include <openssl/crypto.h>
#include <openssl/err.h>
#include <openssl/evp.h>
#include <openssl/rsa.h>
#include <openssl/x509.h>
#include <openssl/pem.h>
#include <openssl/kdf.h>
#include <openssl/provider.h>
#include <openssl/core_names.h>
#include <openssl/params.h>
#include <openssl/dsa.h>
#include <openssl/dh.h>
#include <openssl/aes.h>
#include "testutil.h"
#include "internal/nelem.h"
#include "internal/sizes.h"
#include "crypto/evp.h"
#include "../e_os.h" /* strcasecmp */

#ifndef OPENSSL_NO_SM2
/*
 * TODO(3.0) remove when provider SM2 keymgmt is implemented and
 * EVP_PKEY_set_alias_type() works with provider-native keys.
 */
# define TMP_SM2_HACK
#endif

static OPENSSL_CTX *testctx = NULL;

/*
 * kExampleRSAKeyDER is an RSA private key in ASN.1, DER format. Of course, you
 * should never use this key anywhere but in an example.
 */
static const unsigned char kExampleRSAKeyDER[] = {
    0x30, 0x82, 0x02, 0x5c, 0x02, 0x01, 0x00, 0x02, 0x81, 0x81, 0x00, 0xf8,
    0xb8, 0x6c, 0x83, 0xb4, 0xbc, 0xd9, 0xa8, 0x57, 0xc0, 0xa5, 0xb4, 0x59,
    0x76, 0x8c, 0x54, 0x1d, 0x79, 0xeb, 0x22, 0x52, 0x04, 0x7e, 0xd3, 0x37,
    0xeb, 0x41, 0xfd, 0x83, 0xf9, 0xf0, 0xa6, 0x85, 0x15, 0x34, 0x75, 0x71,
    0x5a, 0x84, 0xa8, 0x3c, 0xd2, 0xef, 0x5a, 0x4e, 0xd3, 0xde, 0x97, 0x8a,
    0xdd, 0xff, 0xbb, 0xcf, 0x0a, 0xaa, 0x86, 0x92, 0xbe, 0xb8, 0x50, 0xe4,
    0xcd, 0x6f, 0x80, 0x33, 0x30, 0x76, 0x13, 0x8f, 0xca, 0x7b, 0xdc, 0xec,
    0x5a, 0xca, 0x63, 0xc7, 0x03, 0x25, 0xef, 0xa8, 0x8a, 0x83, 0x58, 0x76,
    0x20, 0xfa, 0x16, 0x77, 0xd7, 0x79, 0x92, 0x63, 0x01, 0x48, 0x1a, 0xd8,
    0x7b, 0x67, 0xf1, 0x52, 0x55, 0x49, 0x4e, 0xd6, 0x6e, 0x4a, 0x5c, 0xd7,
    0x7a, 0x37, 0x36, 0x0c, 0xde, 0xdd, 0x8f, 0x44, 0xe8, 0xc2, 0xa7, 0x2c,
    0x2b, 0xb5, 0xaf, 0x64, 0x4b, 0x61, 0x07, 0x02, 0x03, 0x01, 0x00, 0x01,
    0x02, 0x81, 0x80, 0x74, 0x88, 0x64, 0x3f, 0x69, 0x45, 0x3a, 0x6d, 0xc7,
    0x7f, 0xb9, 0xa3, 0xc0, 0x6e, 0xec, 0xdc, 0xd4, 0x5a, 0xb5, 0x32, 0x85,
    0x5f, 0x19, 0xd4, 0xf8, 0xd4, 0x3f, 0x3c, 0xfa, 0xc2, 0xf6, 0x5f, 0xee,
    0xe6, 0xba, 0x87, 0x74, 0x2e, 0xc7, 0x0c, 0xd4, 0x42, 0xb8, 0x66, 0x85,
    0x9c, 0x7b, 0x24, 0x61, 0xaa, 0x16, 0x11, 0xf6, 0xb5, 0xb6, 0xa4, 0x0a,
    0xc9, 0x55, 0x2e, 0x81, 0xa5, 0x47, 0x61, 0xcb, 0x25, 0x8f, 0xc2, 0x15,
    0x7b, 0x0e, 0x7c, 0x36, 0x9f, 0x3a, 0xda, 0x58, 0x86, 0x1c, 0x5b, 0x83,
    0x79, 0xe6, 0x2b, 0xcc, 0xe6, 0xfa, 0x2c, 0x61, 0xf2, 0x78, 0x80, 0x1b,
    0xe2, 0xf3, 0x9d, 0x39, 0x2b, 0x65, 0x57, 0x91, 0x3d, 0x71, 0x99, 0x73,
    0xa5, 0xc2, 0x79, 0x20, 0x8c, 0x07, 0x4f, 0xe5, 0xb4, 0x60, 0x1f, 0x99,
    0xa2, 0xb1, 0x4f, 0x0c, 0xef, 0xbc, 0x59, 0x53, 0x00, 0x7d, 0xb1, 0x02,
    0x41, 0x00, 0xfc, 0x7e, 0x23, 0x65, 0x70, 0xf8, 0xce, 0xd3, 0x40, 0x41,
    0x80, 0x6a, 0x1d, 0x01, 0xd6, 0x01, 0xff, 0xb6, 0x1b, 0x3d, 0x3d, 0x59,
    0x09, 0x33, 0x79, 0xc0, 0x4f, 0xde, 0x96, 0x27, 0x4b, 0x18, 0xc6, 0xd9,
    0x78, 0xf1, 0xf4, 0x35, 0x46, 0xe9, 0x7c, 0x42, 0x7a, 0x5d, 0x9f, 0xef,
    0x54, 0xb8, 0xf7, 0x9f, 0xc4, 0x33, 0x6c, 0xf3, 0x8c, 0x32, 0x46, 0x87,
    0x67, 0x30, 0x7b, 0xa7, 0xac, 0xe3, 0x02, 0x41, 0x00, 0xfc, 0x2c, 0xdf,
    0x0c, 0x0d, 0x88, 0xf5, 0xb1, 0x92, 0xa8, 0x93, 0x47, 0x63, 0x55, 0xf5,
    0xca, 0x58, 0x43, 0xba, 0x1c, 0xe5, 0x9e, 0xb6, 0x95, 0x05, 0xcd, 0xb5,
    0x82, 0xdf, 0xeb, 0x04, 0x53, 0x9d, 0xbd, 0xc2, 0x38, 0x16, 0xb3, 0x62,
    0xdd, 0xa1, 0x46, 0xdb, 0x6d, 0x97, 0x93, 0x9f, 0x8a, 0xc3, 0x9b, 0x64,
    0x7e, 0x42, 0xe3, 0x32, 0x57, 0x19, 0x1b, 0xd5, 0x6e, 0x85, 0xfa, 0xb8,
    0x8d, 0x02, 0x41, 0x00, 0xbc, 0x3d, 0xde, 0x6d, 0xd6, 0x97, 0xe8, 0xba,
    0x9e, 0x81, 0x37, 0x17, 0xe5, 0xa0, 0x64, 0xc9, 0x00, 0xb7, 0xe7, 0xfe,
    0xf4, 0x29, 0xd9, 0x2e, 0x43, 0x6b, 0x19, 0x20, 0xbd, 0x99, 0x75, 0xe7,
    0x76, 0xf8, 0xd3, 0xae, 0xaf, 0x7e, 0xb8, 0xeb, 0x81, 0xf4, 0x9d, 0xfe,
    0x07, 0x2b, 0x0b, 0x63, 0x0b, 0x5a, 0x55, 0x90, 0x71, 0x7d, 0xf1, 0xdb,
    0xd9, 0xb1, 0x41, 0x41, 0x68, 0x2f, 0x4e, 0x39, 0x02, 0x40, 0x5a, 0x34,
    0x66, 0xd8, 0xf5, 0xe2, 0x7f, 0x18, 0xb5, 0x00, 0x6e, 0x26, 0x84, 0x27,
    0x14, 0x93, 0xfb, 0xfc, 0xc6, 0x0f, 0x5e, 0x27, 0xe6, 0xe1, 0xe9, 0xc0,
    0x8a, 0xe4, 0x34, 0xda, 0xe9, 0xa2, 0x4b, 0x73, 0xbc, 0x8c, 0xb9, 0xba,
    0x13, 0x6c, 0x7a, 0x2b, 0x51, 0x84, 0xa3, 0x4a, 0xe0, 0x30, 0x10, 0x06,
    0x7e, 0xed, 0x17, 0x5a, 0x14, 0x00, 0xc9, 0xef, 0x85, 0xea, 0x52, 0x2c,
    0xbc, 0x65, 0x02, 0x40, 0x51, 0xe3, 0xf2, 0x83, 0x19, 0x9b, 0xc4, 0x1e,
    0x2f, 0x50, 0x3d, 0xdf, 0x5a, 0xa2, 0x18, 0xca, 0x5f, 0x2e, 0x49, 0xaf,
    0x6f, 0xcc, 0xfa, 0x65, 0x77, 0x94, 0xb5, 0xa1, 0x0a, 0xa9, 0xd1, 0x8a,
    0x39, 0x37, 0xf4, 0x0b, 0xa0, 0xd7, 0x82, 0x27, 0x5e, 0xae, 0x17, 0x17,
    0xa1, 0x1e, 0x54, 0x34, 0xbf, 0x6e, 0xc4, 0x8e, 0x99, 0x5d, 0x08, 0xf1,
    0x2d, 0x86, 0x9d, 0xa5, 0x20, 0x1b, 0xe5, 0xdf,
};

/*
* kExampleDSAKeyDER is a DSA private key in ASN.1, DER format. Of course, you
 * should never use this key anywhere but in an example.
 */
#ifndef OPENSSL_NO_DSA
static const unsigned char kExampleDSAKeyDER[] = {
    0x30, 0x82, 0x01, 0xba, 0x02, 0x01, 0x00, 0x02, 0x81, 0x81, 0x00, 0x9a,
    0x05, 0x6d, 0x33, 0xcd, 0x5d, 0x78, 0xa1, 0xbb, 0xcb, 0x7d, 0x5b, 0x8d,
    0xb4, 0xcc, 0xbf, 0x03, 0x99, 0x64, 0xde, 0x38, 0x78, 0x06, 0x15, 0x2f,
    0x86, 0x26, 0x77, 0xf3, 0xb1, 0x85, 0x00, 0xed, 0xfc, 0x28, 0x3a, 0x42,
    0x4d, 0xab, 0xab, 0xdf, 0xbc, 0x9c, 0x16, 0xd0, 0x22, 0x50, 0xd1, 0x38,
    0xdd, 0x3f, 0x64, 0x05, 0x9e, 0x68, 0x7a, 0x1e, 0xf1, 0x56, 0xbf, 0x1e,
    0x2c, 0xc5, 0x97, 0x2a, 0xfe, 0x7a, 0x22, 0xdc, 0x6c, 0x68, 0xb8, 0x2e,
    0x06, 0xdb, 0x41, 0xca, 0x98, 0xd8, 0x54, 0xc7, 0x64, 0x48, 0x24, 0x04,
    0x20, 0xbc, 0x59, 0xe3, 0x6b, 0xea, 0x7e, 0xfc, 0x7e, 0xc5, 0x4e, 0xd4,
    0xd8, 0x3a, 0xed, 0xcd, 0x5d, 0x99, 0xb8, 0x5c, 0xa2, 0x8b, 0xbb, 0x0b,
    0xac, 0xe6, 0x8e, 0x25, 0x56, 0x22, 0x3a, 0x2d, 0x3a, 0x56, 0x41, 0x14,
    0x1f, 0x1c, 0x8f, 0x53, 0x46, 0x13, 0x85, 0x02, 0x15, 0x00, 0x98, 0x7e,
    0x92, 0x81, 0x88, 0xc7, 0x3f, 0x70, 0x49, 0x54, 0xf6, 0x76, 0xb4, 0xa3,
    0x9e, 0x1d, 0x45, 0x98, 0x32, 0x7f, 0x02, 0x81, 0x80, 0x69, 0x4d, 0xef,
    0x55, 0xff, 0x4d, 0x59, 0x2c, 0x01, 0xfa, 0x6a, 0x38, 0xe0, 0x70, 0x9f,
    0x9e, 0x66, 0x8e, 0x3e, 0x8c, 0x52, 0x22, 0x9d, 0x15, 0x7e, 0x3c, 0xef,
    0x4c, 0x7a, 0x61, 0x26, 0xe0, 0x2b, 0x81, 0x3f, 0xeb, 0xaf, 0x35, 0x38,
    0x8d, 0xfe, 0xed, 0x46, 0xff, 0x5f, 0x03, 0x9b, 0x81, 0x92, 0xe7, 0x6f,
    0x76, 0x4f, 0x1d, 0xd9, 0xbb, 0x89, 0xc9, 0x3e, 0xd9, 0x0b, 0xf9, 0xf4,
    0x78, 0x11, 0x59, 0xc0, 0x1d, 0xcd, 0x0e, 0xa1, 0x6f, 0x15, 0xf1, 0x4d,
    0xc1, 0xc9, 0x22, 0xed, 0x8d, 0xad, 0x67, 0xc5, 0x4b, 0x95, 0x93, 0x86,
    0xa6, 0xaf, 0x8a, 0xee, 0x06, 0x89, 0x2f, 0x37, 0x7e, 0x64, 0xaa, 0xf6,
    0xe7, 0xb1, 0x5a, 0x0a, 0x93, 0x95, 0x5d, 0x3e, 0x53, 0x9a, 0xde, 0x8a,
    0xc2, 0x95, 0x45, 0x81, 0xbe, 0x5c, 0x2f, 0xc2, 0xb2, 0x92, 0x58, 0x19,
    0x72, 0x80, 0xe9, 0x79, 0xa1, 0x02, 0x81, 0x80, 0x07, 0xd7, 0x62, 0xff,
    0xdf, 0x1a, 0x3f, 0xed, 0x32, 0xd4, 0xd4, 0x88, 0x7b, 0x2c, 0x63, 0x7f,
    0x97, 0xdc, 0x44, 0xd4, 0x84, 0xa2, 0xdd, 0x17, 0x16, 0x85, 0x13, 0xe0,
    0xac, 0x51, 0x8d, 0x29, 0x1b, 0x75, 0x9a, 0xe4, 0xe3, 0x8a, 0x92, 0x69,
    0x09, 0x03, 0xc5, 0x68, 0xae, 0x5e, 0x94, 0xfe, 0xc9, 0x92, 0x6c, 0x07,
    0xb4, 0x1e, 0x64, 0x62, 0x87, 0xc6, 0xa4, 0xfd, 0x0d, 0x5f, 0xe5, 0xf9,
    0x1b, 0x4f, 0x85, 0x5f, 0xae, 0xf3, 0x11, 0xe5, 0x18, 0xd4, 0x4d, 0x79,
    0x9f, 0xc4, 0x79, 0x26, 0x04, 0x27, 0xf0, 0x0b, 0xee, 0x2b, 0x86, 0x9f,
    0x86, 0x61, 0xe6, 0x51, 0xce, 0x04, 0x9b, 0x5d, 0x6b, 0x34, 0x43, 0x8c,
    0x85, 0x3c, 0xf1, 0x51, 0x9b, 0x08, 0x23, 0x1b, 0xf5, 0x7e, 0x33, 0x12,
    0xea, 0xab, 0x1f, 0xb7, 0x2d, 0xe2, 0x5f, 0xe6, 0x97, 0x99, 0xb5, 0x45,
    0x16, 0x5b, 0xc3, 0x41, 0x02, 0x14, 0x61, 0xbf, 0x51, 0x60, 0xcf, 0xc8,
    0xf1, 0x8c, 0x82, 0x97, 0xf2, 0xf4, 0x19, 0xba, 0x2b, 0xf3, 0x16, 0xbe,
    0x40, 0x48
};
#endif

/*
 * kExampleBadRSAKeyDER is an RSA private key in ASN.1, DER format. The private
 * components are not correct.
 */
static const unsigned char kExampleBadRSAKeyDER[] = {
    0x30, 0x82, 0x04, 0x27, 0x02, 0x01, 0x00, 0x02, 0x82, 0x01, 0x01, 0x00,
    0xa6, 0x1a, 0x1e, 0x6e, 0x7b, 0xee, 0xc6, 0x89, 0x66, 0xe7, 0x93, 0xef,
    0x54, 0x12, 0x68, 0xea, 0xbf, 0x86, 0x2f, 0xdd, 0xd2, 0x79, 0xb8, 0xa9,
    0x6e, 0x03, 0xc2, 0xa3, 0xb9, 0xa3, 0xe1, 0x4b, 0x2a, 0xb3, 0xf8, 0xb4,
    0xcd, 0xea, 0xbe, 0x24, 0xa6, 0x57, 0x5b, 0x83, 0x1f, 0x0f, 0xf2, 0xd3,
    0xb7, 0xac, 0x7e, 0xd6, 0x8e, 0x6e, 0x1e, 0xbf, 0xb8, 0x73, 0x8c, 0x05,
    0x56, 0xe6, 0x35, 0x1f, 0xe9, 0x04, 0x0b, 0x09, 0x86, 0x7d, 0xf1, 0x26,
    0x08, 0x99, 0xad, 0x7b, 0xc8, 0x4d, 0x94, 0xb0, 0x0b, 0x8b, 0x38, 0xa0,
    0x5c, 0x62, 0xa0, 0xab, 0xd3, 0x8f, 0xd4, 0x09, 0x60, 0x72, 0x1e, 0x33,
    0x50, 0x80, 0x6e, 0x22, 0xa6, 0x77, 0x57, 0x6b, 0x9a, 0x33, 0x21, 0x66,
    0x87, 0x6e, 0x21, 0x7b, 0xc7, 0x24, 0x0e, 0xd8, 0x13, 0xdf, 0x83, 0xde,
    0xcd, 0x40, 0x58, 0x1d, 0x84, 0x86, 0xeb, 0xb8, 0x12, 0x4e, 0xd2, 0xfa,
    0x80, 0x1f, 0xe4, 0xe7, 0x96, 0x29, 0xb8, 0xcc, 0xce, 0x66, 0x6d, 0x53,
    0xca, 0xb9, 0x5a, 0xd7, 0xf6, 0x84, 0x6c, 0x2d, 0x9a, 0x1a, 0x14, 0x1c,
    0x4e, 0x93, 0x39, 0xba, 0x74, 0xed, 0xed, 0x87, 0x87, 0x5e, 0x48, 0x75,
    0x36, 0xf0, 0xbc, 0x34, 0xfb, 0x29, 0xf9, 0x9f, 0x96, 0x5b, 0x0b, 0xa7,
    0x54, 0x30, 0x51, 0x29, 0x18, 0x5b, 0x7d, 0xac, 0x0f, 0xd6, 0x5f, 0x7c,
    0xf8, 0x98, 0x8c, 0xd8, 0x86, 0x62, 0xb3, 0xdc, 0xff, 0x0f, 0xff, 0x7a,
    0xaf, 0x5c, 0x4c, 0x61, 0x49, 0x2e, 0xc8, 0x95, 0x86, 0xc4, 0x0e, 0x87,
    0xfc, 0x1d, 0xcf, 0x8b, 0x7c, 0x61, 0xf6, 0xd8, 0xd0, 0x69, 0xf6, 0xcd,
    0x8a, 0x8c, 0xf6, 0x62, 0xa2, 0x56, 0xa9, 0xe3, 0xd1, 0xcf, 0x4d, 0xa0,
    0xf6, 0x2d, 0x20, 0x0a, 0x04, 0xb7, 0xa2, 0xf7, 0xb5, 0x99, 0x47, 0x18,
    0x56, 0x85, 0x87, 0xc7, 0x02, 0x03, 0x01, 0x00, 0x01, 0x02, 0x82, 0x01,
    0x01, 0x00, 0x99, 0x41, 0x38, 0x1a, 0xd0, 0x96, 0x7a, 0xf0, 0x83, 0xd5,
    0xdf, 0x94, 0xce, 0x89, 0x3d, 0xec, 0x7a, 0x52, 0x21, 0x10, 0x16, 0x06,
    0xe0, 0xee, 0xd2, 0xe6, 0xfd, 0x4b, 0x7b, 0x19, 0x4d, 0xe1, 0xc0, 0xc0,
    0xd5, 0x14, 0x5d, 0x79, 0xdd, 0x7e, 0x8b, 0x4b, 0xc6, 0xcf, 0xb0, 0x75,
    0x52, 0xa3, 0x2d, 0xb1, 0x26, 0x46, 0x68, 0x9c, 0x0a, 0x1a, 0xf2, 0xe1,
    0x09, 0xac, 0x53, 0x85, 0x8c, 0x36, 0xa9, 0x14, 0x65, 0xea, 0xa0, 0x00,
    0xcb, 0xe3, 0x3f, 0xc4, 0x2b, 0x61, 0x2e, 0x6b, 0x06, 0x69, 0x77, 0xfd,
    0x38, 0x7e, 0x1d, 0x3f, 0x92, 0xe7, 0x77, 0x08, 0x19, 0xa7, 0x9d, 0x29,
    0x2d, 0xdc, 0x42, 0xc6, 0x7c, 0xd7, 0xd3, 0xa8, 0x01, 0x2c, 0xf2, 0xd5,
    0x82, 0x57, 0xcb, 0x55, 0x3d, 0xe7, 0xaa, 0xd2, 0x06, 0x30, 0x30, 0x05,
    0xe6, 0xf2, 0x47, 0x86, 0xba, 0xc6, 0x61, 0x64, 0xeb, 0x4f, 0x2a, 0x5e,
    0x07, 0x29, 0xe0, 0x96, 0xb2, 0x43, 0xff, 0x5f, 0x1a, 0x54, 0x16, 0xcf,
    0xb5, 0x56, 0x5c, 0xa0, 0x9b, 0x0c, 0xfd, 0xb3, 0xd2, 0xe3, 0x79, 0x1d,
    0x21, 0xe2, 0xd6, 0x13, 0xc4, 0x74, 0xa6, 0xf5, 0x8e, 0x8e, 0x81, 0xbb,
    0xb4, 0xad, 0x8a, 0xf0, 0x93, 0x0a, 0xd8, 0x0a, 0x42, 0x36, 0xbc, 0xe5,
    0x26, 0x2a, 0x0d, 0x5d, 0x57, 0x13, 0xc5, 0x4e, 0x2f, 0x12, 0x0e, 0xef,
    0xa7, 0x81, 0x1e, 0xc3, 0xa5, 0xdb, 0xc9, 0x24, 0xeb, 0x1a, 0xa1, 0xf9,
    0xf6, 0xa1, 0x78, 0x98, 0x93, 0x77, 0x42, 0x45, 0x03, 0xe2, 0xc9, 0xa2,
    0xfe, 0x2d, 0x77, 0xc8, 0xc6, 0xac, 0x9b, 0x98, 0x89, 0x6d, 0x9a, 0xe7,
    0x61, 0x63, 0xb7, 0xf2, 0xec, 0xd6, 0xb1, 0xa1, 0x6e, 0x0a, 0x1a, 0xff,
    0xfd, 0x43, 0x28, 0xc3, 0x0c, 0xdc, 0xf2, 0x47, 0x4f, 0x27, 0xaa, 0x99,
    0x04, 0x8e, 0xac, 0xe8, 0x7c, 0x01, 0x02, 0x04, 0x12, 0x34, 0x56, 0x78,
    0x02, 0x81, 0x81, 0x00, 0xca, 0x69, 0xe5, 0xbb, 0x3a, 0x90, 0x82, 0xcb,
    0x82, 0x50, 0x2f, 0x29, 0xe2, 0x76, 0x6a, 0x57, 0x55, 0x45, 0x4e, 0x35,
    0x18, 0x61, 0xe0, 0x12, 0x70, 0xc0, 0xab, 0xc7, 0x80, 0xa2, 0xd4, 0x46,
    0x34, 0x03, 0xa0, 0x19, 0x26, 0x23, 0x9e, 0xef, 0x1a, 0xcb, 0x75, 0xd6,
    0xba, 0x81, 0xf4, 0x7e, 0x52, 0xe5, 0x2a, 0xe8, 0xf1, 0x49, 0x6c, 0x0f,
    0x1a, 0xa0, 0xf9, 0xc6, 0xe7, 0xec, 0x60, 0xe4, 0xcb, 0x2a, 0xb5, 0x56,
    0xe9, 0x9c, 0xcd, 0x19, 0x75, 0x92, 0xb1, 0x66, 0xce, 0xc3, 0xd9, 0x3d,
    0x11, 0xcb, 0xc4, 0x09, 0xce, 0x1e, 0x30, 0xba, 0x2f, 0x60, 0x60, 0x55,
    0x8d, 0x02, 0xdc, 0x5d, 0xaf, 0xf7, 0x52, 0x31, 0x17, 0x07, 0x53, 0x20,
    0x33, 0xad, 0x8c, 0xd5, 0x2f, 0x5a, 0xd0, 0x57, 0xd7, 0xd1, 0x80, 0xd6,
    0x3a, 0x9b, 0x04, 0x4f, 0x35, 0xbf, 0xe7, 0xd5, 0xbc, 0x8f, 0xd4, 0x81,
    0x02, 0x81, 0x81, 0x00, 0xc0, 0x9f, 0xf8, 0xcd, 0xf7, 0x3f, 0x26, 0x8a,
    0x3d, 0x4d, 0x2b, 0x0c, 0x01, 0xd0, 0xa2, 0xb4, 0x18, 0xfe, 0xf7, 0x5e,
    0x2f, 0x06, 0x13, 0xcd, 0x63, 0xaa, 0x12, 0xa9, 0x24, 0x86, 0xe3, 0xf3,
    0x7b, 0xda, 0x1a, 0x3c, 0xb1, 0x38, 0x80, 0x80, 0xef, 0x64, 0x64, 0xa1,
    0x9b, 0xfe, 0x76, 0x63, 0x8e, 0x83, 0xd2, 0xd9, 0xb9, 0x86, 0xb0, 0xe6,
    0xa6, 0x0c, 0x7e, 0xa8, 0x84, 0x90, 0x98, 0x0c, 0x1e, 0xf3, 0x14, 0x77,
    0xe0, 0x5f, 0x81, 0x08, 0x11, 0x8f, 0xa6, 0x23, 0xc4, 0xba, 0xc0, 0x8a,
    0xe4, 0xc6, 0xe3, 0x5c, 0xbe, 0xc5, 0xec, 0x2c, 0xb9, 0xd8, 0x8c, 0x4d,
    0x1a, 0x9d, 0xe7, 0x7c, 0x85, 0x4c, 0x0d, 0x71, 0x4e, 0x72, 0x33, 0x1b,
    0xfe, 0xa9, 0x17, 0x72, 0x76, 0x56, 0x9d, 0x74, 0x7e, 0x52, 0x67, 0x9a,
    0x87, 0x9a, 0xdb, 0x30, 0xde, 0xe4, 0x49, 0x28, 0x3b, 0xd2, 0x67, 0xaf,
    0x02, 0x81, 0x81, 0x00, 0x89, 0x74, 0x9a, 0x8e, 0xa7, 0xb9, 0xa5, 0x28,
    0xc0, 0x68, 0xe5, 0x6e, 0x63, 0x1c, 0x99, 0x20, 0x8f, 0x86, 0x8e, 0x12,
    0x9e, 0x69, 0x30, 0xfa, 0x34, 0xd9, 0x92, 0x8d, 0xdb, 0x7c, 0x37, 0xfd,
    0x28, 0xab, 0x61, 0x98, 0x52, 0x7f, 0x14, 0x1a, 0x39, 0xae, 0xfb, 0x6a,
    0x03, 0xa3, 0xe6, 0xbd, 0xb6, 0x5b, 0x6b, 0xe5, 0x5e, 0x9d, 0xc6, 0xa5,
    0x07, 0x27, 0x54, 0x17, 0xd0, 0x3d, 0x84, 0x9b, 0x3a, 0xa0, 0xd9, 0x1e,
    0x99, 0x6c, 0x63, 0x17, 0xab, 0xf1, 0x1f, 0x49, 0xba, 0x95, 0xe3, 0x3b,
    0x86, 0x8f, 0x42, 0xa4, 0x89, 0xf5, 0x94, 0x8f, 0x8b, 0x46, 0xbe, 0x84,
    0xba, 0x4a, 0xbc, 0x0d, 0x5f, 0x46, 0xeb, 0xe8, 0xec, 0x43, 0x8c, 0x1e,
    0xad, 0x19, 0x69, 0x2f, 0x08, 0x86, 0x7a, 0x3f, 0x7d, 0x0f, 0x07, 0x97,
    0xf3, 0x9a, 0x7b, 0xb5, 0xb2, 0xc1, 0x8c, 0x95, 0x68, 0x04, 0xa0, 0x81,
    0x02, 0x81, 0x80, 0x4e, 0xbf, 0x7e, 0x1b, 0xcb, 0x13, 0x61, 0x75, 0x3b,
    0xdb, 0x59, 0x5f, 0xb1, 0xd4, 0xb8, 0xeb, 0x9e, 0x73, 0xb5, 0xe7, 0xf6,
    0x89, 0x3d, 0x1c, 0xda, 0xf0, 0x36, 0xff, 0x35, 0xbd, 0x1e, 0x0b, 0x74,
    0xe3, 0x9e, 0xf0, 0xf2, 0xf7, 0xd7, 0x82, 0xb7, 0x7b, 0x6a, 0x1b, 0x0e,
    0x30, 0x4a, 0x98, 0x0e, 0xb4, 0xf9, 0x81, 0x07, 0xe4, 0x75, 0x39, 0xe9,
    0x53, 0xca, 0xbb, 0x5c, 0xaa, 0x93, 0x07, 0x0e, 0xa8, 0x2f, 0xba, 0x98,
    0x49, 0x30, 0xa7, 0xcc, 0x1a, 0x3c, 0x68, 0x0c, 0xe1, 0xa4, 0xb1, 0x05,
    0xe6, 0xe0, 0x25, 0x78, 0x58, 0x14, 0x37, 0xf5, 0x1f, 0xe3, 0x22, 0xef,
    0xa8, 0x0e, 0x22, 0xa0, 0x94, 0x3a, 0xf6, 0xc9, 0x13, 0xe6, 0x06, 0xbf,
    0x7f, 0x99, 0xc6, 0xcc, 0xd8, 0xc6, 0xbe, 0xd9, 0x2e, 0x24, 0xc7, 0x69,
    0x8c, 0x95, 0xba, 0xf6, 0x04, 0xb3, 0x0a, 0xf4, 0xcb, 0xf0, 0xce,
};

static const unsigned char kMsg[] = { 1, 2, 3, 4 };

static const unsigned char kSignature[] = {
    0xa5, 0xf0, 0x8a, 0x47, 0x5d, 0x3c, 0xb3, 0xcc, 0xa9, 0x79, 0xaf, 0x4d,
    0x8c, 0xae, 0x4c, 0x14, 0xef, 0xc2, 0x0b, 0x34, 0x36, 0xde, 0xf4, 0x3e,
    0x3d, 0xbb, 0x4a, 0x60, 0x5c, 0xc8, 0x91, 0x28, 0xda, 0xfb, 0x7e, 0x04,
    0x96, 0x7e, 0x63, 0x13, 0x90, 0xce, 0xb9, 0xb4, 0x62, 0x7a, 0xfd, 0x09,
    0x3d, 0xc7, 0x67, 0x78, 0x54, 0x04, 0xeb, 0x52, 0x62, 0x6e, 0x24, 0x67,
    0xb4, 0x40, 0xfc, 0x57, 0x62, 0xc6, 0xf1, 0x67, 0xc1, 0x97, 0x8f, 0x6a,
    0xa8, 0xae, 0x44, 0x46, 0x5e, 0xab, 0x67, 0x17, 0x53, 0x19, 0x3a, 0xda,
    0x5a, 0xc8, 0x16, 0x3e, 0x86, 0xd5, 0xc5, 0x71, 0x2f, 0xfc, 0x23, 0x48,
    0xd9, 0x0b, 0x13, 0xdd, 0x7b, 0x5a, 0x25, 0x79, 0xef, 0xa5, 0x7b, 0x04,
    0xed, 0x44, 0xf6, 0x18, 0x55, 0xe4, 0x0a, 0xe9, 0x57, 0x79, 0x5d, 0xd7,
    0x55, 0xa7, 0xab, 0x45, 0x02, 0x97, 0x60, 0x42,
};

/*
 * kExampleRSAKeyPKCS8 is kExampleRSAKeyDER encoded in a PKCS #8
 * PrivateKeyInfo.
 */
static const unsigned char kExampleRSAKeyPKCS8[] = {
    0x30, 0x82, 0x02, 0x76, 0x02, 0x01, 0x00, 0x30, 0x0d, 0x06, 0x09, 0x2a,
    0x86, 0x48, 0x86, 0xf7, 0x0d, 0x01, 0x01, 0x01, 0x05, 0x00, 0x04, 0x82,
    0x02, 0x60, 0x30, 0x82, 0x02, 0x5c, 0x02, 0x01, 0x00, 0x02, 0x81, 0x81,
    0x00, 0xf8, 0xb8, 0x6c, 0x83, 0xb4, 0xbc, 0xd9, 0xa8, 0x57, 0xc0, 0xa5,
    0xb4, 0x59, 0x76, 0x8c, 0x54, 0x1d, 0x79, 0xeb, 0x22, 0x52, 0x04, 0x7e,
    0xd3, 0x37, 0xeb, 0x41, 0xfd, 0x83, 0xf9, 0xf0, 0xa6, 0x85, 0x15, 0x34,
    0x75, 0x71, 0x5a, 0x84, 0xa8, 0x3c, 0xd2, 0xef, 0x5a, 0x4e, 0xd3, 0xde,
    0x97, 0x8a, 0xdd, 0xff, 0xbb, 0xcf, 0x0a, 0xaa, 0x86, 0x92, 0xbe, 0xb8,
    0x50, 0xe4, 0xcd, 0x6f, 0x80, 0x33, 0x30, 0x76, 0x13, 0x8f, 0xca, 0x7b,
    0xdc, 0xec, 0x5a, 0xca, 0x63, 0xc7, 0x03, 0x25, 0xef, 0xa8, 0x8a, 0x83,
    0x58, 0x76, 0x20, 0xfa, 0x16, 0x77, 0xd7, 0x79, 0x92, 0x63, 0x01, 0x48,
    0x1a, 0xd8, 0x7b, 0x67, 0xf1, 0x52, 0x55, 0x49, 0x4e, 0xd6, 0x6e, 0x4a,
    0x5c, 0xd7, 0x7a, 0x37, 0x36, 0x0c, 0xde, 0xdd, 0x8f, 0x44, 0xe8, 0xc2,
    0xa7, 0x2c, 0x2b, 0xb5, 0xaf, 0x64, 0x4b, 0x61, 0x07, 0x02, 0x03, 0x01,
    0x00, 0x01, 0x02, 0x81, 0x80, 0x74, 0x88, 0x64, 0x3f, 0x69, 0x45, 0x3a,
    0x6d, 0xc7, 0x7f, 0xb9, 0xa3, 0xc0, 0x6e, 0xec, 0xdc, 0xd4, 0x5a, 0xb5,
    0x32, 0x85, 0x5f, 0x19, 0xd4, 0xf8, 0xd4, 0x3f, 0x3c, 0xfa, 0xc2, 0xf6,
    0x5f, 0xee, 0xe6, 0xba, 0x87, 0x74, 0x2e, 0xc7, 0x0c, 0xd4, 0x42, 0xb8,
    0x66, 0x85, 0x9c, 0x7b, 0x24, 0x61, 0xaa, 0x16, 0x11, 0xf6, 0xb5, 0xb6,
    0xa4, 0x0a, 0xc9, 0x55, 0x2e, 0x81, 0xa5, 0x47, 0x61, 0xcb, 0x25, 0x8f,
    0xc2, 0x15, 0x7b, 0x0e, 0x7c, 0x36, 0x9f, 0x3a, 0xda, 0x58, 0x86, 0x1c,
    0x5b, 0x83, 0x79, 0xe6, 0x2b, 0xcc, 0xe6, 0xfa, 0x2c, 0x61, 0xf2, 0x78,
    0x80, 0x1b, 0xe2, 0xf3, 0x9d, 0x39, 0x2b, 0x65, 0x57, 0x91, 0x3d, 0x71,
    0x99, 0x73, 0xa5, 0xc2, 0x79, 0x20, 0x8c, 0x07, 0x4f, 0xe5, 0xb4, 0x60,
    0x1f, 0x99, 0xa2, 0xb1, 0x4f, 0x0c, 0xef, 0xbc, 0x59, 0x53, 0x00, 0x7d,
    0xb1, 0x02, 0x41, 0x00, 0xfc, 0x7e, 0x23, 0x65, 0x70, 0xf8, 0xce, 0xd3,
    0x40, 0x41, 0x80, 0x6a, 0x1d, 0x01, 0xd6, 0x01, 0xff, 0xb6, 0x1b, 0x3d,
    0x3d, 0x59, 0x09, 0x33, 0x79, 0xc0, 0x4f, 0xde, 0x96, 0x27, 0x4b, 0x18,
    0xc6, 0xd9, 0x78, 0xf1, 0xf4, 0x35, 0x46, 0xe9, 0x7c, 0x42, 0x7a, 0x5d,
    0x9f, 0xef, 0x54, 0xb8, 0xf7, 0x9f, 0xc4, 0x33, 0x6c, 0xf3, 0x8c, 0x32,
    0x46, 0x87, 0x67, 0x30, 0x7b, 0xa7, 0xac, 0xe3, 0x02, 0x41, 0x00, 0xfc,
    0x2c, 0xdf, 0x0c, 0x0d, 0x88, 0xf5, 0xb1, 0x92, 0xa8, 0x93, 0x47, 0x63,
    0x55, 0xf5, 0xca, 0x58, 0x43, 0xba, 0x1c, 0xe5, 0x9e, 0xb6, 0x95, 0x05,
    0xcd, 0xb5, 0x82, 0xdf, 0xeb, 0x04, 0x53, 0x9d, 0xbd, 0xc2, 0x38, 0x16,
    0xb3, 0x62, 0xdd, 0xa1, 0x46, 0xdb, 0x6d, 0x97, 0x93, 0x9f, 0x8a, 0xc3,
    0x9b, 0x64, 0x7e, 0x42, 0xe3, 0x32, 0x57, 0x19, 0x1b, 0xd5, 0x6e, 0x85,
    0xfa, 0xb8, 0x8d, 0x02, 0x41, 0x00, 0xbc, 0x3d, 0xde, 0x6d, 0xd6, 0x97,
    0xe8, 0xba, 0x9e, 0x81, 0x37, 0x17, 0xe5, 0xa0, 0x64, 0xc9, 0x00, 0xb7,
    0xe7, 0xfe, 0xf4, 0x29, 0xd9, 0x2e, 0x43, 0x6b, 0x19, 0x20, 0xbd, 0x99,
    0x75, 0xe7, 0x76, 0xf8, 0xd3, 0xae, 0xaf, 0x7e, 0xb8, 0xeb, 0x81, 0xf4,
    0x9d, 0xfe, 0x07, 0x2b, 0x0b, 0x63, 0x0b, 0x5a, 0x55, 0x90, 0x71, 0x7d,
    0xf1, 0xdb, 0xd9, 0xb1, 0x41, 0x41, 0x68, 0x2f, 0x4e, 0x39, 0x02, 0x40,
    0x5a, 0x34, 0x66, 0xd8, 0xf5, 0xe2, 0x7f, 0x18, 0xb5, 0x00, 0x6e, 0x26,
    0x84, 0x27, 0x14, 0x93, 0xfb, 0xfc, 0xc6, 0x0f, 0x5e, 0x27, 0xe6, 0xe1,
    0xe9, 0xc0, 0x8a, 0xe4, 0x34, 0xda, 0xe9, 0xa2, 0x4b, 0x73, 0xbc, 0x8c,
    0xb9, 0xba, 0x13, 0x6c, 0x7a, 0x2b, 0x51, 0x84, 0xa3, 0x4a, 0xe0, 0x30,
    0x10, 0x06, 0x7e, 0xed, 0x17, 0x5a, 0x14, 0x00, 0xc9, 0xef, 0x85, 0xea,
    0x52, 0x2c, 0xbc, 0x65, 0x02, 0x40, 0x51, 0xe3, 0xf2, 0x83, 0x19, 0x9b,
    0xc4, 0x1e, 0x2f, 0x50, 0x3d, 0xdf, 0x5a, 0xa2, 0x18, 0xca, 0x5f, 0x2e,
    0x49, 0xaf, 0x6f, 0xcc, 0xfa, 0x65, 0x77, 0x94, 0xb5, 0xa1, 0x0a, 0xa9,
    0xd1, 0x8a, 0x39, 0x37, 0xf4, 0x0b, 0xa0, 0xd7, 0x82, 0x27, 0x5e, 0xae,
    0x17, 0x17, 0xa1, 0x1e, 0x54, 0x34, 0xbf, 0x6e, 0xc4, 0x8e, 0x99, 0x5d,
    0x08, 0xf1, 0x2d, 0x86, 0x9d, 0xa5, 0x20, 0x1b, 0xe5, 0xdf,
};

#ifndef OPENSSL_NO_EC
/*
 * kExampleECKeyDER is a sample EC private key encoded as an ECPrivateKey
 * structure.
 */
static const unsigned char kExampleECKeyDER[] = {
    0x30, 0x77, 0x02, 0x01, 0x01, 0x04, 0x20, 0x07, 0x0f, 0x08, 0x72, 0x7a,
    0xd4, 0xa0, 0x4a, 0x9c, 0xdd, 0x59, 0xc9, 0x4d, 0x89, 0x68, 0x77, 0x08,
    0xb5, 0x6f, 0xc9, 0x5d, 0x30, 0x77, 0x0e, 0xe8, 0xd1, 0xc9, 0xce, 0x0a,
    0x8b, 0xb4, 0x6a, 0xa0, 0x0a, 0x06, 0x08, 0x2a, 0x86, 0x48, 0xce, 0x3d,
    0x03, 0x01, 0x07, 0xa1, 0x44, 0x03, 0x42, 0x00, 0x04, 0xe6, 0x2b, 0x69,
    0xe2, 0xbf, 0x65, 0x9f, 0x97, 0xbe, 0x2f, 0x1e, 0x0d, 0x94, 0x8a, 0x4c,
    0xd5, 0x97, 0x6b, 0xb7, 0xa9, 0x1e, 0x0d, 0x46, 0xfb, 0xdd, 0xa9, 0xa9,
    0x1e, 0x9d, 0xdc, 0xba, 0x5a, 0x01, 0xe7, 0xd6, 0x97, 0xa8, 0x0a, 0x18,
    0xf9, 0xc3, 0xc4, 0xa3, 0x1e, 0x56, 0xe2, 0x7c, 0x83, 0x48, 0xdb, 0x16,
    0x1a, 0x1c, 0xf5, 0x1d, 0x7e, 0xf1, 0x94, 0x2d, 0x4b, 0xcf, 0x72, 0x22,
    0xc1,
};

/*
 * kExampleBadECKeyDER is a sample EC private key encoded as an ECPrivateKey
 * structure. The private key is equal to the order and will fail to import
 */
static const unsigned char kExampleBadECKeyDER[] = {
    0x30, 0x66, 0x02, 0x01, 0x00, 0x30, 0x13, 0x06, 0x07, 0x2A, 0x86, 0x48,
    0xCE, 0x3D, 0x02, 0x01, 0x06, 0x08, 0x2A, 0x86, 0x48, 0xCE, 0x3D, 0x03,
    0x01, 0x07, 0x04, 0x4C, 0x30, 0x4A, 0x02, 0x01, 0x01, 0x04, 0x20, 0xFF,
    0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xBC, 0xE6, 0xFA, 0xAD, 0xA7, 0x17, 0x9E, 0x84, 0xF3,
    0xB9, 0xCA, 0xC2, 0xFC, 0x63, 0x25, 0x51, 0xA1, 0x23, 0x03, 0x21, 0x00,
    0x00, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xBC, 0xE6, 0xFA, 0xAD, 0xA7, 0x17, 0x9E, 0x84,
    0xF3, 0xB9, 0xCA, 0xC2, 0xFC, 0x63, 0x25, 0x51
};

/* prime256v1 */
static const unsigned char kExampleECPubKeyDER[] = {
    0x30, 0x59, 0x30, 0x13, 0x06, 0x07, 0x2a, 0x86, 0x48, 0xce, 0x3d, 0x02,
    0x01, 0x06, 0x08, 0x2a, 0x86, 0x48, 0xce, 0x3d, 0x03, 0x01, 0x07, 0x03,
    0x42, 0x00, 0x04, 0xba, 0xeb, 0x83, 0xfb, 0x3b, 0xb2, 0xff, 0x30, 0x53,
    0xdb, 0xce, 0x32, 0xf2, 0xac, 0xae, 0x44, 0x0d, 0x3d, 0x13, 0x53, 0xb8,
    0xd1, 0x68, 0x55, 0xde, 0x44, 0x46, 0x05, 0xa6, 0xc9, 0xd2, 0x04, 0xb7,
    0xe3, 0xa2, 0x96, 0xc8, 0xb2, 0x5e, 0x22, 0x03, 0xd7, 0x03, 0x7a, 0x8b,
    0x13, 0x5c, 0x42, 0x49, 0xc2, 0xab, 0x86, 0xd6, 0xac, 0x6b, 0x93, 0x20,
    0x56, 0x6a, 0xc6, 0xc8, 0xa5, 0x0b, 0xe5
};

/*
 * kExampleBadECPubKeyDER is a sample EC public key with a wrong OID
 * 1.2.840.10045.2.2 instead of 1.2.840.10045.2.1 - EC Public Key
 */
static const unsigned char kExampleBadECPubKeyDER[] = {
    0x30, 0x59, 0x30, 0x13, 0x06, 0x07, 0x2a, 0x86, 0x48, 0xce, 0x3d, 0x02,
    0x02, 0x06, 0x08, 0x2a, 0x86, 0x48, 0xce, 0x3d, 0x03, 0x01, 0x07, 0x03,
    0x42, 0x00, 0x04, 0xba, 0xeb, 0x83, 0xfb, 0x3b, 0xb2, 0xff, 0x30, 0x53,
    0xdb, 0xce, 0x32, 0xf2, 0xac, 0xae, 0x44, 0x0d, 0x3d, 0x13, 0x53, 0xb8,
    0xd1, 0x68, 0x55, 0xde, 0x44, 0x46, 0x05, 0xa6, 0xc9, 0xd2, 0x04, 0xb7,
    0xe3, 0xa2, 0x96, 0xc8, 0xb2, 0x5e, 0x22, 0x03, 0xd7, 0x03, 0x7a, 0x8b,
    0x13, 0x5c, 0x42, 0x49, 0xc2, 0xab, 0x86, 0xd6, 0xac, 0x6b, 0x93, 0x20,
    0x56, 0x6a, 0xc6, 0xc8, 0xa5, 0x0b, 0xe5
};

static const unsigned char pExampleECParamDER[] = {
    0x06, 0x08, 0x2a, 0x86, 0x48, 0xce, 0x3d, 0x03, 0x01, 0x07
};
#endif

typedef struct APK_DATA_st {
    const unsigned char *kder;
    size_t size;
    int evptype;
    int check;
    int pub_check;
    int param_check;
    int type; /* 0 for private, 1 for public, 2 for params */
} APK_DATA;

static APK_DATA keydata[] = {
    {kExampleRSAKeyDER, sizeof(kExampleRSAKeyDER), EVP_PKEY_RSA},
    {kExampleRSAKeyPKCS8, sizeof(kExampleRSAKeyPKCS8), EVP_PKEY_RSA},
#ifndef OPENSSL_NO_EC
    {kExampleECKeyDER, sizeof(kExampleECKeyDER), EVP_PKEY_EC}
#endif
};

static APK_DATA keycheckdata[] = {
    {kExampleRSAKeyDER, sizeof(kExampleRSAKeyDER), EVP_PKEY_RSA, 1, -2, -2, 0},
    {kExampleBadRSAKeyDER, sizeof(kExampleBadRSAKeyDER), EVP_PKEY_RSA,
     0, -2, -2, 0},
#ifndef OPENSSL_NO_EC
    {kExampleECKeyDER, sizeof(kExampleECKeyDER), EVP_PKEY_EC, 1, 1, 1, 0},
    /* group is also associated in our pub key */
    {kExampleECPubKeyDER, sizeof(kExampleECPubKeyDER), EVP_PKEY_EC, 0, 1, 1, 1},
    {pExampleECParamDER, sizeof(pExampleECParamDER), EVP_PKEY_EC, 0, 0, 1, 2}
#endif
};

static EVP_PKEY *load_example_rsa_key(void)
{
    EVP_PKEY *ret = NULL;
    const unsigned char *derp = kExampleRSAKeyDER;
    EVP_PKEY *pkey = NULL;
    RSA *rsa = NULL;

    if (!TEST_true(d2i_RSAPrivateKey(&rsa, &derp, sizeof(kExampleRSAKeyDER))))
        return NULL;

    if (!TEST_ptr(pkey = EVP_PKEY_new())
            || !TEST_true(EVP_PKEY_set1_RSA(pkey, rsa)))
        goto end;

    ret = pkey;
    pkey = NULL;

end:
    EVP_PKEY_free(pkey);
    RSA_free(rsa);

    return ret;
}

#ifndef OPENSSL_NO_DSA
static EVP_PKEY *load_example_dsa_key(void)
{
    EVP_PKEY *ret = NULL;
    const unsigned char *derp = kExampleDSAKeyDER;
    EVP_PKEY *pkey = NULL;
    DSA *dsa = NULL;

    if (!TEST_true(d2i_DSAPrivateKey(&dsa, &derp, sizeof(kExampleDSAKeyDER))))
        return NULL;

    if (!TEST_ptr(pkey = EVP_PKEY_new())
            || !TEST_true(EVP_PKEY_set1_DSA(pkey, dsa)))
        goto end;

    ret = pkey;
    pkey = NULL;

end:
    EVP_PKEY_free(pkey);
    DSA_free(dsa);

    return ret;
}
#endif

static EVP_PKEY *load_example_hmac_key(void)
{
    EVP_PKEY *pkey = NULL;
    unsigned char key[] = {
        0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b,
        0x0c, 0x0d, 0x0e, 0x0f, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
        0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f
    };

    pkey = EVP_PKEY_new_raw_private_key(EVP_PKEY_HMAC, NULL, key, sizeof(key));
    if (!TEST_ptr(pkey))
        return NULL;

    return pkey;
}

static int test_EVP_set_default_properties(void)
{
    OPENSSL_CTX *ctx;
    EVP_MD *md = NULL;
    int res = 0;

    if (!TEST_ptr(ctx = OPENSSL_CTX_new())
            || !TEST_ptr(md = EVP_MD_fetch(ctx, "sha256", NULL)))
        goto err;
    EVP_MD_free(md);
    md = NULL;

    if (!TEST_true(EVP_set_default_properties(ctx, "provider=fizzbang"))
            || !TEST_ptr_null(md = EVP_MD_fetch(ctx, "sha256", NULL))
            || !TEST_ptr(md = EVP_MD_fetch(ctx, "sha256", "-provider")))
        goto err;
    EVP_MD_free(md);
    md = NULL;

    if (!TEST_true(EVP_set_default_properties(ctx, NULL))
            || !TEST_ptr(md = EVP_MD_fetch(ctx, "sha256", NULL)))
        goto err;
    res = 1;
err:
    EVP_MD_free(md);
    OPENSSL_CTX_free(ctx);
    return res;
}

static int test_EVP_Enveloped(void)
{
    int ret = 0;
    EVP_CIPHER_CTX *ctx = NULL;
    EVP_PKEY *keypair = NULL;
    unsigned char *kek = NULL;
    unsigned char iv[EVP_MAX_IV_LENGTH];
    static const unsigned char msg[] = { 1, 2, 3, 4, 5, 6, 7, 8 };
    int len, kek_len, ciphertext_len, plaintext_len;
    unsigned char ciphertext[32], plaintext[16];
    const EVP_CIPHER *type = EVP_aes_256_cbc();

    if (!TEST_ptr(keypair = load_example_rsa_key())
            || !TEST_ptr(kek = OPENSSL_zalloc(EVP_PKEY_size(keypair)))
            || !TEST_ptr(ctx = EVP_CIPHER_CTX_new())
            || !TEST_true(EVP_SealInit(ctx, type, &kek, &kek_len, iv,
                                       &keypair, 1))
            || !TEST_true(EVP_SealUpdate(ctx, ciphertext, &ciphertext_len,
                                         msg, sizeof(msg)))
            || !TEST_true(EVP_SealFinal(ctx, ciphertext + ciphertext_len,
                                        &len)))
        goto err;

    ciphertext_len += len;

    if (!TEST_true(EVP_OpenInit(ctx, type, kek, kek_len, iv, keypair))
            || !TEST_true(EVP_OpenUpdate(ctx, plaintext, &plaintext_len,
                                         ciphertext, ciphertext_len))
            || !TEST_true(EVP_OpenFinal(ctx, plaintext + plaintext_len, &len)))
        goto err;

    plaintext_len += len;
    if (!TEST_mem_eq(msg, sizeof(msg), plaintext, plaintext_len))
        goto err;

    ret = 1;
err:
    OPENSSL_free(kek);
    EVP_PKEY_free(keypair);
    EVP_CIPHER_CTX_free(ctx);
    return ret;
}

/*
 * Test 0: Standard calls to EVP_DigestSignInit/Update/Final (Implicit fetch digest, RSA)
 * Test 1: Standard calls to EVP_DigestSignInit/Update/Final (Implicit fetch digest, DSA)
 * Test 2: Standard calls to EVP_DigestSignInit/Update/Final (Implicit fetch digest, HMAC)
 * Test 3: Standard calls to EVP_DigestSignInit/Update/Final (Explicit fetch digest, RSA)
 * Test 4: Standard calls to EVP_DigestSignInit/Update/Final (Explicit fetch digest, DSA)
 * Test 5: Standard calls to EVP_DigestSignInit/Update/Final (Explicit fetch diegst, HMAC)
 * Test 6: Use an MD BIO to do the Update calls instead (RSA)
 * Test 7: Use an MD BIO to do the Update calls instead (DSA)
 * Test 8: Use an MD BIO to do the Update calls instead (HMAC)
 */
static int test_EVP_DigestSignInit(int tst)
{
    int ret = 0;
    EVP_PKEY *pkey = NULL;
    unsigned char *sig = NULL;
    size_t sig_len = 0;
    EVP_MD_CTX *md_ctx = NULL, *md_ctx_verify = NULL;
    EVP_MD_CTX *a_md_ctx = NULL, *a_md_ctx_verify = NULL;
    BIO *mdbio = NULL, *membio = NULL;
    size_t written;
    const EVP_MD *md;
    EVP_MD *mdexp = NULL;

    if (tst >= 6) {
        membio = BIO_new(BIO_s_mem());
        mdbio = BIO_new(BIO_f_md());
        if (!TEST_ptr(membio) || !TEST_ptr(mdbio))
            goto out;
        BIO_push(mdbio, membio);
        if (!TEST_int_gt(BIO_get_md_ctx(mdbio, &md_ctx), 0))
            goto out;
    } else {
        if (!TEST_ptr(a_md_ctx = md_ctx = EVP_MD_CTX_new())
                || !TEST_ptr(a_md_ctx_verify = md_ctx_verify = EVP_MD_CTX_new()))
            goto out;
    }

    if (tst == 0 || tst == 3 || tst == 6) {
        if (!TEST_ptr(pkey = load_example_rsa_key()))
                goto out;
    } else if (tst == 1 || tst == 4 || tst == 7) {
#ifndef OPENSSL_NO_DSA
        if (!TEST_ptr(pkey = load_example_dsa_key()))
                goto out;
#else
        ret = 1;
        goto out;
#endif
    } else {
        if (!TEST_ptr(pkey = load_example_hmac_key()))
                goto out;
    }

    if (tst >= 3 && tst <= 5)
        md = mdexp = EVP_MD_fetch(NULL, "SHA256", NULL);
    else
        md = EVP_sha256();

    if (!TEST_true(EVP_DigestSignInit(md_ctx, NULL, md, NULL, pkey)))
        goto out;

    if (tst >= 6) {
        if (!BIO_write_ex(mdbio, kMsg, sizeof(kMsg), &written))
            goto out;
    } else {
        if (!TEST_true(EVP_DigestSignUpdate(md_ctx, kMsg, sizeof(kMsg))))
            goto out;
    }

    /* Determine the size of the signature. */
    if (!TEST_true(EVP_DigestSignFinal(md_ctx, NULL, &sig_len))
            || !TEST_ptr(sig = OPENSSL_malloc(sig_len))
            || !TEST_true(EVP_DigestSignFinal(md_ctx, sig, &sig_len)))
        goto out;

    if (tst >= 6) {
        if (!TEST_int_gt(BIO_reset(mdbio), 0)
                || !TEST_int_gt(BIO_get_md_ctx(mdbio, &md_ctx_verify), 0))
            goto out;
    }

    /*
     * Ensure that the signature round-trips (Verification isn't supported for
     * HMAC via EVP_DigestVerify*)
     */
    if (tst != 2 && tst != 5 && tst != 8) {
        if (!TEST_true(EVP_DigestVerifyInit(md_ctx_verify, NULL, md,
                                            NULL, pkey)))
            goto out;

        if (tst >= 6) {
            if (!TEST_true(BIO_write_ex(mdbio, kMsg, sizeof(kMsg), &written)))
                goto out;
        } else {
            if (!TEST_true(EVP_DigestVerifyUpdate(md_ctx_verify, kMsg,
                                                  sizeof(kMsg))))
                goto out;
        }
        if (!TEST_true(EVP_DigestVerifyFinal(md_ctx_verify, sig, sig_len)))
            goto out;
    }

    ret = 1;

 out:
    BIO_free(membio);
    BIO_free(mdbio);
    EVP_MD_CTX_free(a_md_ctx);
    EVP_MD_CTX_free(a_md_ctx_verify);
    EVP_PKEY_free(pkey);
    OPENSSL_free(sig);
    EVP_MD_free(mdexp);

    return ret;
}

static int test_EVP_DigestVerifyInit(void)
{
    int ret = 0;
    EVP_PKEY *pkey = NULL;
    EVP_MD_CTX *md_ctx = NULL;

    if (!TEST_ptr(md_ctx = EVP_MD_CTX_new())
            || !TEST_ptr(pkey = load_example_rsa_key()))
        goto out;

    if (!TEST_true(EVP_DigestVerifyInit(md_ctx, NULL, EVP_sha256(), NULL, pkey))
            || !TEST_true(EVP_DigestVerifyUpdate(md_ctx, kMsg, sizeof(kMsg)))
            || !TEST_true(EVP_DigestVerifyFinal(md_ctx, kSignature,
                                                 sizeof(kSignature))))
        goto out;
    ret = 1;

 out:
    EVP_MD_CTX_free(md_ctx);
    EVP_PKEY_free(pkey);
    return ret;
}

static int test_d2i_AutoPrivateKey(int i)
{
    int ret = 0;
    const unsigned char *p;
    EVP_PKEY *pkey = NULL;
    const APK_DATA *ak = &keydata[i];
    const unsigned char *input = ak->kder;
    size_t input_len = ak->size;
    int expected_id = ak->evptype;

    p = input;
    if (!TEST_ptr(pkey = d2i_AutoPrivateKey(NULL, &p, input_len))
            || !TEST_ptr_eq(p, input + input_len)
            || !TEST_int_eq(EVP_PKEY_id(pkey), expected_id))
        goto done;

    ret = 1;

 done:
    EVP_PKEY_free(pkey);
    return ret;
}

#ifndef OPENSSL_NO_EC

static const unsigned char ec_public_sect163k1_validxy[] = {
    0x30, 0x40, 0x30, 0x10, 0x06, 0x07, 0x2a, 0x86, 0x48, 0xce, 0x3d, 0x02,
    0x01, 0x06, 0x05, 0x2b, 0x81, 0x04, 0x00, 0x01, 0x03, 0x2c, 0x00, 0x04,
    0x02, 0x84, 0x58, 0xa6, 0xd4, 0xa0, 0x35, 0x2b, 0xae, 0xf0, 0xc0, 0x69,
    0x05, 0xcf, 0x2a, 0x50, 0x33, 0xf9, 0xe3, 0x92, 0x79, 0x02, 0xd1, 0x7b,
    0x9f, 0x22, 0x00, 0xf0, 0x3b, 0x0e, 0x5d, 0x2e, 0xb7, 0x23, 0x24, 0xf3,
    0x6a, 0xd8, 0x17, 0x65, 0x41, 0x2f
};

static const unsigned char ec_public_sect163k1_badx[] = {
    0x30, 0x40, 0x30, 0x10, 0x06, 0x07, 0x2a, 0x86, 0x48, 0xce, 0x3d, 0x02,
    0x01, 0x06, 0x05, 0x2b, 0x81, 0x04, 0x00, 0x01, 0x03, 0x2c, 0x00, 0x04,
    0x0a, 0x84, 0x58, 0xa6, 0xd4, 0xa0, 0x35, 0x2b, 0xae, 0xf0, 0xc0, 0x69,
    0x05, 0xcf, 0x2a, 0x50, 0x33, 0xf9, 0xe3, 0x92, 0xb0, 0x02, 0xd1, 0x7b,
    0x9f, 0x22, 0x00, 0xf0, 0x3b, 0x0e, 0x5d, 0x2e, 0xb7, 0x23, 0x24, 0xf3,
    0x6a, 0xd8, 0x17, 0x65, 0x41, 0x2f
};

static const unsigned char ec_public_sect163k1_bady[] = {
    0x30, 0x40, 0x30, 0x10, 0x06, 0x07, 0x2a, 0x86, 0x48, 0xce, 0x3d, 0x02,
    0x01, 0x06, 0x05, 0x2b, 0x81, 0x04, 0x00, 0x01, 0x03, 0x2c, 0x00, 0x04,
    0x02, 0x84, 0x58, 0xa6, 0xd4, 0xa0, 0x35, 0x2b, 0xae, 0xf0, 0xc0, 0x69,
    0x05, 0xcf, 0x2a, 0x50, 0x33, 0xf9, 0xe3, 0x92, 0x79, 0x0a, 0xd1, 0x7b,
    0x9f, 0x22, 0x00, 0xf0, 0x3b, 0x0e, 0x5d, 0x2e, 0xb7, 0x23, 0x24, 0xf3,
    0x6a, 0xd8, 0x17, 0x65, 0x41, 0xe6
};

static struct ec_der_pub_keys_st {
    const unsigned char *der;
    size_t len;
    int valid;
} ec_der_pub_keys[] = {
    { ec_public_sect163k1_validxy, sizeof(ec_public_sect163k1_validxy), 1 },
    { ec_public_sect163k1_badx, sizeof(ec_public_sect163k1_badx), 0 },
    { ec_public_sect163k1_bady, sizeof(ec_public_sect163k1_bady), 0 },
};

/*
 * Tests the range of the decoded EC char2 public point.
 * See ec_GF2m_simple_oct2point().
 */
static int test_invalide_ec_char2_pub_range_decode(int id)
{
    int ret = 0;
    BIO *bio = NULL;
    EC_KEY *eckey = NULL;

    if (!TEST_ptr(bio = BIO_new_mem_buf(ec_der_pub_keys[id].der,
                                        ec_der_pub_keys[id].len)))
        goto err;
    eckey = d2i_EC_PUBKEY_bio(bio, NULL);
    ret = (ec_der_pub_keys[id].valid && TEST_ptr(eckey))
          || TEST_ptr_null(eckey);
err:
    EC_KEY_free(eckey);
    BIO_free(bio);
    return ret;
}

/* Tests loading a bad key in PKCS8 format */
static int test_EVP_PKCS82PKEY(void)
{
    int ret = 0;
    const unsigned char *derp = kExampleBadECKeyDER;
    PKCS8_PRIV_KEY_INFO *p8inf = NULL;
    EVP_PKEY *pkey = NULL;

    if (!TEST_ptr(p8inf = d2i_PKCS8_PRIV_KEY_INFO(NULL, &derp,
                                              sizeof(kExampleBadECKeyDER))))
        goto done;

    if (!TEST_ptr_eq(derp,
                     kExampleBadECKeyDER + sizeof(kExampleBadECKeyDER)))
        goto done;

    if (!TEST_ptr_null(pkey = EVP_PKCS82PKEY(p8inf)))
        goto done;

    ret = 1;

 done:
    PKCS8_PRIV_KEY_INFO_free(p8inf);
    EVP_PKEY_free(pkey);

    return ret;
}
#endif

/* This uses kExampleRSAKeyDER and kExampleRSAKeyPKCS8 to verify encoding */
static int test_privatekey_to_pkcs8(void)
{
    EVP_PKEY *pkey = NULL;
    BIO *membio = NULL;
    char *membuf = NULL;
    long membuf_len = 0;
    int ok = 0;

    if (!TEST_ptr(membio = BIO_new(BIO_s_mem()))
        || !TEST_ptr(pkey = load_example_rsa_key())
        || !TEST_int_gt(i2d_PKCS8PrivateKey_bio(membio, pkey, NULL,
                                                NULL, 0, NULL, NULL),
                        0)
        || !TEST_int_gt(membuf_len = BIO_get_mem_data(membio, &membuf), 0)
        || !TEST_ptr(membuf)
        || !TEST_mem_eq(membuf, (size_t)membuf_len,
                        kExampleRSAKeyPKCS8, sizeof(kExampleRSAKeyPKCS8))
        /*
         * We try to write PEM as well, just to see that it doesn't err, but
         * assume that the result is correct.
         */
        || !TEST_int_gt(PEM_write_bio_PKCS8PrivateKey(membio, pkey, NULL,
                                                      NULL, 0, NULL, NULL),
                        0))
        goto done;

    ok = 1;
 done:
    EVP_PKEY_free(pkey);
    BIO_free_all(membio);
    return ok;
}

#ifndef OPENSSL_NO_EC
static const struct {
    int encoding;
    const char *encoding_name;
} ec_encodings[] = {
    { OPENSSL_EC_EXPLICIT_CURVE, OSSL_PKEY_EC_ENCODING_EXPLICIT },
    { OPENSSL_EC_NAMED_CURVE,    OSSL_PKEY_EC_ENCODING_GROUP }
};

static int ec_export_get_encoding_cb(const OSSL_PARAM params[], void *arg)
{
    const OSSL_PARAM *p;
    const char *enc_name = NULL;
    int *enc = arg;
    size_t i;

    *enc = -1;

    if (!TEST_ptr(p = OSSL_PARAM_locate_const(params,
                                              OSSL_PKEY_PARAM_EC_ENCODING))
        || !TEST_true(OSSL_PARAM_get_utf8_string_ptr(p, &enc_name)))
        return 0;

    for (i = 0; i < OSSL_NELEM(ec_encodings); i++) {
        if (strcasecmp(enc_name, ec_encodings[i].encoding_name) == 0) {
            *enc = ec_encodings[i].encoding;
            break;
        }
    }

    return (*enc != -1);
}

static int test_EC_keygen_with_enc(int idx)
{
    EVP_PKEY *params = NULL, *key = NULL;
    EVP_PKEY_CTX *pctx = NULL, *kctx = NULL;
    int enc;
    int ret = 0;

    enc = ec_encodings[idx].encoding;

    /* Create key parameters */
    if (!TEST_ptr(pctx = EVP_PKEY_CTX_new_from_name(testctx, "EC", NULL))
        || !TEST_true(EVP_PKEY_paramgen_init(pctx))
        || !TEST_true(EVP_PKEY_CTX_set_group_name(pctx, "P-256"))
        || !TEST_true(EVP_PKEY_CTX_set_ec_param_enc(pctx, enc))
        || !TEST_true(EVP_PKEY_paramgen(pctx, &params))
        || !TEST_ptr(params))
        goto done;

    /* Create key */
    if (!TEST_ptr(kctx = EVP_PKEY_CTX_new_from_pkey(testctx, params, NULL))
        || !TEST_true(EVP_PKEY_keygen_init(kctx))
        || !TEST_true(EVP_PKEY_keygen(kctx, &key))
        || !TEST_ptr(key))
        goto done;

    /* Check that the encoding got all the way into the key */
    if (!TEST_true(evp_keymgmt_util_export(key, OSSL_KEYMGMT_SELECT_ALL,
                                           ec_export_get_encoding_cb, &enc))
        || !TEST_int_eq(enc, ec_encodings[idx].encoding))
        goto done;

    ret = 1;
 done:
    EVP_PKEY_free(key);
    EVP_PKEY_free(params);
    EVP_PKEY_CTX_free(kctx);
    EVP_PKEY_CTX_free(pctx);
    return ret;
}
#endif

#if !defined(OPENSSL_NO_SM2) && !defined(FIPS_MODULE)

static int test_EVP_SM2_verify(void)
{
    /* From https://tools.ietf.org/html/draft-shen-sm2-ecdsa-02#appendix-A */
    const char *pubkey =
       "-----BEGIN PUBLIC KEY-----\n"
       "MIIBMzCB7AYHKoZIzj0CATCB4AIBATAsBgcqhkjOPQEBAiEAhULWnkwETxjouSQ1\n"
       "v2/33kVyg5FcRVF9ci7biwjx38MwRAQgeHlotPoyw/0kF4Quc7v+/y88hItoMdfg\n"
       "7GUiizk35JgEIGPkxtOyOwyEnPhCQUhL/kj2HVmlsWugbm4S0donxSSaBEEEQh3r\n"
       "1hti6rZ0ZDTrw8wxXjIiCzut1QvcTE5sFH/t1D0GgFEry7QsB9RzSdIVO3DE5df9\n"
       "/L+jbqGoWEG55G4JogIhAIVC1p5MBE8Y6LkkNb9v990pdyBjBIVijVrnTufDLnm3\n"
       "AgEBA0IABArkx3mKoPEZRxvuEYJb5GICu3nipYRElel8BP9N8lSKfAJA+I8c1OFj\n"
       "Uqc8F7fxbwc1PlOhdtaEqf4Ma7eY6Fc=\n"
       "-----END PUBLIC KEY-----\n";

    const char *msg = "message digest";
    const char *id = "ALICE123@YAHOO.COM";

    const uint8_t signature[] = {
       0x30, 0x44, 0x02, 0x20,

       0x40, 0xF1, 0xEC, 0x59, 0xF7, 0x93, 0xD9, 0xF4, 0x9E, 0x09, 0xDC,
       0xEF, 0x49, 0x13, 0x0D, 0x41, 0x94, 0xF7, 0x9F, 0xB1, 0xEE, 0xD2,
       0xCA, 0xA5, 0x5B, 0xAC, 0xDB, 0x49, 0xC4, 0xE7, 0x55, 0xD1,

       0x02, 0x20,

       0x6F, 0xC6, 0xDA, 0xC3, 0x2C, 0x5D, 0x5C, 0xF1, 0x0C, 0x77, 0xDF,
       0xB2, 0x0F, 0x7C, 0x2E, 0xB6, 0x67, 0xA4, 0x57, 0x87, 0x2F, 0xB0,
       0x9E, 0xC5, 0x63, 0x27, 0xA6, 0x7E, 0xC7, 0xDE, 0xEB, 0xE7
    };

    int rc = 0;
    BIO *bio = NULL;
    EVP_PKEY *pkey = NULL;
    EVP_MD_CTX *mctx = NULL;
    EVP_PKEY_CTX *pctx = NULL;

    bio = BIO_new_mem_buf(pubkey, strlen(pubkey));
    if (!TEST_true(bio != NULL))
        goto done;

    pkey = PEM_read_bio_PUBKEY(bio, NULL, NULL, NULL);
    if (!TEST_true(pkey != NULL))
        goto done;

#ifdef TMP_SM2_HACK
    if (!TEST_ptr(EVP_PKEY_get0(pkey)))
        goto done;
#endif

    if (!TEST_true(EVP_PKEY_set_alias_type(pkey, EVP_PKEY_SM2)))
        goto done;

    if (!TEST_ptr(mctx = EVP_MD_CTX_new()))
        goto done;

    if (!TEST_ptr(pctx = EVP_PKEY_CTX_new(pkey, NULL)))
        goto done;

    if (!TEST_int_gt(EVP_PKEY_CTX_set1_id(pctx, (const uint8_t *)id,
                                          strlen(id)), 0))
        goto done;

    EVP_MD_CTX_set_pkey_ctx(mctx, pctx);

    if (!TEST_true(EVP_DigestVerifyInit(mctx, NULL, EVP_sm3(), NULL, pkey)))
        goto done;

    if (!TEST_true(EVP_DigestVerifyUpdate(mctx, msg, strlen(msg))))
        goto done;

    if (!TEST_true(EVP_DigestVerifyFinal(mctx, signature, sizeof(signature))))
        goto done;
    rc = 1;

 done:
    BIO_free(bio);
    EVP_PKEY_free(pkey);
    EVP_PKEY_CTX_free(pctx);
    EVP_MD_CTX_free(mctx);
    return rc;
}

static int test_EVP_SM2(void)
{
    int ret = 0;
    EVP_PKEY *pkey = NULL;
    EVP_PKEY *params = NULL;
    EVP_PKEY_CTX *pctx = NULL;
    EVP_PKEY_CTX *kctx = NULL;
    EVP_PKEY_CTX *sctx = NULL;
    size_t sig_len = 0;
    unsigned char *sig = NULL;
    EVP_MD_CTX *md_ctx = NULL;
    EVP_MD_CTX *md_ctx_verify = NULL;
    EVP_PKEY_CTX *cctx = NULL;

    uint8_t ciphertext[128];
    size_t ctext_len = sizeof(ciphertext);

    uint8_t plaintext[8];
    size_t ptext_len = sizeof(plaintext);

    uint8_t sm2_id[] = {1, 2, 3, 4, 'l', 'e', 't', 't', 'e', 'r'};

    pctx = EVP_PKEY_CTX_new_id(EVP_PKEY_EC, NULL);
    if (!TEST_ptr(pctx))
        goto done;

    if (!TEST_true(EVP_PKEY_paramgen_init(pctx) == 1))
        goto done;

    if (!TEST_true(EVP_PKEY_CTX_set_ec_paramgen_curve_nid(pctx, NID_sm2)))
        goto done;

    if (!TEST_true(EVP_PKEY_paramgen(pctx, &params)))
        goto done;

    kctx = EVP_PKEY_CTX_new(params, NULL);
    if (!TEST_ptr(kctx))
        goto done;

    if (!TEST_true(EVP_PKEY_keygen_init(kctx)))
        goto done;

    if (!TEST_true(EVP_PKEY_keygen(kctx, &pkey)))
        goto done;

    if (!TEST_true(EVP_PKEY_set_alias_type(pkey, EVP_PKEY_SM2)))
        goto done;

    if (!TEST_ptr(md_ctx = EVP_MD_CTX_new()))
        goto done;

    if (!TEST_ptr(md_ctx_verify = EVP_MD_CTX_new()))
        goto done;

    if (!TEST_ptr(sctx = EVP_PKEY_CTX_new(pkey, NULL)))
        goto done;

    EVP_MD_CTX_set_pkey_ctx(md_ctx, sctx);
    EVP_MD_CTX_set_pkey_ctx(md_ctx_verify, sctx);

    if (!TEST_int_gt(EVP_PKEY_CTX_set1_id(sctx, sm2_id, sizeof(sm2_id)), 0))
        goto done;

    if (!TEST_true(EVP_DigestSignInit(md_ctx, NULL, EVP_sm3(), NULL, pkey)))
        goto done;

    if(!TEST_true(EVP_DigestSignUpdate(md_ctx, kMsg, sizeof(kMsg))))
        goto done;

    /* Determine the size of the signature. */
    if (!TEST_true(EVP_DigestSignFinal(md_ctx, NULL, &sig_len)))
        goto done;

    if (!TEST_ptr(sig = OPENSSL_malloc(sig_len)))
        goto done;

    if (!TEST_true(EVP_DigestSignFinal(md_ctx, sig, &sig_len)))
        goto done;

    /* Ensure that the signature round-trips. */

    if (!TEST_true(EVP_DigestVerifyInit(md_ctx_verify, NULL, EVP_sm3(), NULL, pkey)))
        goto done;

    if (!TEST_true(EVP_DigestVerifyUpdate(md_ctx_verify, kMsg, sizeof(kMsg))))
        goto done;

    if (!TEST_true(EVP_DigestVerifyFinal(md_ctx_verify, sig, sig_len)))
        goto done;

    /* now check encryption/decryption */

    if (!TEST_ptr(cctx = EVP_PKEY_CTX_new(pkey, NULL)))
        goto done;

    if (!TEST_true(EVP_PKEY_encrypt_init(cctx)))
        goto done;

    if (!TEST_true(EVP_PKEY_encrypt(cctx, ciphertext, &ctext_len, kMsg, sizeof(kMsg))))
        goto done;

    if (!TEST_true(EVP_PKEY_decrypt_init(cctx)))
        goto done;

    if (!TEST_true(EVP_PKEY_decrypt(cctx, plaintext, &ptext_len, ciphertext, ctext_len)))
        goto done;

    if (!TEST_true(ptext_len == sizeof(kMsg)))
        goto done;

    if (!TEST_true(memcmp(plaintext, kMsg, sizeof(kMsg)) == 0))
        goto done;

    ret = 1;
done:
    EVP_PKEY_CTX_free(pctx);
    EVP_PKEY_CTX_free(kctx);
    EVP_PKEY_CTX_free(sctx);
    EVP_PKEY_CTX_free(cctx);
    EVP_PKEY_free(pkey);
    EVP_PKEY_free(params);
    EVP_MD_CTX_free(md_ctx);
    EVP_MD_CTX_free(md_ctx_verify);
    OPENSSL_free(sig);
    return ret;
}

#endif

static struct keys_st {
    int type;
    char *priv;
    char *pub;
} keys[] = {
    {
        EVP_PKEY_HMAC, "0123456789", NULL
    }, {
        EVP_PKEY_POLY1305, "01234567890123456789012345678901", NULL
    }, {
        EVP_PKEY_SIPHASH, "0123456789012345", NULL
    },
#ifndef OPENSSL_NO_EC
    {
        EVP_PKEY_X25519, "01234567890123456789012345678901",
        "abcdefghijklmnopqrstuvwxyzabcdef"
    }, {
        EVP_PKEY_ED25519, "01234567890123456789012345678901",
        "abcdefghijklmnopqrstuvwxyzabcdef"
    }, {
        EVP_PKEY_X448,
        "01234567890123456789012345678901234567890123456789012345",
        "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcd"
    }, {
        EVP_PKEY_ED448,
        "012345678901234567890123456789012345678901234567890123456",
        "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcde"
    }
#endif
};

static int test_set_get_raw_keys_int(int tst, int pub, int uselibctx)
{
    int ret = 0;
    unsigned char buf[80];
    unsigned char *in;
    size_t inlen, len = 0;
    EVP_PKEY *pkey;

    /* Check if this algorithm supports public keys */
    if (keys[tst].pub == NULL)
        return 1;

    memset(buf, 0, sizeof(buf));

    if (pub) {
        inlen = strlen(keys[tst].pub);
        in = (unsigned char *)keys[tst].pub;
        if (uselibctx) {
            pkey = EVP_PKEY_new_raw_public_key_with_libctx(
                        testctx,
                        OBJ_nid2sn(keys[tst].type),
                        NULL,
                        in,
                        inlen);
        } else {
            pkey = EVP_PKEY_new_raw_public_key(keys[tst].type,
                                               NULL,
                                               in,
                                               inlen);
        }
    } else {
        inlen = strlen(keys[tst].priv);
        in = (unsigned char *)keys[tst].priv;
        if (uselibctx) {
            pkey = EVP_PKEY_new_raw_private_key_with_libctx(
                        testctx, OBJ_nid2sn(keys[tst].type),
                        NULL,
                        in,
                        inlen);
        } else {
            pkey = EVP_PKEY_new_raw_private_key(keys[tst].type,
                                                NULL,
                                                in,
                                                inlen);
        }
    }

    if (!TEST_ptr(pkey)
            || (!pub && !TEST_true(EVP_PKEY_get_raw_private_key(pkey, NULL, &len)))
            || (pub && !TEST_true(EVP_PKEY_get_raw_public_key(pkey, NULL, &len)))
            || !TEST_true(len == inlen)
            || (!pub && !TEST_true(EVP_PKEY_get_raw_private_key(pkey, buf, &len)))
            || (pub && !TEST_true(EVP_PKEY_get_raw_public_key(pkey, buf, &len)))
            || !TEST_mem_eq(in, inlen, buf, len))
        goto done;

    ret = 1;
 done:
    EVP_PKEY_free(pkey);
    return ret;
}

static int test_set_get_raw_keys(int tst)
{
    return test_set_get_raw_keys_int(tst, 0, 0)
           && test_set_get_raw_keys_int(tst, 0, 1)
           && test_set_get_raw_keys_int(tst, 1, 0)
           && test_set_get_raw_keys_int(tst, 1, 1);
}

#ifndef OPENSSL_NO_DEPRECATED_3_0
static int pkey_custom_check(EVP_PKEY *pkey)
{
    return 0xbeef;
}

static int pkey_custom_pub_check(EVP_PKEY *pkey)
{
    return 0xbeef;
}

static int pkey_custom_param_check(EVP_PKEY *pkey)
{
    return 0xbeef;
}

static EVP_PKEY_METHOD *custom_pmeth;
#endif

static int test_EVP_PKEY_check(int i)
{
    int ret = 0;
    const unsigned char *p;
    EVP_PKEY *pkey = NULL;
#ifndef OPENSSL_NO_EC
    EC_KEY *eckey = NULL;
#endif
    EVP_PKEY_CTX *ctx = NULL;
#ifndef OPENSSL_NO_DEPRECATED_3_0
    EVP_PKEY_CTX *ctx2 = NULL;
#endif
    const APK_DATA *ak = &keycheckdata[i];
    const unsigned char *input = ak->kder;
    size_t input_len = ak->size;
    int expected_id = ak->evptype;
    int expected_check = ak->check;
    int expected_pub_check = ak->pub_check;
    int expected_param_check = ak->param_check;
    int type = ak->type;
    BIO *pubkey = NULL;

    p = input;

    switch (type) {
    case 0:
        if (!TEST_ptr(pkey = d2i_AutoPrivateKey(NULL, &p, input_len))
            || !TEST_ptr_eq(p, input + input_len)
            || !TEST_int_eq(EVP_PKEY_id(pkey), expected_id))
            goto done;
        break;
#ifndef OPENSSL_NO_EC
    case 1:
        if (!TEST_ptr(pubkey = BIO_new_mem_buf(input, input_len))
            || !TEST_ptr(eckey = d2i_EC_PUBKEY_bio(pubkey, NULL))
            || !TEST_ptr(pkey = EVP_PKEY_new())
            || !TEST_true(EVP_PKEY_assign_EC_KEY(pkey, eckey)))
            goto done;
        break;
    case 2:
        if (!TEST_ptr(eckey = d2i_ECParameters(NULL, &p, input_len))
            || !TEST_ptr_eq(p, input + input_len)
            || !TEST_ptr(pkey = EVP_PKEY_new())
            || !TEST_true(EVP_PKEY_assign_EC_KEY(pkey, eckey)))
            goto done;
        break;
#endif
    default:
        return 0;
    }

    if (!TEST_ptr(ctx = EVP_PKEY_CTX_new(pkey, NULL)))
        goto done;

    if (!TEST_int_eq(EVP_PKEY_check(ctx), expected_check))
        goto done;

    if (!TEST_int_eq(EVP_PKEY_public_check(ctx), expected_pub_check))
        goto done;

    if (!TEST_int_eq(EVP_PKEY_param_check(ctx), expected_param_check))
        goto done;

#ifndef OPENSSL_NO_DEPRECATED_3_0
    ctx2 = EVP_PKEY_CTX_new_id(0xdefaced, NULL);
    /* assign the pkey directly, as an internal test */
    EVP_PKEY_up_ref(pkey);
    ctx2->pkey = pkey;

    if (!TEST_int_eq(EVP_PKEY_check(ctx2), 0xbeef))
        goto done;

    if (!TEST_int_eq(EVP_PKEY_public_check(ctx2), 0xbeef))
        goto done;

    if (!TEST_int_eq(EVP_PKEY_param_check(ctx2), 0xbeef))
        goto done;
#endif

    ret = 1;

 done:
    EVP_PKEY_CTX_free(ctx);
#ifndef OPENSSL_NO_DEPRECATED_3_0
    EVP_PKEY_CTX_free(ctx2);
#endif
    EVP_PKEY_free(pkey);
    BIO_free(pubkey);
    return ret;
}

#ifndef OPENSSL_NO_CMAC
static int get_cmac_val(EVP_PKEY *pkey, unsigned char *mac)
{
    EVP_MD_CTX *mdctx = EVP_MD_CTX_new();
    const char msg[] = "Hello World";
    size_t maclen;
    int ret = 1;

    if (!TEST_ptr(mdctx)
            || !TEST_true(EVP_DigestSignInit(mdctx, NULL, NULL, NULL, pkey))
            || !TEST_true(EVP_DigestSignUpdate(mdctx, msg, sizeof(msg)))
            || !TEST_true(EVP_DigestSignFinal(mdctx, mac, &maclen))
            || !TEST_size_t_eq(maclen, AES_BLOCK_SIZE))
        ret = 0;

    EVP_MD_CTX_free(mdctx);

    return ret;
}
static int test_CMAC_keygen(void)
{
    static unsigned char key[] = {
        0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b,
        0x0c, 0x0d, 0x0e, 0x0f, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
        0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f
    };
    /*
     * This is a legacy method for CMACs, but should still work.
     * This verifies that it works without an ENGINE.
     */
    EVP_PKEY_CTX *kctx = EVP_PKEY_CTX_new_id(EVP_PKEY_CMAC, NULL);
    int ret = 0;
    EVP_PKEY *pkey = NULL;
    unsigned char mac[AES_BLOCK_SIZE], mac2[AES_BLOCK_SIZE];

    /* Test a CMAC key created using the "generated" method */
    if (!TEST_int_gt(EVP_PKEY_keygen_init(kctx), 0)
            || !TEST_int_gt(EVP_PKEY_CTX_ctrl(kctx, -1, EVP_PKEY_OP_KEYGEN,
                                            EVP_PKEY_CTRL_CIPHER,
                                            0, (void *)EVP_aes_256_ecb()), 0)
            || !TEST_int_gt(EVP_PKEY_CTX_ctrl(kctx, -1, EVP_PKEY_OP_KEYGEN,
                                            EVP_PKEY_CTRL_SET_MAC_KEY,
                                            sizeof(key), (void *)key), 0)
            || !TEST_int_gt(EVP_PKEY_keygen(kctx, &pkey), 0)
            || !TEST_ptr(pkey)
            || !TEST_true(get_cmac_val(pkey, mac)))
        goto done;

    EVP_PKEY_free(pkey);

    /*
     * Test a CMAC key using the direct method, and compare with the mac
     * created above.
     */
    pkey = EVP_PKEY_new_CMAC_key(NULL, key, sizeof(key), EVP_aes_256_ecb());
    if (!TEST_ptr(pkey)
            || !TEST_true(get_cmac_val(pkey, mac2))
            || !TEST_mem_eq(mac, sizeof(mac), mac2, sizeof(mac2)))
        goto done;

    ret = 1;

 done:
    EVP_PKEY_free(pkey);
    EVP_PKEY_CTX_free(kctx);
    return ret;
}
#endif

static int test_HKDF(void)
{
    EVP_PKEY_CTX *pctx;
    unsigned char out[20];
    size_t outlen;
    int i, ret = 0;
    unsigned char salt[] = "0123456789";
    unsigned char key[] = "012345678901234567890123456789";
    unsigned char info[] = "infostring";
    const unsigned char expected[] = {
        0xe5, 0x07, 0x70, 0x7f, 0xc6, 0x78, 0xd6, 0x54, 0x32, 0x5f, 0x7e, 0xc5,
        0x7b, 0x59, 0x3e, 0xd8, 0x03, 0x6b, 0xed, 0xca
    };
    size_t expectedlen = sizeof(expected);

    if (!TEST_ptr(pctx = EVP_PKEY_CTX_new_id(EVP_PKEY_HKDF, NULL)))
        goto done;

    /* We do this twice to test reuse of the EVP_PKEY_CTX */
    for (i = 0; i < 2; i++) {
        outlen = sizeof(out);
        memset(out, 0, outlen);

        if (!TEST_int_gt(EVP_PKEY_derive_init(pctx), 0)
                || !TEST_int_gt(EVP_PKEY_CTX_set_hkdf_md(pctx, EVP_sha256()), 0)
                || !TEST_int_gt(EVP_PKEY_CTX_set1_hkdf_salt(pctx, salt,
                                                            sizeof(salt) - 1), 0)
                || !TEST_int_gt(EVP_PKEY_CTX_set1_hkdf_key(pctx, key,
                                                           sizeof(key) - 1), 0)
                || !TEST_int_gt(EVP_PKEY_CTX_add1_hkdf_info(pctx, info,
                                                            sizeof(info) - 1), 0)
                || !TEST_int_gt(EVP_PKEY_derive(pctx, out, &outlen), 0)
                || !TEST_mem_eq(out, outlen, expected, expectedlen))
            goto done;
    }

    ret = 1;

 done:
    EVP_PKEY_CTX_free(pctx);

    return ret;
}

static int test_emptyikm_HKDF(void)
{
    EVP_PKEY_CTX *pctx;
    unsigned char out[20];
    size_t outlen;
    int ret = 0;
    unsigned char salt[] = "9876543210";
    unsigned char key[] = "";
    unsigned char info[] = "stringinfo";
    const unsigned char expected[] = {
        0x68, 0x81, 0xa5, 0x3e, 0x5b, 0x9c, 0x7b, 0x6f, 0x2e, 0xec, 0xc8, 0x47,
        0x7c, 0xfa, 0x47, 0x35, 0x66, 0x82, 0x15, 0x30
    };
    size_t expectedlen = sizeof(expected);

    if (!TEST_ptr(pctx = EVP_PKEY_CTX_new_id(EVP_PKEY_HKDF, NULL)))
        goto done;

    outlen = sizeof(out);
    memset(out, 0, outlen);

    if (!TEST_int_gt(EVP_PKEY_derive_init(pctx), 0)
            || !TEST_int_gt(EVP_PKEY_CTX_set_hkdf_md(pctx, EVP_sha256()), 0)
            || !TEST_int_gt(EVP_PKEY_CTX_set1_hkdf_salt(pctx, salt,
                                                        sizeof(salt) - 1), 0)
            || !TEST_int_gt(EVP_PKEY_CTX_set1_hkdf_key(pctx, key,
                                                       sizeof(key) - 1), 0)
            || !TEST_int_gt(EVP_PKEY_CTX_add1_hkdf_info(pctx, info,
                                                        sizeof(info) - 1), 0)
            || !TEST_int_gt(EVP_PKEY_derive(pctx, out, &outlen), 0)
            || !TEST_mem_eq(out, outlen, expected, expectedlen))
        goto done;

    ret = 1;

 done:
    EVP_PKEY_CTX_free(pctx);

    return ret;
}

#ifndef OPENSSL_NO_EC
static int test_X509_PUBKEY_inplace(void)
{
  int ret = 0;
  X509_PUBKEY *xp = NULL;
  const unsigned char *p = kExampleECPubKeyDER;
  size_t input_len = sizeof(kExampleECPubKeyDER);

  if (!TEST_ptr(xp = d2i_X509_PUBKEY(NULL, &p, input_len)))
    goto done;

  if (!TEST_ptr(X509_PUBKEY_get0(xp)))
    goto done;

  p = kExampleBadECPubKeyDER;
  input_len = sizeof(kExampleBadECPubKeyDER);

  if (!TEST_ptr(xp = d2i_X509_PUBKEY(&xp, &p, input_len)))
    goto done;

  if (!TEST_true(X509_PUBKEY_get0(xp) == NULL))
    goto done;

  ret = 1;

done:
  X509_PUBKEY_free(xp);
  return ret;
}
#endif /* OPENSSL_NO_EC */

/* Test getting and setting parameters on an EVP_PKEY_CTX */
static int test_EVP_PKEY_CTX_get_set_params(EVP_PKEY *pkey)
{
    EVP_MD_CTX *mdctx = NULL;
    EVP_PKEY_CTX *ctx = NULL;
    const OSSL_PARAM *params;
    OSSL_PARAM ourparams[2], *param = ourparams, *param_md;
    int ret = 0;
    const EVP_MD *md;
    char mdname[OSSL_MAX_NAME_SIZE];
    char ssl3ms[48];

    /* Initialise a sign operation */
    ctx = EVP_PKEY_CTX_new(pkey, NULL);
    if (!TEST_ptr(ctx)
            || !TEST_int_gt(EVP_PKEY_sign_init(ctx), 0))
        goto err;

    /*
     * We should be able to query the parameters now.
     */
    params = EVP_PKEY_CTX_settable_params(ctx);
    if (!TEST_ptr(params)
        || !TEST_ptr(OSSL_PARAM_locate_const(params,
                                             OSSL_SIGNATURE_PARAM_DIGEST)))
        goto err;

    params = EVP_PKEY_CTX_gettable_params(ctx);
    if (!TEST_ptr(params)
        || !TEST_ptr(OSSL_PARAM_locate_const(params,
                                             OSSL_SIGNATURE_PARAM_ALGORITHM_ID))
        || !TEST_ptr(OSSL_PARAM_locate_const(params,
                                             OSSL_SIGNATURE_PARAM_DIGEST)))
        goto err;

    /*
     * Test getting and setting params via EVP_PKEY_CTX_set_params() and
     * EVP_PKEY_CTX_get_params()
     */
    strcpy(mdname, "SHA512");
    param_md = param;
    *param++ = OSSL_PARAM_construct_utf8_string(OSSL_SIGNATURE_PARAM_DIGEST,
                                                mdname, 0);
    *param++ = OSSL_PARAM_construct_end();

    if (!TEST_true(EVP_PKEY_CTX_set_params(ctx, ourparams)))
        goto err;

    mdname[0] = '\0';
    *param_md = OSSL_PARAM_construct_utf8_string(OSSL_SIGNATURE_PARAM_DIGEST,
                                                 mdname, sizeof(mdname));
    if (!TEST_true(EVP_PKEY_CTX_get_params(ctx, ourparams))
            || !TEST_str_eq(mdname, "SHA512"))
        goto err;

    /*
     * Test the TEST_PKEY_CTX_set_signature_md() and
     * TEST_PKEY_CTX_get_signature_md() functions
     */
    if (!TEST_int_gt(EVP_PKEY_CTX_set_signature_md(ctx, EVP_sha256()), 0)
            || !TEST_int_gt(EVP_PKEY_CTX_get_signature_md(ctx, &md), 0)
            || !TEST_ptr_eq(md, EVP_sha256()))
        goto err;

    /*
     * Test getting MD parameters via an associated EVP_PKEY_CTX
     */
    mdctx = EVP_MD_CTX_new();
    if (!TEST_ptr(mdctx)
        || !TEST_true(EVP_DigestSignInit_with_libctx(mdctx, NULL,
                                                     "SHA1", NULL, NULL,
                                                     pkey)))
        goto err;

    /*
     * We now have an EVP_MD_CTX with an EVP_PKEY_CTX inside it. We should be
     * able to obtain the digest's settable parameters from the provider.
     */
    params = EVP_MD_CTX_settable_params(mdctx);
    if (!TEST_ptr(params)
            || !TEST_int_eq(strcmp(params[0].key, OSSL_DIGEST_PARAM_SSL3_MS), 0)
               /* The final key should be NULL */
            || !TEST_ptr_null(params[1].key))
        goto err;

    param = ourparams;
    memset(ssl3ms, 0, sizeof(ssl3ms));
    *param++ = OSSL_PARAM_construct_octet_string(OSSL_DIGEST_PARAM_SSL3_MS,
                                                 ssl3ms, sizeof(ssl3ms));
    *param++ = OSSL_PARAM_construct_end();

    if (!TEST_true(EVP_MD_CTX_set_params(mdctx, ourparams)))
        goto err;

    ret = 1;

 err:
    EVP_MD_CTX_free(mdctx);
    EVP_PKEY_CTX_free(ctx);

    return ret;
}

#ifndef OPENSSL_NO_DSA
static int test_DSA_get_set_params(void)
{
    DSA *dsa = NULL;
    BIGNUM *p = NULL, *q = NULL, *g = NULL, *pub = NULL, *priv = NULL;
    EVP_PKEY *pkey = NULL;
    int ret = 0;

    /*
     * Setup the parameters for our DSA object. For our purposes they don't
     * have to actually be *valid* parameters. We just need to set something.
     */
    dsa = DSA_new();
    p = BN_new();
    q = BN_new();
    g = BN_new();
    pub = BN_new();
    priv = BN_new();
    if (!TEST_ptr(dsa)
            || !TEST_ptr(p)
            || !TEST_ptr(q)
            || !TEST_ptr(g)
            || !TEST_ptr(pub)
            || !DSA_set0_pqg(dsa, p, q, g)
        || !DSA_set0_key(dsa, pub, priv))
        goto err;
    p = q = g = pub = priv = NULL;

    pkey = EVP_PKEY_new();
    if (!TEST_ptr(pkey)
            || !TEST_true(EVP_PKEY_assign_DSA(pkey, dsa)))
        goto err;

    dsa = NULL;

    ret = test_EVP_PKEY_CTX_get_set_params(pkey);

 err:
    EVP_PKEY_free(pkey);
    DSA_free(dsa);
    BN_free(p);
    BN_free(q);
    BN_free(g);
    BN_free(pub);
    BN_free(priv);

    return ret;
}
#endif

static int test_RSA_get_set_params(void)
{
    RSA *rsa = NULL;
    BIGNUM *n = NULL, *e = NULL, *d = NULL;
    EVP_PKEY *pkey = NULL;
    int ret = 0;

    /*
     * Setup the parameters for our RSA object. For our purposes they don't
     * have to actually be *valid* parameters. We just need to set something.
     */
    rsa = RSA_new();
    n = BN_new();
    e = BN_new();
    d = BN_new();
    if (!TEST_ptr(rsa)
            || !TEST_ptr(n)
            || !TEST_ptr(e)
            || !TEST_ptr(d)
        || !RSA_set0_key(rsa, n, e, d))
        goto err;
    n = e = d = NULL;

    pkey = EVP_PKEY_new();
    if (!TEST_ptr(pkey)
            || !TEST_true(EVP_PKEY_assign_RSA(pkey, rsa)))
        goto err;

    rsa = NULL;

    ret = test_EVP_PKEY_CTX_get_set_params(pkey);

 err:
    EVP_PKEY_free(pkey);
    RSA_free(rsa);
    BN_free(n);
    BN_free(e);
    BN_free(d);

    return ret;
}

#if !defined(OPENSSL_NO_CHACHA) && !defined(OPENSSL_NO_POLY1305)
static int test_decrypt_null_chunks(void)
{
    EVP_CIPHER_CTX* ctx = NULL;
    const unsigned char key[32] = {
        0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b,
        0x0c, 0x0d, 0x0e, 0x0f, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
        0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1
    };
    unsigned char iv[12] = {
        0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b
    };
    unsigned char msg[] = "It was the best of times, it was the worst of times";
    unsigned char ciphertext[80];
    unsigned char plaintext[80];
    /* We initialise tmp to a non zero value on purpose */
    int ctlen, ptlen, tmp = 99;
    int ret = 0;
    const int enc_offset = 10, dec_offset = 20;

    if (!TEST_ptr(ctx = EVP_CIPHER_CTX_new())
            || !TEST_true(EVP_EncryptInit_ex(ctx, EVP_chacha20_poly1305(), NULL,
                                             key, iv))
            || !TEST_true(EVP_EncryptUpdate(ctx, ciphertext, &ctlen, msg,
                                            enc_offset))
            /* Deliberate add a zero length update */
            || !TEST_true(EVP_EncryptUpdate(ctx, ciphertext + ctlen, &tmp, NULL,
                                            0))
            || !TEST_int_eq(tmp, 0)
            || !TEST_true(EVP_EncryptUpdate(ctx, ciphertext + ctlen, &tmp,
                                            msg + enc_offset,
                                            sizeof(msg) - enc_offset))
            || !TEST_int_eq(ctlen += tmp, sizeof(msg))
            || !TEST_true(EVP_EncryptFinal(ctx, ciphertext + ctlen, &tmp))
            || !TEST_int_eq(tmp, 0))
        goto err;

    /* Deliberately initialise tmp to a non zero value */
    tmp = 99;
    if (!TEST_true(EVP_DecryptInit_ex(ctx, EVP_chacha20_poly1305(), NULL, key,
                                      iv))
            || !TEST_true(EVP_DecryptUpdate(ctx, plaintext, &ptlen, ciphertext,
                                            dec_offset))
            /*
             * Deliberately add a zero length update. We also deliberately do
             * this at a different offset than for encryption.
             */
            || !TEST_true(EVP_DecryptUpdate(ctx, plaintext + ptlen, &tmp, NULL,
                                            0))
            || !TEST_int_eq(tmp, 0)
            || !TEST_true(EVP_DecryptUpdate(ctx, plaintext + ptlen, &tmp,
                                            ciphertext + dec_offset,
                                            ctlen - dec_offset))
            || !TEST_int_eq(ptlen += tmp, sizeof(msg))
            || !TEST_true(EVP_DecryptFinal(ctx, plaintext + ptlen, &tmp))
            || !TEST_int_eq(tmp, 0)
            || !TEST_mem_eq(msg, sizeof(msg), plaintext, ptlen))
        goto err;

    ret = 1;
 err:
    EVP_CIPHER_CTX_free(ctx);
    return ret;
}
#endif /* !defined(OPENSSL_NO_CHACHA) && !defined(OPENSSL_NO_POLY1305) */

#ifndef OPENSSL_NO_DH
static int test_EVP_PKEY_set1_DH(void)
{
    DH *x942dh = NULL, *noqdh = NULL;
    EVP_PKEY *pkey1 = NULL, *pkey2 = NULL;
    int ret = 0;
    BIGNUM *p, *g = NULL;

    if (!TEST_ptr(p = BN_new())
            || !TEST_ptr(g = BN_new())
            || !BN_set_word(p, 9999)
            || !BN_set_word(g, 2)
            || !TEST_ptr(noqdh = DH_new())
            || !DH_set0_pqg(noqdh, p, NULL, g))
        goto err;
    p = g = NULL;

    x942dh = DH_get_2048_256();
    pkey1 = EVP_PKEY_new();
    pkey2 = EVP_PKEY_new();
    if (!TEST_ptr(x942dh)
            || !TEST_ptr(noqdh)
            || !TEST_ptr(pkey1)
            || !TEST_ptr(pkey2))
        goto err;

    if(!TEST_true(EVP_PKEY_set1_DH(pkey1, x942dh))
            || !TEST_int_eq(EVP_PKEY_id(pkey1), EVP_PKEY_DHX))
        goto err;

    if(!TEST_true(EVP_PKEY_set1_DH(pkey2, noqdh))
            || !TEST_int_eq(EVP_PKEY_id(pkey2), EVP_PKEY_DH))
        goto err;

    ret = 1;
 err:
    BN_free(p);
    BN_free(g);
    EVP_PKEY_free(pkey1);
    EVP_PKEY_free(pkey2);
    DH_free(x942dh);
    DH_free(noqdh);

    return ret;
}
#endif

/*
 * We test what happens with an empty template.  For the sake of this test,
 * the template must be ignored, and we know that's the case for RSA keys
 * (this might arguably be a misfeature, but that's what we currently do,
 * even in provider code, since that's how the legacy RSA implementation
 * does things)
 */
static int test_keygen_with_empty_template(int n)
{
    EVP_PKEY_CTX *ctx = NULL;
    EVP_PKEY *pkey = NULL;
    EVP_PKEY *tkey = NULL;
    int ret = 0;

    switch (n) {
    case 0:
        /* We do test with no template at all as well */
        if (!TEST_ptr(ctx = EVP_PKEY_CTX_new_id(EVP_PKEY_RSA, NULL)))
            goto err;
        break;
    case 1:
        /* Here we create an empty RSA key that serves as our template */
        if (!TEST_ptr(tkey = EVP_PKEY_new())
            || !TEST_true(EVP_PKEY_set_type(tkey, EVP_PKEY_RSA))
            || !TEST_ptr(ctx = EVP_PKEY_CTX_new(tkey, NULL)))
            goto err;
        break;
    }

    if (!TEST_int_gt(EVP_PKEY_keygen_init(ctx), 0)
        || !TEST_int_gt(EVP_PKEY_keygen(ctx, &pkey), 0))
        goto err;

    ret = 1;
 err:
    EVP_PKEY_CTX_free(ctx);
    EVP_PKEY_free(pkey);
    EVP_PKEY_free(tkey);
    return ret;
}

/*
 * Test that we fail if we attempt to use an algorithm that is not available
 * in the current library context (unless we are using an algorithm that
 * should be made available via legacy codepaths).
 *
 * 0:   RSA
 * 1:   SM2
 */
static int test_pkey_ctx_fail_without_provider(int tst)
{
    OPENSSL_CTX *tmpctx = OPENSSL_CTX_new();
    OSSL_PROVIDER *nullprov = NULL;
    EVP_PKEY_CTX *pctx = NULL;
    const char *keytype = NULL;
    int expect_null = 0;
    int ret = 0;

    if (!TEST_ptr(tmpctx))
        goto err;

    nullprov = OSSL_PROVIDER_load(tmpctx, "null");
    if (!TEST_ptr(nullprov))
        goto err;

    /*
     * We check for certain algos in the null provider.
     * If an algo is expected to have a provider keymgmt, contructing an
     * EVP_PKEY_CTX is expected to fail (return NULL).
     * Otherwise, if it's expected to have legacy support, contructing an
     * EVP_PKEY_CTX is expected to succeed (return non-NULL).
     */
    switch (tst) {
    case 0:
        keytype = "RSA";
        expect_null = 1;
        break;
    case 1:
        keytype = "SM2";
        expect_null = 0; /* TODO: change to 1 when we have a SM2 keymgmt */
#ifdef OPENSSL_NO_EC
        TEST_info("EC disable, skipping SM2 check...");
        goto end;
#endif
#ifdef OPENSSL_NO_SM2
        TEST_info("SM2 disable, skipping SM2 check...");
        goto end;
#endif
        break;
    default:
        TEST_error("No test for case %d", tst);
        goto err;
    }

    pctx = EVP_PKEY_CTX_new_from_name(tmpctx, keytype, "");
    if (expect_null ? !TEST_ptr_null(pctx) : !TEST_ptr(pctx))
        goto err;

#if defined(OPENSSL_NO_EC) || defined(OPENSSL_NO_SM2)
 end:
#endif
    ret = 1;

 err:
    EVP_PKEY_CTX_free(pctx);
    OSSL_PROVIDER_unload(nullprov);
    OPENSSL_CTX_free(tmpctx);
    return ret;
}

static int test_rand_agglomeration(void)
{
    EVP_RAND *rand;
    EVP_RAND_CTX *ctx;
    OSSL_PARAM params[3], *p = params;
    int res;
    unsigned int step = 7;
    static unsigned char seed[] = "It does not matter how slowly you go "
                                  "as long as you do not stop.";
    unsigned char out[sizeof(seed)];

    if (!TEST_int_ne(sizeof(seed) % step, 0)
            || !TEST_ptr(rand = EVP_RAND_fetch(NULL, "TEST-RAND", NULL)))
        return 0;
    ctx = EVP_RAND_CTX_new(rand, NULL);
    EVP_RAND_free(rand);
    if (!TEST_ptr(ctx))
        return 0;

    memset(out, 0, sizeof(out));
    *p++ = OSSL_PARAM_construct_octet_string(OSSL_RAND_PARAM_TEST_ENTROPY,
                                             seed, sizeof(seed));
    *p++ = OSSL_PARAM_construct_uint(OSSL_DRBG_PARAM_MAX_REQUEST, &step);
    *p = OSSL_PARAM_construct_end();
    res = TEST_true(EVP_RAND_set_ctx_params(ctx, params))
          && TEST_true(EVP_RAND_generate(ctx, out, sizeof(out), 0, 1, NULL, 0))
          && TEST_mem_eq(seed, sizeof(seed), out, sizeof(out));
    EVP_RAND_CTX_free(ctx);
    return res;
}

/*
 * Test that we correctly return the original or "running" IV after
 * an encryption operation.
 * Run multiple times for some different relevant algorithms/modes.
 */
static int test_evp_iv(int idx)
{
    int ret = 0;
    EVP_CIPHER_CTX *ctx = NULL;
    unsigned char key[16] = {0x4c, 0x43, 0xdb, 0xdd, 0x42, 0x73, 0x47, 0xd1,
                             0xe5, 0x62, 0x7d, 0xcd, 0x4d, 0x76, 0x4d, 0x57};
    unsigned char init_iv[EVP_MAX_IV_LENGTH] =
        {0x57, 0x71, 0x7d, 0xad, 0xdb, 0x9b, 0x98, 0x82,
         0x5a, 0x55, 0x91, 0x81, 0x42, 0xa8, 0x89, 0x34};
    static const unsigned char msg[] = { 1, 2, 3, 4, 5, 6, 7, 8,
                                         9, 10, 11, 12, 13, 14, 15, 16 };
    unsigned char ciphertext[32], oiv[16], iv[16];
    unsigned char *ref_iv;
    unsigned char cbc_state[16] = {0x10, 0x2f, 0x05, 0xcc, 0xc2, 0x55, 0x72, 0xb9,
                                   0x88, 0xe6, 0x4a, 0x17, 0x10, 0x74, 0x22, 0x5e};

    unsigned char ofb_state[16] = {0x76, 0xe6, 0x66, 0x61, 0xd0, 0x8a, 0xe4, 0x64,
                                   0xdd, 0x66, 0xbf, 0x00, 0xf0, 0xe3, 0x6f, 0xfd};
    unsigned char gcm_state[12] = {0x57, 0x71, 0x7d, 0xad, 0xdb, 0x9b,
                                   0x98, 0x82, 0x5a, 0x55, 0x91, 0x81};
    unsigned char ccm_state[7] = {0x57, 0x71, 0x7d, 0xad, 0xdb, 0x9b, 0x98};
#ifndef OPENSSL_NO_OCB
    unsigned char ocb_state[12] = {0x57, 0x71, 0x7d, 0xad, 0xdb, 0x9b,
                                   0x98, 0x82, 0x5a, 0x55, 0x91, 0x81};
#endif
    int len = sizeof(ciphertext);
    size_t ivlen, ref_len;
    const EVP_CIPHER *type = NULL;

    switch(idx) {
    case 0:
        type = EVP_aes_128_cbc();
        /* FALLTHROUGH */
    case 5:
        type = (type != NULL) ? type :
                                EVP_CIPHER_fetch(testctx, "aes-128-cbc", NULL);
        ref_iv = cbc_state;
        ref_len = sizeof(cbc_state);
        break;
    case 1:
        type = EVP_aes_128_ofb();
        /* FALLTHROUGH */
    case 6:
        type = (type != NULL) ? type :
                                EVP_CIPHER_fetch(testctx, "aes-128-ofb", NULL);
        ref_iv = ofb_state;
        ref_len = sizeof(ofb_state);
        break;
    case 2:
        type = EVP_aes_128_gcm();
        /* FALLTHROUGH */
    case 7:
        type = (type != NULL) ? type :
                                EVP_CIPHER_fetch(testctx, "aes-128-gcm", NULL);
        ref_iv = gcm_state;
        ref_len = sizeof(gcm_state);
        break;
    case 3:
        type = EVP_aes_128_ccm();
        /* FALLTHROUGH */
    case 8:
        type = (type != NULL) ? type :
                                EVP_CIPHER_fetch(testctx, "aes-128-ccm", NULL);
        ref_iv = ccm_state;
        ref_len = sizeof(ccm_state);
        break;
#ifdef OPENSSL_NO_OCB
    case 4:
    case 9:
        return 1;
#else
    case 4:
        type = EVP_aes_128_ocb();
        /* FALLTHROUGH */
    case 9:
        type = (type != NULL) ? type :
                                EVP_CIPHER_fetch(testctx, "aes-128-ocb", NULL);
        ref_iv = ocb_state;
        ref_len = sizeof(ocb_state);
        break;
#endif
    default:
        return 0;
    }

    if (!TEST_ptr(type)
            || !TEST_ptr((ctx = EVP_CIPHER_CTX_new()))
            || !TEST_true(EVP_EncryptInit_ex(ctx, type, NULL, key, init_iv))
            || !TEST_true(EVP_EncryptUpdate(ctx, ciphertext, &len, msg,
                          (int)sizeof(msg)))
            || !TEST_true(EVP_CIPHER_CTX_get_iv(ctx, oiv, sizeof(oiv)))
            || !TEST_true(EVP_CIPHER_CTX_get_iv_state(ctx, iv, sizeof(iv)))
            || !TEST_true(EVP_EncryptFinal_ex(ctx, ciphertext, &len)))
        goto err;
    ivlen = EVP_CIPHER_CTX_iv_length(ctx);
    if (!TEST_mem_eq(init_iv, ivlen, oiv, ivlen)
            || !TEST_mem_eq(ref_iv, ref_len, iv, ivlen))
        goto err;

    ret = 1;
err:
    EVP_CIPHER_CTX_free(ctx);
    if (idx >= 5)
        EVP_CIPHER_free((EVP_CIPHER *)type);
    return ret;
}

int setup_tests(void)
{
    testctx = OPENSSL_CTX_new();

    if (!TEST_ptr(testctx))
        return 0;

    ADD_TEST(test_EVP_set_default_properties);
    ADD_ALL_TESTS(test_EVP_DigestSignInit, 9);
    ADD_TEST(test_EVP_DigestVerifyInit);
    ADD_TEST(test_EVP_Enveloped);
    ADD_ALL_TESTS(test_d2i_AutoPrivateKey, OSSL_NELEM(keydata));
    ADD_TEST(test_privatekey_to_pkcs8);
#ifndef OPENSSL_NO_EC
    ADD_TEST(test_EVP_PKCS82PKEY);
#endif
#ifndef OPENSSL_NO_EC
    ADD_ALL_TESTS(test_EC_keygen_with_enc, OSSL_NELEM(ec_encodings));
#endif
#if !defined(OPENSSL_NO_SM2) && !defined(FIPS_MODULE)
    ADD_TEST(test_EVP_SM2);
    ADD_TEST(test_EVP_SM2_verify);
#endif
    ADD_ALL_TESTS(test_set_get_raw_keys, OSSL_NELEM(keys));
#ifndef OPENSSL_NO_DEPRECATED_3_0
    custom_pmeth = EVP_PKEY_meth_new(0xdefaced, 0);
    if (!TEST_ptr(custom_pmeth))
        return 0;
    EVP_PKEY_meth_set_check(custom_pmeth, pkey_custom_check);
    EVP_PKEY_meth_set_public_check(custom_pmeth, pkey_custom_pub_check);
    EVP_PKEY_meth_set_param_check(custom_pmeth, pkey_custom_param_check);
    if (!TEST_int_eq(EVP_PKEY_meth_add0(custom_pmeth), 1))
        return 0;
#endif
    ADD_ALL_TESTS(test_EVP_PKEY_check, OSSL_NELEM(keycheckdata));
#ifndef OPENSSL_NO_CMAC
    ADD_TEST(test_CMAC_keygen);
#endif
    ADD_TEST(test_HKDF);
    ADD_TEST(test_emptyikm_HKDF);
#ifndef OPENSSL_NO_EC
    ADD_TEST(test_X509_PUBKEY_inplace);
    ADD_ALL_TESTS(test_invalide_ec_char2_pub_range_decode,
                  OSSL_NELEM(ec_der_pub_keys));
#endif
#ifndef OPENSSL_NO_DSA
    ADD_TEST(test_DSA_get_set_params);
#endif
    ADD_TEST(test_RSA_get_set_params);
#if !defined(OPENSSL_NO_CHACHA) && !defined(OPENSSL_NO_POLY1305)
    ADD_TEST(test_decrypt_null_chunks);
#endif
#ifndef OPENSSL_NO_DH
    ADD_TEST(test_EVP_PKEY_set1_DH);
#endif
    ADD_ALL_TESTS(test_keygen_with_empty_template, 2);
    ADD_ALL_TESTS(test_pkey_ctx_fail_without_provider, 2);

    ADD_TEST(test_rand_agglomeration);
    ADD_ALL_TESTS(test_evp_iv, 10);

    return 1;
}

void cleanup_tests(void)
{
    OPENSSL_CTX_free(testctx);
}
