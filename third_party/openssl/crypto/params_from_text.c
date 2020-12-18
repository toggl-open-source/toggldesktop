/*
 * Copyright 2019-2020 The OpenSSL Project Authors. All Rights Reserved.
 * Copyright (c) 2019, Oracle and/or its affiliates.  All rights reserved.
 *
 * Licensed under the Apache License 2.0 (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://www.openssl.org/source/license.html
 */

#include <string.h>
#include <openssl/err.h>
#include <openssl/params.h>

/*
 * When processing text to params, we're trying to be smart with numbers.
 * Instead of handling each specific separate integer type, we use a bignum
 * and ensure that it isn't larger than the expected size, and we then make
 * sure it is the expected size...  if there is one given.
 * (if the size can be arbitrary, then we give whatever we have)
 */

static int prepare_from_text(const OSSL_PARAM *paramdefs, const char *key,
                             const char *value, size_t value_n,
                             /* Output parameters */
                             const OSSL_PARAM **paramdef, int *ishex,
                             size_t *buf_n, BIGNUM **tmpbn, int *found)
{
    const OSSL_PARAM *p;

    /*
     * ishex is used to translate legacy style string controls in hex format
     * to octet string parameters.
     */
    *ishex = strncmp(key, "hex", 3) == 0;

    if (*ishex)
        key += 3;

    p = *paramdef = OSSL_PARAM_locate_const(paramdefs, key);
    if (found != NULL)
        *found = p != NULL;
    if (p == NULL)
        return 0;

    switch (p->data_type) {
    case OSSL_PARAM_INTEGER:
    case OSSL_PARAM_UNSIGNED_INTEGER:
        if (*ishex)
            BN_hex2bn(tmpbn, value);
        else
            BN_dec2bn(tmpbn, value);

        if (*tmpbn == NULL)
            return 0;

        /*
         * 2s complement negate, part 1
         *
         * BN_bn2nativepad puts the absolute value of the number in the
         * buffer, i.e. if it's negative, we need to deal with it.  We do
         * it by subtracting 1 here and inverting the bytes in
         * construct_from_text() below.
         */
        if (p->data_type == OSSL_PARAM_INTEGER && BN_is_negative(*tmpbn)
            && !BN_sub_word(*tmpbn, 1)) {
            return 0;
        }

        *buf_n = BN_num_bytes(*tmpbn);

        /*
         * TODO(v3.0) is this the right way to do this?  This code expects
         * a zero data size to simply mean "arbitrary size".
         */
        if (p->data_size > 0) {
            if (*buf_n >= p->data_size) {
                CRYPTOerr(0, CRYPTO_R_TOO_SMALL_BUFFER);
                /* Since this is a different error, we don't break */
                return 0;
            }
            /* Change actual size to become the desired size. */
            *buf_n = p->data_size;
        }
        break;
    case OSSL_PARAM_UTF8_STRING:
        if (*ishex) {
            CRYPTOerr(0, ERR_R_PASSED_INVALID_ARGUMENT);
            return 0;
        }
        *buf_n = strlen(value) + 1;
        break;
    case OSSL_PARAM_OCTET_STRING:
        if (*ishex) {
            *buf_n = strlen(value) >> 1;
        } else {
            *buf_n = value_n;
        }
        break;
    }

    return 1;
}

static int construct_from_text(OSSL_PARAM *to, const OSSL_PARAM *paramdef,
                               const char *value, size_t value_n, int ishex,
                               void *buf, size_t buf_n, BIGNUM *tmpbn)
{
    if (buf == NULL)
        return 0;

    if (buf_n > 0) {
        switch (paramdef->data_type) {
        case OSSL_PARAM_INTEGER:
        case OSSL_PARAM_UNSIGNED_INTEGER:
            /*
            {
                if ((new_value = OPENSSL_malloc(new_value_n)) == NULL) {
                    BN_free(a);
                    break;
                }
            */

            BN_bn2nativepad(tmpbn, buf, buf_n);

            /*
             * 2s complement negate, part two.
             *
             * Because we did the first part on the BIGNUM itself, we can just
             * invert all the bytes here and be done with it.
             */
            if (paramdef->data_type == OSSL_PARAM_INTEGER
                && BN_is_negative(tmpbn)) {
                unsigned char *cp;
                size_t i = buf_n;

                for (cp = buf; i-- > 0; cp++)
                    *cp ^= 0xFF;
            }
            break;
        case OSSL_PARAM_UTF8_STRING:
            strncpy(buf, value, buf_n);
            break;
        case OSSL_PARAM_OCTET_STRING:
            if (ishex) {
                size_t l = 0;

                if (!OPENSSL_hexstr2buf_ex(buf, buf_n, &l, value))
                    return 0;
            } else {
                memcpy(buf, value, buf_n);
            }
            break;
        }
    }

    *to = *paramdef;
    to->data = buf;
    to->data_size = buf_n;
    to->return_size = OSSL_PARAM_UNMODIFIED;

    return 1;
}

int OSSL_PARAM_allocate_from_text(OSSL_PARAM *to,
                                  const OSSL_PARAM *paramdefs,
                                  const char *key, const char *value,
                                  size_t value_n, int *found)
{
    const OSSL_PARAM *paramdef = NULL;
    int ishex = 0;
    void *buf = NULL;
    size_t buf_n = 0;
    BIGNUM *tmpbn = NULL;
    int ok = 0;

    if (to == NULL || paramdefs == NULL)
        return 0;

    if (!prepare_from_text(paramdefs, key, value, value_n,
                           &paramdef, &ishex, &buf_n, &tmpbn, found))
        return 0;

    if ((buf = OPENSSL_zalloc(buf_n > 0 ? buf_n : 1)) == NULL) {
        CRYPTOerr(0, ERR_R_MALLOC_FAILURE);
        return 0;
    }

    ok = construct_from_text(to, paramdef, value, value_n, ishex,
                             buf, buf_n, tmpbn);
    BN_free(tmpbn);
    if (!ok)
        OPENSSL_free(buf);
    return ok;
}
