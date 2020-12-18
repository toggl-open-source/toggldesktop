/*
 * Generated by util/mkerr.pl DO NOT EDIT
 * Copyright 1995-2020 The OpenSSL Project Authors. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://www.openssl.org/source/license.html
 */

#include <openssl/err.h>
#include <openssl/encodererr.h>

#ifndef OPENSSL_NO_ERR

static const ERR_STRING_DATA OSSL_ENCODER_str_reasons[] = {
    {ERR_PACK(ERR_LIB_OSSL_ENCODER, 0, OSSL_ENCODER_R_INCORRECT_PROPERTY_QUERY),
     "incorrect property query"},
    {ERR_PACK(ERR_LIB_OSSL_ENCODER, 0, OSSL_ENCODER_R_ENCODER_NOT_FOUND),
     "encoder not found"},
    {0, NULL}
};

#endif

int ERR_load_OSSL_ENCODER_strings(void)
{
#ifndef OPENSSL_NO_ERR
    if (ERR_reason_error_string(OSSL_ENCODER_str_reasons[0].error) == NULL)
        ERR_load_strings_const(OSSL_ENCODER_str_reasons);
#endif
    return 1;
}
