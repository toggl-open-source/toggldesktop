/*
 * Copyright 2019-2020 The OpenSSL Project Authors. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://www.openssl.org/source/license.html
 */

#include <string.h>
#include <openssl/evp.h>
#include <openssl/params.h>
#include <openssl/crypto.h>
#include <openssl/fipskey.h>
#include <openssl/err.h>
#include "e_os.h"
#include "prov/providercommonerr.h"
#include "prov/providercommon.h"

/*
 * We're cheating here. Normally we don't allow RUN_ONCE usage inside the FIPS
 * module because all such initialisation should be associated with an
 * individual OPENSSL_CTX. That doesn't work with the self test though because
 * it should be run once regardless of the number of OPENSSL_CTXs we have.
 */
#define ALLOW_RUN_ONCE_IN_FIPS
#include <internal/thread_once.h>
#include "self_test.h"

#define FIPS_STATE_INIT     0
#define FIPS_STATE_SELFTEST 1
#define FIPS_STATE_RUNNING  2
#define FIPS_STATE_ERROR    3

/*
 * The number of times the module will report it is in the error state
 * before going quiet.
 */
#define FIPS_ERROR_REPORTING_RATE_LIMIT     10

/* The size of a temp buffer used to read in data */
#define INTEGRITY_BUF_SIZE (4096)
#define MAX_MD_SIZE 64
#define MAC_NAME    "HMAC"
#define DIGEST_NAME "SHA256"

static int FIPS_conditional_error_check = 1;
static int FIPS_state = FIPS_STATE_INIT;
static CRYPTO_RWLOCK *self_test_lock = NULL;
static unsigned char fixed_key[32] = { FIPS_KEY_ELEMENTS };

static CRYPTO_ONCE fips_self_test_init = CRYPTO_ONCE_STATIC_INIT;
DEFINE_RUN_ONCE_STATIC(do_fips_self_test_init)
{
    /*
     * This lock gets freed in platform specific ways that may occur after we
     * do mem leak checking. If we don't know how to free it for a particular
     * platform then we just leak it deliberately. So we temporarily disable the
     * mem leak checking while we allocate this.
     */
    self_test_lock = CRYPTO_THREAD_lock_new();
    return self_test_lock != NULL;
}

#define DEP_DECLARE()                                                          \
void init(void);                                                               \
void cleanup(void);

/*
 * This is the Default Entry Point (DEP) code. Every platform must have a DEP.
 * See FIPS 140-2 IG 9.10
 *
 * If we're run on a platform where we don't know how to define the DEP then
 * the self-tests will never get triggered (FIPS_state never moves to
 * FIPS_STATE_SELFTEST). This will be detected as an error when SELF_TEST_post()
 * is called from OSSL_provider_init(), and so the fips module will be unusable
 * on those platforms.
 */
#if defined(_WIN32) || defined(__CYGWIN__)
# ifdef __CYGWIN__
/* pick DLL_[PROCESS|THREAD]_[ATTACH|DETACH] definitions */
#  include <windows.h>
/*
 * this has side-effect of _WIN32 getting defined, which otherwise is
 * mutually exclusive with __CYGWIN__...
 */
# endif

DEP_DECLARE()
# define DEP_INIT_ATTRIBUTE
# define DEP_FINI_ATTRIBUTE
BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved);
BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
    switch (fdwReason) {
    case DLL_PROCESS_ATTACH:
        init();
        break;
    case DLL_PROCESS_DETACH:
        cleanup();
        break;
    default:
        break;
    }
    return TRUE;
}
#elif defined(__sun) || defined(_AIX)

DEP_DECLARE() /* must be declared before pragma */
# define DEP_INIT_ATTRIBUTE
# define DEP_FINI_ATTRIBUTE
# pragma init(init)
# pragma fini(cleanup)

#elif defined(__hpux)

DEP_DECLARE()
# define DEP_INIT_ATTRIBUTE
# define DEP_FINI_ATTRIBUTE
# pragma init "init"
# pragma fini "cleanup"

#elif defined(__GNUC__)
# define DEP_INIT_ATTRIBUTE static __attribute__((constructor))
# define DEP_FINI_ATTRIBUTE static __attribute__((destructor))
#endif

#if defined(DEP_INIT_ATTRIBUTE) && defined(DEP_FINI_ATTRIBUTE)
DEP_INIT_ATTRIBUTE void init(void)
{
    FIPS_state = FIPS_STATE_SELFTEST;
}

DEP_FINI_ATTRIBUTE void cleanup(void)
{
    CRYPTO_THREAD_lock_free(self_test_lock);
}
#endif

/*
 * Calculate the HMAC SHA256 of data read using a BIO and read_cb, and verify
 * the result matches the expected value.
 * Return 1 if verified, or 0 if it fails.
 */
static int verify_integrity(OSSL_CORE_BIO *bio, OSSL_FUNC_BIO_read_ex_fn read_ex_cb,
                            unsigned char *expected, size_t expected_len,
                            OPENSSL_CTX *libctx, OSSL_SELF_TEST *ev,
                            const char *event_type)
{
    int ret = 0, status;
    unsigned char out[MAX_MD_SIZE];
    unsigned char buf[INTEGRITY_BUF_SIZE];
    size_t bytes_read = 0, out_len = 0;
    EVP_MAC *mac = NULL;
    EVP_MAC_CTX *ctx = NULL;
    OSSL_PARAM params[3], *p = params;

    OSSL_SELF_TEST_onbegin(ev, event_type, OSSL_SELF_TEST_DESC_INTEGRITY_HMAC);

    mac = EVP_MAC_fetch(libctx, MAC_NAME, NULL);
    ctx = EVP_MAC_CTX_new(mac);
    if (mac == NULL || ctx == NULL)
        goto err;

    *p++ = OSSL_PARAM_construct_utf8_string("digest", DIGEST_NAME,
                                            strlen(DIGEST_NAME) + 1);
    *p++ = OSSL_PARAM_construct_octet_string("key", fixed_key,
                                             sizeof(fixed_key));
    *p = OSSL_PARAM_construct_end();

    if (EVP_MAC_CTX_set_params(ctx, params) <= 0
        || !EVP_MAC_init(ctx))
        goto err;

    while (1) {
        status = read_ex_cb(bio, buf, sizeof(buf), &bytes_read);
        if (status != 1)
            break;
        if (!EVP_MAC_update(ctx, buf, bytes_read))
            goto err;
    }
    if (!EVP_MAC_final(ctx, out, &out_len, sizeof(out)))
        goto err;

    OSSL_SELF_TEST_oncorrupt_byte(ev, out);
    if (expected_len != out_len
            || memcmp(expected, out, out_len) != 0)
        goto err;
    ret = 1;
err:
    OSSL_SELF_TEST_onend(ev, ret);
    EVP_MAC_CTX_free(ctx);
    EVP_MAC_free(mac);
    return ret;
}

/* This API is triggered either on loading of the FIPS module or on demand */
int SELF_TEST_post(SELF_TEST_POST_PARAMS *st, int on_demand_test)
{
    int ok = 0;
    int kats_already_passed = 0;
    long checksum_len;
    OSSL_CORE_BIO *bio_module = NULL, *bio_indicator = NULL;
    unsigned char *module_checksum = NULL;
    unsigned char *indicator_checksum = NULL;
    int loclstate;
    OSSL_SELF_TEST *ev = NULL;

    if (!RUN_ONCE(&fips_self_test_init, do_fips_self_test_init))
        return 0;

    CRYPTO_THREAD_read_lock(self_test_lock);
    loclstate = FIPS_state;
    CRYPTO_THREAD_unlock(self_test_lock);

    if (loclstate == FIPS_STATE_RUNNING) {
        if (!on_demand_test)
            return 1;
    } else if (loclstate != FIPS_STATE_SELFTEST) {
        ERR_raise(ERR_LIB_PROV, PROV_R_INVALID_STATE);
        return 0;
    }

    CRYPTO_THREAD_write_lock(self_test_lock);
    if (FIPS_state == FIPS_STATE_RUNNING) {
        if (!on_demand_test) {
            CRYPTO_THREAD_unlock(self_test_lock);
            return 1;
        }
        FIPS_state = FIPS_STATE_SELFTEST;
    } else if (FIPS_state != FIPS_STATE_SELFTEST) {
        CRYPTO_THREAD_unlock(self_test_lock);
        ERR_raise(ERR_LIB_PROV, PROV_R_INVALID_STATE);
        return 0;
    }
    if (st == NULL
            || st->module_checksum_data == NULL) {
        ERR_raise(ERR_LIB_PROV, PROV_R_MISSING_CONFIG_DATA);
        goto end;
    }

    ev = OSSL_SELF_TEST_new(st->cb, st->cb_arg);
    if (ev == NULL)
        goto end;

    module_checksum = OPENSSL_hexstr2buf(st->module_checksum_data,
                                         &checksum_len);
    if (module_checksum == NULL) {
        ERR_raise(ERR_LIB_PROV, PROV_R_INVALID_CONFIG_DATA);
        goto end;
    }
    bio_module = (*st->bio_new_file_cb)(st->module_filename, "rb");

    /* Always check the integrity of the fips module */
    if (bio_module == NULL
            || !verify_integrity(bio_module, st->bio_read_ex_cb,
                                 module_checksum, checksum_len, st->libctx,
                                 ev, OSSL_SELF_TEST_TYPE_MODULE_INTEGRITY)) {
        ERR_raise(ERR_LIB_PROV, PROV_R_MODULE_INTEGRITY_FAILURE);
        goto end;
    }

    /* This will be NULL during installation - so the self test KATS will run */
    if (st->indicator_data != NULL) {
        /*
         * If the kats have already passed indicator is set - then check the
         * integrity of the indicator.
         */
        if (st->indicator_checksum_data == NULL) {
            ERR_raise(ERR_LIB_PROV, PROV_R_MISSING_CONFIG_DATA);
            goto end;
        }
        indicator_checksum = OPENSSL_hexstr2buf(st->indicator_checksum_data,
                                                &checksum_len);
        if (indicator_checksum == NULL) {
            ERR_raise(ERR_LIB_PROV, PROV_R_INVALID_CONFIG_DATA);
            goto end;
        }

        bio_indicator =
            (*st->bio_new_buffer_cb)(st->indicator_data,
                                     strlen(st->indicator_data));
        if (bio_indicator == NULL
                || !verify_integrity(bio_indicator, st->bio_read_ex_cb,
                                     indicator_checksum, checksum_len,
                                     st->libctx, ev,
                                     OSSL_SELF_TEST_TYPE_INSTALL_INTEGRITY)) {
            ERR_raise(ERR_LIB_PROV, PROV_R_INDICATOR_INTEGRITY_FAILURE);
            goto end;
        } else {
            kats_already_passed = 1;
        }
    }

    /* Only runs the KAT's during installation OR on_demand() */
    if (on_demand_test || kats_already_passed == 0) {
        if (!SELF_TEST_kats(ev, st->libctx)) {
            ERR_raise(ERR_LIB_PROV, PROV_R_SELF_TEST_KAT_FAILURE);
            goto end;
        }
    }
    ok = 1;
end:
    OSSL_SELF_TEST_free(ev);
    OPENSSL_free(module_checksum);
    OPENSSL_free(indicator_checksum);

    if (st != NULL) {
        (*st->bio_free_cb)(bio_indicator);
        (*st->bio_free_cb)(bio_module);
    }
    if (ok)
        FIPS_state = FIPS_STATE_RUNNING;
    else
        ossl_set_error_state(OSSL_SELF_TEST_TYPE_NONE);
    CRYPTO_THREAD_unlock(self_test_lock);

    return ok;
}

void SELF_TEST_disable_conditional_error_state(void)
{
    FIPS_conditional_error_check = 0;
}

void ossl_set_error_state(const char *type)
{
    int cond_test = (type != NULL && strcmp(type, OSSL_SELF_TEST_TYPE_PCT) == 0);

    if (!cond_test || (FIPS_conditional_error_check == 1)) {
        FIPS_state = FIPS_STATE_ERROR;
        ERR_raise(ERR_LIB_PROV, PROV_R_FIPS_MODULE_ENTERING_ERROR_STATE);
    } else {
        ERR_raise(ERR_LIB_PROV, PROV_R_FIPS_MODULE_CONDITIONAL_ERROR);
    }
}

int ossl_prov_is_running(void)
{
    const int res = FIPS_state == FIPS_STATE_RUNNING
                    || FIPS_state == FIPS_STATE_SELFTEST;
    static unsigned int rate_limit = 0;

    if (res) {
        rate_limit = 0;
    } else if (FIPS_state == FIPS_STATE_ERROR) {
        if (rate_limit++ < FIPS_ERROR_REPORTING_RATE_LIMIT)
            ERR_raise(ERR_LIB_PROV, PROV_R_FIPS_MODULE_IN_ERROR_STATE);
    }
    return res;
}
