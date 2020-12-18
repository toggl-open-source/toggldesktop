/*
 * Copyright 2020 The OpenSSL Project Authors. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://www.openssl.org/source/license.html
 */

#include <openssl/obj_mac.h>
#include "internal/packet.h"
#include "prov/der_rsa.h"
#include "prov/der_digests.h"

/* Aliases so we can have a uniform MD_with_RSA_CASE */
#define der_oid_sha3_224WithRSAEncryption \
    der_oid_id_rsassa_pkcs1_v1_5_with_sha3_224
#define der_oid_sha3_256WithRSAEncryption \
    der_oid_id_rsassa_pkcs1_v1_5_with_sha3_256
#define der_oid_sha3_384WithRSAEncryption \
    der_oid_id_rsassa_pkcs1_v1_5_with_sha3_384
#define der_oid_sha3_512WithRSAEncryption \
    der_oid_id_rsassa_pkcs1_v1_5_with_sha3_512

#define MD_with_RSA_CASE(name, var)                                     \
    case NID_##name:                                                    \
        var = der_oid_##name##WithRSAEncryption;                        \
        var##_sz = sizeof(der_oid_##name##WithRSAEncryption);           \
        break;

int DER_w_algorithmIdentifier_MDWithRSAEncryption(WPACKET *pkt, int tag,
                                                  RSA *rsa, int mdnid)
{
    const unsigned char *precompiled = NULL;
    size_t precompiled_sz = 0;

    switch (mdnid) {
#ifndef FIPS_MODULE
        MD_with_RSA_CASE(md2, precompiled);
        MD_with_RSA_CASE(md5, precompiled);
        MD_with_RSA_CASE(md4, precompiled);
        MD_with_RSA_CASE(ripemd160, precompiled);
/* TODO(3.0) Decide what to do about mdc2 and md5_sha1 */
#endif
        MD_with_RSA_CASE(sha1, precompiled);
        MD_with_RSA_CASE(sha224, precompiled);
        MD_with_RSA_CASE(sha256, precompiled);
        MD_with_RSA_CASE(sha384, precompiled);
        MD_with_RSA_CASE(sha512, precompiled);
        MD_with_RSA_CASE(sha512_224, precompiled);
        MD_with_RSA_CASE(sha512_256, precompiled);
        MD_with_RSA_CASE(sha3_224, precompiled);
        MD_with_RSA_CASE(sha3_256, precompiled);
        MD_with_RSA_CASE(sha3_384, precompiled);
        MD_with_RSA_CASE(sha3_512, precompiled);
    default:
        return 0;
    }

    return DER_w_begin_sequence(pkt, tag)
        /* No parameters (yet?) */
        && DER_w_precompiled(pkt, -1, precompiled, precompiled_sz)
        && DER_w_end_sequence(pkt, tag);
}
