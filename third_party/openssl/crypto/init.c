/*
 * Copyright 2016-2020 The OpenSSL Project Authors. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://www.openssl.org/source/license.html
 */

/* We need to use some engine deprecated APIs */
#define OPENSSL_SUPPRESS_DEPRECATED

#include "e_os.h"
#include "crypto/cryptlib.h"
#include <openssl/err.h>
#include "crypto/rand.h"
#include "internal/bio.h"
#include <openssl/evp.h>
#include "crypto/evp.h"
#include "internal/conf.h"
#include "crypto/async.h"
#include "crypto/engine.h"
#include "internal/comp.h"
#include "internal/err.h"
#include "crypto/err.h"
#include "crypto/objects.h"
#include <stdlib.h>
#include <assert.h>
#include "internal/thread_once.h"
#include "crypto/dso_conf.h"
#include "internal/dso.h"
#include "crypto/store.h"
#include <openssl/cmp_util.h> /* for OSSL_CMP_log_close() */
#include <openssl/trace.h>

static int stopped = 0;

typedef struct ossl_init_stop_st OPENSSL_INIT_STOP;
struct ossl_init_stop_st {
    void (*handler)(void);
    OPENSSL_INIT_STOP *next;
};

static OPENSSL_INIT_STOP *stop_handlers = NULL;
static CRYPTO_RWLOCK *init_lock = NULL;

static CRYPTO_ONCE base = CRYPTO_ONCE_STATIC_INIT;
static int base_inited = 0;
DEFINE_RUN_ONCE_STATIC(ossl_init_base)
{
    /* no need to init trace */

    OSSL_TRACE(INIT, "ossl_init_base: setting up stop handlers\n");
#ifndef OPENSSL_NO_CRYPTO_MDEBUG
    ossl_malloc_setup_failures();
#endif

    if ((init_lock = CRYPTO_THREAD_lock_new()) == NULL)
        goto err;
    OPENSSL_cpuid_setup();

    if (!ossl_init_thread())
        return 0;

    base_inited = 1;
    return 1;

err:
    OSSL_TRACE(INIT, "ossl_init_base failed!\n");
    CRYPTO_THREAD_lock_free(init_lock);
    init_lock = NULL;

    return 0;
}

static CRYPTO_ONCE register_atexit = CRYPTO_ONCE_STATIC_INIT;
#if !defined(OPENSSL_SYS_UEFI) && defined(_WIN32)
static int win32atexit(void)
{
    OPENSSL_cleanup();
    return 0;
}
#endif

DEFINE_RUN_ONCE_STATIC(ossl_init_register_atexit)
{
#ifdef OPENSSL_INIT_DEBUG
    fprintf(stderr, "OPENSSL_INIT: ossl_init_register_atexit()\n");
#endif
#ifndef OPENSSL_SYS_UEFI
# ifdef _WIN32
    /* We use _onexit() in preference because it gets called on DLL unload */
    if (_onexit(win32atexit) == NULL)
        return 0;
# else
    if (atexit(OPENSSL_cleanup) != 0)
        return 0;
# endif
#endif

    return 1;
}

DEFINE_RUN_ONCE_STATIC_ALT(ossl_init_no_register_atexit,
                           ossl_init_register_atexit)
{
#ifdef OPENSSL_INIT_DEBUG
    fprintf(stderr, "OPENSSL_INIT: ossl_init_no_register_atexit ok!\n");
#endif
    /* Do nothing in this case */
    return 1;
}

static CRYPTO_ONCE load_crypto_nodelete = CRYPTO_ONCE_STATIC_INIT;
DEFINE_RUN_ONCE_STATIC(ossl_init_load_crypto_nodelete)
{
    OSSL_TRACE(INIT, "ossl_init_load_crypto_nodelete()\n");

#if !defined(OPENSSL_USE_NODELETE) \
    && !defined(OPENSSL_NO_PINSHARED)
# if defined(DSO_WIN32) && !defined(_WIN32_WCE)
    {
        HMODULE handle = NULL;
        BOOL ret;

        /* We don't use the DSO route for WIN32 because there is a better way */
        ret = GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS
                                | GET_MODULE_HANDLE_EX_FLAG_PIN,
                                (void *)&base_inited, &handle);

        OSSL_TRACE1(INIT,
                    "ossl_init_load_crypto_nodelete: "
                    "obtained DSO reference? %s\n",
                    (ret == TRUE ? "No!" : "Yes."));
        return (ret == TRUE) ? 1 : 0;
    }
# elif !defined(DSO_NONE)
    /*
     * Deliberately leak a reference to ourselves. This will force the library
     * to remain loaded until the atexit() handler is run at process exit.
     */
    {
        DSO *dso;
        void *err;

        if (!err_shelve_state(&err))
            return 0;

        dso = DSO_dsobyaddr(&base_inited, DSO_FLAG_NO_UNLOAD_ON_FREE);
        /*
         * In case of No!, it is uncertain our exit()-handlers can still be
         * called. After dlclose() the whole library might have been unloaded
         * already.
         */
        OSSL_TRACE1(INIT, "obtained DSO reference? %s\n",
                    (dso == NULL ? "No!" : "Yes."));
        DSO_free(dso);
        err_unshelve_state(err);
    }
# endif
#endif

    return 1;
}

static CRYPTO_ONCE load_crypto_strings = CRYPTO_ONCE_STATIC_INIT;
static int load_crypto_strings_inited = 0;
DEFINE_RUN_ONCE_STATIC(ossl_init_load_crypto_strings)
{
    int ret = 1;
    /*
     * OPENSSL_NO_AUTOERRINIT is provided here to prevent at compile time
     * pulling in all the error strings during static linking
     */
#if !defined(OPENSSL_NO_ERR) && !defined(OPENSSL_NO_AUTOERRINIT)
    OSSL_TRACE(INIT, "err_load_crypto_strings_int()\n");
    ret = err_load_crypto_strings_int();
    load_crypto_strings_inited = 1;
#endif
    return ret;
}

DEFINE_RUN_ONCE_STATIC_ALT(ossl_init_no_load_crypto_strings,
                           ossl_init_load_crypto_strings)
{
    /* Do nothing in this case */
    return 1;
}

static CRYPTO_ONCE add_all_ciphers = CRYPTO_ONCE_STATIC_INIT;
DEFINE_RUN_ONCE_STATIC(ossl_init_add_all_ciphers)
{
    /*
     * OPENSSL_NO_AUTOALGINIT is provided here to prevent at compile time
     * pulling in all the ciphers during static linking
     */
#ifndef OPENSSL_NO_AUTOALGINIT
    OSSL_TRACE(INIT, "openssl_add_all_ciphers_int()\n");
    openssl_add_all_ciphers_int();
#endif
    return 1;
}

DEFINE_RUN_ONCE_STATIC_ALT(ossl_init_no_add_all_ciphers,
                           ossl_init_add_all_ciphers)
{
    /* Do nothing */
    return 1;
}

static CRYPTO_ONCE add_all_digests = CRYPTO_ONCE_STATIC_INIT;
DEFINE_RUN_ONCE_STATIC(ossl_init_add_all_digests)
{
    /*
     * OPENSSL_NO_AUTOALGINIT is provided here to prevent at compile time
     * pulling in all the ciphers during static linking
     */
#ifndef OPENSSL_NO_AUTOALGINIT
    OSSL_TRACE(INIT, "openssl_add_all_digests()\n");
    openssl_add_all_digests_int();
#endif
    return 1;
}

DEFINE_RUN_ONCE_STATIC_ALT(ossl_init_no_add_all_digests,
                           ossl_init_add_all_digests)
{
    /* Do nothing */
    return 1;
}

static CRYPTO_ONCE config = CRYPTO_ONCE_STATIC_INIT;
static int config_inited = 0;
static const OPENSSL_INIT_SETTINGS *conf_settings = NULL;
DEFINE_RUN_ONCE_STATIC(ossl_init_config)
{
    int ret = openssl_config_int(conf_settings);
    config_inited = 1;
    return ret;
}
DEFINE_RUN_ONCE_STATIC_ALT(ossl_init_no_config, ossl_init_config)
{
    OSSL_TRACE(INIT, "openssl_no_config_int()\n");
    openssl_no_config_int();
    config_inited = 1;
    return 1;
}

static CRYPTO_ONCE async = CRYPTO_ONCE_STATIC_INIT;
static int async_inited = 0;
DEFINE_RUN_ONCE_STATIC(ossl_init_async)
{
    OSSL_TRACE(INIT, "async_init()\n");
    if (!async_init())
        return 0;
    async_inited = 1;
    return 1;
}

#ifndef OPENSSL_NO_ENGINE
static CRYPTO_ONCE engine_openssl = CRYPTO_ONCE_STATIC_INIT;
DEFINE_RUN_ONCE_STATIC(ossl_init_engine_openssl)
{
    OSSL_TRACE(INIT, "engine_load_openssl_int()\n");
    engine_load_openssl_int();
    return 1;
}
# ifndef OPENSSL_NO_RDRAND
static CRYPTO_ONCE engine_rdrand = CRYPTO_ONCE_STATIC_INIT;
DEFINE_RUN_ONCE_STATIC(ossl_init_engine_rdrand)
{
    OSSL_TRACE(INIT, "engine_load_rdrand_int()\n");
    engine_load_rdrand_int();
    return 1;
}
# endif
static CRYPTO_ONCE engine_dynamic = CRYPTO_ONCE_STATIC_INIT;
DEFINE_RUN_ONCE_STATIC(ossl_init_engine_dynamic)
{
    OSSL_TRACE(INIT, "engine_load_dynamic_int()\n");
    engine_load_dynamic_int();
    return 1;
}
# ifndef OPENSSL_NO_STATIC_ENGINE
#  ifndef OPENSSL_NO_DEVCRYPTOENG
static CRYPTO_ONCE engine_devcrypto = CRYPTO_ONCE_STATIC_INIT;
DEFINE_RUN_ONCE_STATIC(ossl_init_engine_devcrypto)
{
    OSSL_TRACE(INIT, "engine_load_devcrypto_int()\n");
    engine_load_devcrypto_int();
    return 1;
}
#  endif
#  if !defined(OPENSSL_NO_PADLOCKENG)
static CRYPTO_ONCE engine_padlock = CRYPTO_ONCE_STATIC_INIT;
DEFINE_RUN_ONCE_STATIC(ossl_init_engine_padlock)
{
    OSSL_TRACE(INIT, "engine_load_padlock_int()\n");
    engine_load_padlock_int();
    return 1;
}
#  endif
#  if defined(OPENSSL_SYS_WIN32) && !defined(OPENSSL_NO_CAPIENG)
static CRYPTO_ONCE engine_capi = CRYPTO_ONCE_STATIC_INIT;
DEFINE_RUN_ONCE_STATIC(ossl_init_engine_capi)
{
    OSSL_TRACE(INIT, "engine_load_capi_int()\n");
    engine_load_capi_int();
    return 1;
}
#  endif
#  if !defined(OPENSSL_NO_AFALGENG)
static CRYPTO_ONCE engine_afalg = CRYPTO_ONCE_STATIC_INIT;
DEFINE_RUN_ONCE_STATIC(ossl_init_engine_afalg)
{
    OSSL_TRACE(INIT, "engine_load_afalg_int()\n");
    engine_load_afalg_int();
    return 1;
}
#  endif
# endif
#endif

#ifndef OPENSSL_NO_COMP
static CRYPTO_ONCE zlib = CRYPTO_ONCE_STATIC_INIT;

static int zlib_inited = 0;
DEFINE_RUN_ONCE_STATIC(ossl_init_zlib)
{
    /* Do nothing - we need to know about this for the later cleanup */
    zlib_inited = 1;
    return 1;
}
#endif

void OPENSSL_cleanup(void)
{
    OPENSSL_INIT_STOP *currhandler, *lasthandler;

    /*
     * TODO(3.0): This function needs looking at with a view to moving most/all
     * of this into onfree handlers in OPENSSL_CTX.
     */

    /* If we've not been inited then no need to deinit */
    if (!base_inited)
        return;

    /* Might be explicitly called and also by atexit */
    if (stopped)
        return;
    stopped = 1;

    /*
     * Thread stop may not get automatically called by the thread library for
     * the very last thread in some situations, so call it directly.
     */
    OPENSSL_thread_stop();

    currhandler = stop_handlers;
    while (currhandler != NULL) {
        currhandler->handler();
        lasthandler = currhandler;
        currhandler = currhandler->next;
        OPENSSL_free(lasthandler);
    }
    stop_handlers = NULL;

    CRYPTO_THREAD_lock_free(init_lock);
    init_lock = NULL;

    /*
     * We assume we are single-threaded for this function, i.e. no race
     * conditions for the various "*_inited" vars below.
     */

#ifndef OPENSSL_NO_COMP
    if (zlib_inited) {
        OSSL_TRACE(INIT, "OPENSSL_cleanup: comp_zlib_cleanup_int()\n");
        comp_zlib_cleanup_int();
    }
#endif

    if (async_inited) {
        OSSL_TRACE(INIT, "OPENSSL_cleanup: async_deinit()\n");
        async_deinit();
    }

    if (load_crypto_strings_inited) {
        OSSL_TRACE(INIT, "OPENSSL_cleanup: err_free_strings_int()\n");
        err_free_strings_int();
    }

    /*
     * Note that cleanup order is important:
     * - rand_cleanup_int could call an ENGINE's RAND cleanup function so
     * must be called before engine_cleanup_int()
     * - ENGINEs use CRYPTO_EX_DATA and therefore, must be cleaned up
     * before the ex data handlers are wiped during default openssl_ctx deinit.
     * - conf_modules_free_int() can end up in ENGINE code so must be called
     * before engine_cleanup_int()
     * - ENGINEs and additional EVP algorithms might use added OIDs names so
     * obj_cleanup_int() must be called last
     */
    OSSL_TRACE(INIT, "OPENSSL_cleanup: rand_cleanup_int()\n");
    rand_cleanup_int();

    OSSL_TRACE(INIT, "OPENSSL_cleanup: conf_modules_free_int()\n");
    conf_modules_free_int();

#ifndef OPENSSL_NO_ENGINE
    OSSL_TRACE(INIT, "OPENSSL_cleanup: engine_cleanup_int()\n");
    engine_cleanup_int();
#endif

#ifndef OPENSSL_NO_DEPRECATED_3_0
    OSSL_TRACE(INIT, "OPENSSL_cleanup: ossl_store_cleanup_int()\n");
    ossl_store_cleanup_int();
#endif

    OSSL_TRACE(INIT, "OPENSSL_cleanup: openssl_ctx_default_deinit()\n");
    openssl_ctx_default_deinit();

    ossl_cleanup_thread();

    OSSL_TRACE(INIT, "OPENSSL_cleanup: bio_cleanup()\n");
    bio_cleanup();

    OSSL_TRACE(INIT, "OPENSSL_cleanup: evp_cleanup_int()\n");
    evp_cleanup_int();

    OSSL_TRACE(INIT, "OPENSSL_cleanup: obj_cleanup_int()\n");
    obj_cleanup_int();

    OSSL_TRACE(INIT, "OPENSSL_cleanup: err_int()\n");
    err_cleanup();

    OSSL_TRACE(INIT, "OPENSSL_cleanup: CRYPTO_secure_malloc_done()\n");
    CRYPTO_secure_malloc_done();

#ifndef OPENSSL_NO_CMP
    OSSL_TRACE(INIT, "OPENSSL_cleanup: OSSL_CMP_log_close()\n");
    OSSL_CMP_log_close();
#endif

    OSSL_TRACE(INIT, "OPENSSL_cleanup: ossl_trace_cleanup()\n");
    ossl_trace_cleanup();

    base_inited = 0;
}

/*
 * If this function is called with a non NULL settings value then it must be
 * called prior to any threads making calls to any OpenSSL functions,
 * i.e. passing a non-null settings value is assumed to be single-threaded.
 */
int OPENSSL_init_crypto(uint64_t opts, const OPENSSL_INIT_SETTINGS *settings)
{
    /*
     * TODO(3.0): This function needs looking at with a view to moving most/all
     * of this into OPENSSL_CTX.
     */

    if (stopped) {
        if (!(opts & OPENSSL_INIT_BASE_ONLY))
            CRYPTOerr(CRYPTO_F_OPENSSL_INIT_CRYPTO, ERR_R_INIT_FAIL);
        return 0;
    }

    /*
     * When the caller specifies OPENSSL_INIT_BASE_ONLY, that should be the
     * *only* option specified.  With that option we return immediately after
     * doing the requested limited initialization.  Note that
     * err_shelve_state() called by us via ossl_init_load_crypto_nodelete()
     * re-enters OPENSSL_init_crypto() with OPENSSL_INIT_BASE_ONLY, but with
     * base already initialized this is a harmless NOOP.
     *
     * If we remain the only caller of err_shelve_state() the recursion should
     * perhaps be removed, but if in doubt, it can be left in place.
     */
    if (!RUN_ONCE(&base, ossl_init_base))
        return 0;

    if (opts & OPENSSL_INIT_BASE_ONLY)
        return 1;

    /*
     * Now we don't always set up exit handlers, the INIT_BASE_ONLY calls
     * should not have the side-effect of setting up exit handlers, and
     * therefore, this code block is below the INIT_BASE_ONLY-conditioned early
     * return above.
     */
    if ((opts & OPENSSL_INIT_NO_ATEXIT) != 0) {
        if (!RUN_ONCE_ALT(&register_atexit, ossl_init_no_register_atexit,
                          ossl_init_register_atexit))
            return 0;
    } else if (!RUN_ONCE(&register_atexit, ossl_init_register_atexit)) {
        return 0;
    }

    if (!RUN_ONCE(&load_crypto_nodelete, ossl_init_load_crypto_nodelete))
        return 0;

    if ((opts & OPENSSL_INIT_NO_LOAD_CRYPTO_STRINGS)
            && !RUN_ONCE_ALT(&load_crypto_strings,
                             ossl_init_no_load_crypto_strings,
                             ossl_init_load_crypto_strings))
        return 0;

    if ((opts & OPENSSL_INIT_LOAD_CRYPTO_STRINGS)
            && !RUN_ONCE(&load_crypto_strings, ossl_init_load_crypto_strings))
        return 0;

    if ((opts & OPENSSL_INIT_NO_ADD_ALL_CIPHERS)
            && !RUN_ONCE_ALT(&add_all_ciphers, ossl_init_no_add_all_ciphers,
                             ossl_init_add_all_ciphers))
        return 0;

    if ((opts & OPENSSL_INIT_ADD_ALL_CIPHERS)
            && !RUN_ONCE(&add_all_ciphers, ossl_init_add_all_ciphers))
        return 0;

    if ((opts & OPENSSL_INIT_NO_ADD_ALL_DIGESTS)
            && !RUN_ONCE_ALT(&add_all_digests, ossl_init_no_add_all_digests,
                             ossl_init_add_all_digests))
        return 0;

    if ((opts & OPENSSL_INIT_ADD_ALL_DIGESTS)
            && !RUN_ONCE(&add_all_digests, ossl_init_add_all_digests))
        return 0;

    if ((opts & OPENSSL_INIT_ATFORK)
            && !openssl_init_fork_handlers())
        return 0;

    if ((opts & OPENSSL_INIT_NO_LOAD_CONFIG)
            && !RUN_ONCE_ALT(&config, ossl_init_no_config, ossl_init_config))
        return 0;

    if (opts & OPENSSL_INIT_LOAD_CONFIG) {
        int ret;
        CRYPTO_THREAD_write_lock(init_lock);
        conf_settings = settings;
        ret = RUN_ONCE(&config, ossl_init_config);
        conf_settings = NULL;
        CRYPTO_THREAD_unlock(init_lock);
        if (ret <= 0)
            return 0;
    }

    if ((opts & OPENSSL_INIT_ASYNC)
            && !RUN_ONCE(&async, ossl_init_async))
        return 0;

#ifndef OPENSSL_NO_ENGINE
    if ((opts & OPENSSL_INIT_ENGINE_OPENSSL)
            && !RUN_ONCE(&engine_openssl, ossl_init_engine_openssl))
        return 0;
# ifndef OPENSSL_NO_RDRAND
    if ((opts & OPENSSL_INIT_ENGINE_RDRAND)
            && !RUN_ONCE(&engine_rdrand, ossl_init_engine_rdrand))
        return 0;
# endif
    if ((opts & OPENSSL_INIT_ENGINE_DYNAMIC)
            && !RUN_ONCE(&engine_dynamic, ossl_init_engine_dynamic))
        return 0;
# ifndef OPENSSL_NO_STATIC_ENGINE
#  ifndef OPENSSL_NO_DEVCRYPTOENG
    if ((opts & OPENSSL_INIT_ENGINE_CRYPTODEV)
            && !RUN_ONCE(&engine_devcrypto, ossl_init_engine_devcrypto))
        return 0;
#  endif
#  if !defined(OPENSSL_NO_PADLOCKENG)
    if ((opts & OPENSSL_INIT_ENGINE_PADLOCK)
            && !RUN_ONCE(&engine_padlock, ossl_init_engine_padlock))
        return 0;
#  endif
#  if defined(OPENSSL_SYS_WIN32) && !defined(OPENSSL_NO_CAPIENG)
    if ((opts & OPENSSL_INIT_ENGINE_CAPI)
            && !RUN_ONCE(&engine_capi, ossl_init_engine_capi))
        return 0;
#  endif
#  if !defined(OPENSSL_NO_AFALGENG)
    if ((opts & OPENSSL_INIT_ENGINE_AFALG)
            && !RUN_ONCE(&engine_afalg, ossl_init_engine_afalg))
        return 0;
#  endif
# endif
    if (opts & (OPENSSL_INIT_ENGINE_ALL_BUILTIN
                | OPENSSL_INIT_ENGINE_OPENSSL
                | OPENSSL_INIT_ENGINE_AFALG)) {
        ENGINE_register_all_complete();
    }
#endif

#ifndef OPENSSL_NO_COMP
    if ((opts & OPENSSL_INIT_ZLIB)
            && !RUN_ONCE(&zlib, ossl_init_zlib))
        return 0;
#endif

    return 1;
}

int OPENSSL_atexit(void (*handler)(void))
{
    OPENSSL_INIT_STOP *newhand;

#if !defined(OPENSSL_USE_NODELETE)\
    && !defined(OPENSSL_NO_PINSHARED)
    {
        union {
            void *sym;
            void (*func)(void);
        } handlersym;

        handlersym.func = handler;
# if defined(DSO_WIN32) && !defined(_WIN32_WCE)
        {
            HMODULE handle = NULL;
            BOOL ret;

            /*
             * We don't use the DSO route for WIN32 because there is a better
             * way
             */
            ret = GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS
                                    | GET_MODULE_HANDLE_EX_FLAG_PIN,
                                    handlersym.sym, &handle);

            if (!ret)
                return 0;
        }
# elif !defined(DSO_NONE)
        /*
         * Deliberately leak a reference to the handler. This will force the
         * library/code containing the handler to remain loaded until we run the
         * atexit handler. If -znodelete has been used then this is
         * unnecessary.
         */
        {
            DSO *dso = NULL;

            ERR_set_mark();
            dso = DSO_dsobyaddr(handlersym.sym, DSO_FLAG_NO_UNLOAD_ON_FREE);
            /* See same code above in ossl_init_base() for an explanation. */
            OSSL_TRACE1(INIT,
                       "atexit: obtained DSO reference? %s\n",
                       (dso == NULL ? "No!" : "Yes."));
            DSO_free(dso);
            ERR_pop_to_mark();
        }
# endif
    }
#endif

    if ((newhand = OPENSSL_malloc(sizeof(*newhand))) == NULL) {
        CRYPTOerr(CRYPTO_F_OPENSSL_ATEXIT, ERR_R_MALLOC_FAILURE);
        return 0;
    }

    newhand->handler = handler;
    newhand->next = stop_handlers;
    stop_handlers = newhand;

    return 1;
}

#ifdef OPENSSL_SYS_UNIX
/*
 * The following three functions are for OpenSSL developers.  This is
 * where we set/reset state across fork (called via pthread_atfork when
 * it exists, or manually by the application when it doesn't).
 *
 * WARNING!  If you put code in either OPENSSL_fork_parent or
 * OPENSSL_fork_child, you MUST MAKE SURE that they are async-signal-
 * safe.  See this link, for example:
 *      http://man7.org/linux/man-pages/man7/signal-safety.7.html
 */

void OPENSSL_fork_prepare(void)
{
}

void OPENSSL_fork_parent(void)
{
}

void OPENSSL_fork_child(void)
{
    /* TODO(3.0): Inform all providers about a fork event */
}
#endif
