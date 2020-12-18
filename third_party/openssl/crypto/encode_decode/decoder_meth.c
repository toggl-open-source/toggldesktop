/*
 * Copyright 2020 The OpenSSL Project Authors. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://www.openssl.org/source/license.html
 */

#include <openssl/core.h>
#include <openssl/core_dispatch.h>
#include <openssl/decoder.h>
#include <openssl/ui.h>
#include "internal/core.h"
#include "internal/namemap.h"
#include "internal/property.h"
#include "internal/provider.h"
#include "crypto/decoder.h"
#include "encoder_local.h"

/*
 * Decoder can have multiple names, separated with colons in a name string
 */
#define NAME_SEPARATOR ':'

/* Simple method structure constructor and destructor */
static OSSL_DECODER *ossl_decoder_new(void)
{
    OSSL_DECODER *decoder = NULL;

    if ((decoder = OPENSSL_zalloc(sizeof(*decoder))) == NULL
        || (decoder->base.lock = CRYPTO_THREAD_lock_new()) == NULL) {
        OSSL_DECODER_free(decoder);
        ERR_raise(ERR_LIB_OSSL_DECODER, ERR_R_MALLOC_FAILURE);
        return NULL;
    }

    decoder->base.refcnt = 1;

    return decoder;
}

int OSSL_DECODER_up_ref(OSSL_DECODER *decoder)
{
    int ref = 0;

    CRYPTO_UP_REF(&decoder->base.refcnt, &ref, decoder->base.lock);
    return 1;
}

void OSSL_DECODER_free(OSSL_DECODER *decoder)
{
    int ref = 0;

    if (decoder == NULL)
        return;

    CRYPTO_DOWN_REF(&decoder->base.refcnt, &ref, decoder->base.lock);
    if (ref > 0)
        return;
    ossl_provider_free(decoder->base.prov);
    CRYPTO_THREAD_lock_free(decoder->base.lock);
    OPENSSL_free(decoder);
}

/* Permanent decoder method store, constructor and destructor */
static void decoder_store_free(void *vstore)
{
    ossl_method_store_free(vstore);
}

static void *decoder_store_new(OPENSSL_CTX *ctx)
{
    return ossl_method_store_new(ctx);
}


static const OPENSSL_CTX_METHOD decoder_store_method = {
    decoder_store_new,
    decoder_store_free,
};

/* Data to be passed through ossl_method_construct() */
struct decoder_data_st {
    OPENSSL_CTX *libctx;
    OSSL_METHOD_CONSTRUCT_METHOD *mcm;
    int id;                      /* For get_decoder_from_store() */
    const char *names;           /* For get_decoder_from_store() */
    const char *propquery;       /* For get_decoder_from_store() */
};

/*
 * Generic routines to fetch / create DECODER methods with
 * ossl_method_construct()
 */

/* Temporary decoder method store, constructor and destructor */
static void *alloc_tmp_decoder_store(OPENSSL_CTX *ctx)
{
    return ossl_method_store_new(ctx);
}

static void dealloc_tmp_decoder_store(void *store)
{
    if (store != NULL)
        ossl_method_store_free(store);
}

/* Get the permanent decoder store */
static OSSL_METHOD_STORE *get_decoder_store(OPENSSL_CTX *libctx)
{
    return openssl_ctx_get_data(libctx, OPENSSL_CTX_DECODER_STORE_INDEX,
                                &decoder_store_method);
}

/* Get decoder methods from a store, or put one in */
static void *get_decoder_from_store(OPENSSL_CTX *libctx, void *store,
                                    void *data)
{
    struct decoder_data_st *methdata = data;
    void *method = NULL;
    int id;

    if ((id = methdata->id) == 0) {
        OSSL_NAMEMAP *namemap = ossl_namemap_stored(libctx);

        id = ossl_namemap_name2num(namemap, methdata->names);
    }

    if (store == NULL
        && (store = get_decoder_store(libctx)) == NULL)
        return NULL;

    if (!ossl_method_store_fetch(store, id, methdata->propquery, &method))
        return NULL;
    return method;
}

static int put_decoder_in_store(OPENSSL_CTX *libctx, void *store,
                                void *method, const OSSL_PROVIDER *prov,
                                int operation_id, const char *names,
                                const char *propdef, void *unused)
{
    OSSL_NAMEMAP *namemap;
    int id;

    if ((namemap = ossl_namemap_stored(libctx)) == NULL
        || (id = ossl_namemap_name2num(namemap, names)) == 0)
        return 0;

    if (store == NULL && (store = get_decoder_store(libctx)) == NULL)
        return 0;

    return ossl_method_store_add(store, prov, id, propdef, method,
                                 (int (*)(void *))OSSL_DECODER_up_ref,
                                 (void (*)(void *))OSSL_DECODER_free);
}

/* Create and populate a decoder method */
void *ossl_decoder_from_dispatch(int id, const OSSL_ALGORITHM *algodef,
                                 OSSL_PROVIDER *prov)
{
    OSSL_DECODER *decoder = NULL;
    const OSSL_DISPATCH *fns = algodef->implementation;

    if ((decoder = ossl_decoder_new()) == NULL)
        return NULL;
    decoder->base.id = id;
    decoder->base.propdef = algodef->property_definition;

    for (; fns->function_id != 0; fns++) {
        switch (fns->function_id) {
        case OSSL_FUNC_DECODER_NEWCTX:
            if (decoder->newctx == NULL)
                decoder->newctx = OSSL_FUNC_decoder_newctx(fns);
            break;
        case OSSL_FUNC_DECODER_FREECTX:
            if (decoder->freectx == NULL)
                decoder->freectx = OSSL_FUNC_decoder_freectx(fns);
            break;
        case OSSL_FUNC_DECODER_GET_PARAMS:
            if (decoder->get_params == NULL)
                decoder->get_params =
                    OSSL_FUNC_decoder_get_params(fns);
            break;
        case OSSL_FUNC_DECODER_GETTABLE_PARAMS:
            if (decoder->gettable_params == NULL)
                decoder->gettable_params =
                    OSSL_FUNC_decoder_gettable_params(fns);
            break;
        case OSSL_FUNC_DECODER_SET_CTX_PARAMS:
            if (decoder->set_ctx_params == NULL)
                decoder->set_ctx_params =
                    OSSL_FUNC_decoder_set_ctx_params(fns);
            break;
        case OSSL_FUNC_DECODER_SETTABLE_CTX_PARAMS:
            if (decoder->settable_ctx_params == NULL)
                decoder->settable_ctx_params =
                    OSSL_FUNC_decoder_settable_ctx_params(fns);
            break;
        case OSSL_FUNC_DECODER_DECODE:
            if (decoder->decode == NULL)
                decoder->decode = OSSL_FUNC_decoder_decode(fns);
            break;
        case OSSL_FUNC_DECODER_EXPORT_OBJECT:
            if (decoder->export_object == NULL)
                decoder->export_object = OSSL_FUNC_decoder_export_object(fns);
            break;
        }
    }
    /*
     * Try to check that the method is sensible.
     * If you have a constructor, you must have a destructor and vice versa.
     * You must have at least one of the encoding driver functions.
     */
    if (!((decoder->newctx == NULL && decoder->freectx == NULL)
          || (decoder->newctx != NULL && decoder->freectx != NULL))
        || decoder->decode == NULL) {
        OSSL_DECODER_free(decoder);
        ERR_raise(ERR_LIB_OSSL_DECODER, ERR_R_INVALID_PROVIDER_FUNCTIONS);
        return NULL;
    }

    if (prov != NULL && !ossl_provider_up_ref(prov)) {
        OSSL_DECODER_free(decoder);
        return NULL;
    }

    decoder->base.prov = prov;
    return decoder;
}


/*
 * The core fetching functionality passes the names of the implementation.
 * This function is responsible to getting an identity number for them,
 * then call ossl_decoder_from_dispatch() with that identity number.
 */
static void *construct_decoder(const OSSL_ALGORITHM *algodef,
                               OSSL_PROVIDER *prov, void *unused)
{
    /*
     * This function is only called if get_decoder_from_store() returned
     * NULL, so it's safe to say that of all the spots to create a new
     * namemap entry, this is it.  Should the name already exist there, we
     * know that ossl_namemap_add() will return its corresponding number.
     */
    OPENSSL_CTX *libctx = ossl_provider_library_context(prov);
    OSSL_NAMEMAP *namemap = ossl_namemap_stored(libctx);
    const char *names = algodef->algorithm_names;
    int id = ossl_namemap_add_names(namemap, 0, names, NAME_SEPARATOR);
    void *method = NULL;

    if (id != 0)
        method = ossl_decoder_from_dispatch(id, algodef, prov);

    return method;
}

/* Intermediary function to avoid ugly casts, used below */
static void destruct_decoder(void *method, void *data)
{
    OSSL_DECODER_free(method);
}

static int up_ref_decoder(void *method)
{
    return OSSL_DECODER_up_ref(method);
}

static void free_decoder(void *method)
{
    OSSL_DECODER_free(method);
}

/* Fetching support.  Can fetch by numeric identity or by name */
static OSSL_DECODER *inner_ossl_decoder_fetch(OPENSSL_CTX *libctx, int id,
                                              const char *name,
                                              const char *properties)
{
    OSSL_METHOD_STORE *store = get_decoder_store(libctx);
    OSSL_NAMEMAP *namemap = ossl_namemap_stored(libctx);
    void *method = NULL;

    if (store == NULL || namemap == NULL)
        return NULL;

    /*
     * If we have been passed neither a name_id or a name, we have an
     * internal programming error.
     */
    if (!ossl_assert(id != 0 || name != NULL))
        return NULL;

    if (id == 0)
        id = ossl_namemap_name2num(namemap, name);

    if (id == 0
        || !ossl_method_store_cache_get(store, id, properties, &method)) {
        OSSL_METHOD_CONSTRUCT_METHOD mcm = {
            alloc_tmp_decoder_store,
            dealloc_tmp_decoder_store,
            get_decoder_from_store,
            put_decoder_in_store,
            construct_decoder,
            destruct_decoder
        };
        struct decoder_data_st mcmdata;

        mcmdata.libctx = libctx;
        mcmdata.mcm = &mcm;
        mcmdata.id = id;
        mcmdata.names = name;
        mcmdata.propquery = properties;
        if ((method = ossl_method_construct(libctx, OSSL_OP_DECODER,
                                            0 /* !force_cache */,
                                            &mcm, &mcmdata)) != NULL) {
            /*
             * If construction did create a method for us, we know that
             * there is a correct name_id and meth_id, since those have
             * already been calculated in get_decoder_from_store() and
             * put_decoder_in_store() above.
             */
            if (id == 0)
                id = ossl_namemap_name2num(namemap, name);
            ossl_method_store_cache_set(store, id, properties, method,
                                        up_ref_decoder, free_decoder);
        }
    }

    return method;
}

OSSL_DECODER *OSSL_DECODER_fetch(OPENSSL_CTX *libctx, const char *name,
                                 const char *properties)
{
    return inner_ossl_decoder_fetch(libctx, 0, name, properties);
}

OSSL_DECODER *ossl_decoder_fetch_by_number(OPENSSL_CTX *libctx, int id,
                                           const char *properties)
{
    return inner_ossl_decoder_fetch(libctx, id, NULL, properties);
}

/*
 * Library of basic method functions
 */

const OSSL_PROVIDER *OSSL_DECODER_provider(const OSSL_DECODER *decoder)
{
    if (!ossl_assert(decoder != NULL)) {
        ERR_raise(ERR_LIB_OSSL_DECODER, ERR_R_PASSED_NULL_PARAMETER);
        return 0;
    }

    return decoder->base.prov;
}

const char *OSSL_DECODER_properties(const OSSL_DECODER *decoder)
{
    if (!ossl_assert(decoder != NULL)) {
        ERR_raise(ERR_LIB_OSSL_DECODER, ERR_R_PASSED_NULL_PARAMETER);
        return 0;
    }

    return decoder->base.propdef;
}

int OSSL_DECODER_number(const OSSL_DECODER *decoder)
{
    if (!ossl_assert(decoder != NULL)) {
        ERR_raise(ERR_LIB_OSSL_DECODER, ERR_R_PASSED_NULL_PARAMETER);
        return 0;
    }

    return decoder->base.id;
}

int OSSL_DECODER_is_a(const OSSL_DECODER *decoder, const char *name)
{
    if (decoder->base.prov != NULL) {
        OPENSSL_CTX *libctx = ossl_provider_library_context(decoder->base.prov);
        OSSL_NAMEMAP *namemap = ossl_namemap_stored(libctx);

        return ossl_namemap_name2num(namemap, name) == decoder->base.id;
    }
    return 0;
}

struct decoder_do_all_data_st {
    void (*user_fn)(void *method, void *arg);
    void *user_arg;
};

static void decoder_do_one(OSSL_PROVIDER *provider,
                           const OSSL_ALGORITHM *algodef,
                           int no_store, void *vdata)
{
    struct decoder_do_all_data_st *data = vdata;
    OPENSSL_CTX *libctx = ossl_provider_library_context(provider);
    OSSL_NAMEMAP *namemap = ossl_namemap_stored(libctx);
    const char *names = algodef->algorithm_names;
    int id = ossl_namemap_add_names(namemap, 0, names, NAME_SEPARATOR);
    void *method = NULL;

    if (id != 0)
        method = ossl_decoder_from_dispatch(id, algodef, provider);

    if (method != NULL) {
        data->user_fn(method, data->user_arg);
        OSSL_DECODER_free(method);
    }
}

void OSSL_DECODER_do_all_provided(OPENSSL_CTX *libctx,
                                  void (*fn)(OSSL_DECODER *decoder, void *arg),
                                  void *arg)
{
    struct decoder_do_all_data_st data;

    data.user_fn = (void (*)(void *, void *))fn;
    data.user_arg = arg;
    ossl_algorithm_do_all(libctx, OSSL_OP_DECODER, NULL,
                          NULL, decoder_do_one, NULL,
                          &data);
}

void OSSL_DECODER_names_do_all(const OSSL_DECODER *decoder,
                               void (*fn)(const char *name, void *data),
                               void *data)
{
    if (decoder == NULL)
        return;

    if (decoder->base.prov != NULL) {
        OPENSSL_CTX *libctx = ossl_provider_library_context(decoder->base.prov);
        OSSL_NAMEMAP *namemap = ossl_namemap_stored(libctx);

        ossl_namemap_doall_names(namemap, decoder->base.id, fn, data);
    }
}

const OSSL_PARAM *
OSSL_DECODER_gettable_params(OSSL_DECODER *decoder)
{
    if (decoder != NULL && decoder->gettable_params != NULL) {
        void *provctx = ossl_provider_ctx(OSSL_DECODER_provider(decoder));

        return decoder->gettable_params(provctx);
    }
    return NULL;
}

int OSSL_DECODER_get_params(OSSL_DECODER *decoder, OSSL_PARAM params[])
{
    if (decoder != NULL && decoder->get_params != NULL)
        return decoder->get_params(params);
    return 0;
}

const OSSL_PARAM *
OSSL_DECODER_settable_ctx_params(OSSL_DECODER *decoder)
{
    if (decoder != NULL && decoder->settable_ctx_params != NULL) {
        void *provctx = ossl_provider_ctx(OSSL_DECODER_provider(decoder));

        return decoder->settable_ctx_params(provctx);
    }
    return NULL;
}

/*
 * Decoder context support
 */

/*
 * |encoder| value NULL is valid, and signifies that there is no decoder.
 * This is useful to provide fallback mechanisms.
 *  Functions that want to verify if there is a decoder can do so with
 * OSSL_DECODER_CTX_get_decoder()
 */
OSSL_DECODER_CTX *OSSL_DECODER_CTX_new(void)
{
    OSSL_DECODER_CTX *ctx;

    if ((ctx = OPENSSL_zalloc(sizeof(*ctx))) == NULL) {
        ERR_raise(ERR_LIB_OSSL_DECODER, ERR_R_MALLOC_FAILURE);
        return NULL;
    }

    return ctx;
}

int OSSL_DECODER_CTX_set_params(OSSL_DECODER_CTX *ctx,
                                const OSSL_PARAM params[])
{
    size_t i;
    size_t l;

    if (!ossl_assert(ctx != NULL)) {
        ERR_raise(ERR_LIB_OSSL_DECODER, ERR_R_PASSED_NULL_PARAMETER);
        return 0;
    }

    if (ctx->decoder_insts == NULL)
        return 1;

    l = (size_t)sk_OSSL_DECODER_INSTANCE_num(ctx->decoder_insts);
    for (i = 0; i < l; i++) {
        OSSL_DECODER_INSTANCE *decoder_inst =
            sk_OSSL_DECODER_INSTANCE_value(ctx->decoder_insts, i);

        if (decoder_inst->decoderctx == NULL
            || decoder_inst->decoder->set_ctx_params == NULL)
            continue;
        if (!decoder_inst->decoder->set_ctx_params(decoder_inst->decoderctx,
                                                   params))
            return 0;
    }
    return 1;
}

static void
OSSL_DECODER_INSTANCE_free(OSSL_DECODER_INSTANCE *decoder_inst)
{
    if (decoder_inst != NULL) {
        if (decoder_inst->decoder->freectx != NULL)
            decoder_inst->decoder->freectx(decoder_inst->decoderctx);
        decoder_inst->decoderctx = NULL;
        OSSL_DECODER_free(decoder_inst->decoder);
        decoder_inst->decoder = NULL;
        OPENSSL_free(decoder_inst);
        decoder_inst = NULL;
    }
}

void OSSL_DECODER_CTX_free(OSSL_DECODER_CTX *ctx)
{
    if (ctx != NULL) {
        if (ctx->cleanup != NULL)
            ctx->cleanup(ctx->construct_data);
        sk_OSSL_DECODER_INSTANCE_pop_free(ctx->decoder_insts,
                                          OSSL_DECODER_INSTANCE_free);
        ossl_pw_clear_passphrase_data(&ctx->pwdata);
        OPENSSL_free(ctx);
    }
}
