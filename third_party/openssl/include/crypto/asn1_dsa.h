/*
 * Copyright 2019 The OpenSSL Project Authors. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://www.openssl.org/source/license.html
 */

#ifndef OSSL_CRYPTO_ASN1_DSA_H
# define OSSL_CRYPTO_ASN1_DSA_H

#include "internal/packet.h"

int encode_der_length(WPACKET *pkt, size_t cont_len);
int encode_der_integer(WPACKET *pkt, const BIGNUM *n);
int encode_der_dsa_sig(WPACKET *pkt, const BIGNUM *r, const BIGNUM *s);
int decode_der_length(PACKET *pkt, PACKET *subpkt);
int decode_der_integer(PACKET *pkt, BIGNUM *n);
size_t decode_der_dsa_sig(BIGNUM *r, BIGNUM *s, const unsigned char **ppin,
                          size_t len);

#endif
