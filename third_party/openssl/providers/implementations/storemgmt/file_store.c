/*
 * Copyright 2020 The OpenSSL Project Authors. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://www.openssl.org/source/license.html
 */

#include "e_os.h"                /* To get strncasecmp() on Windows */
#include <string.h>
#include <sys/stat.h>
#include <ctype.h>
#include <assert.h>

#include <openssl/core.h>
#include <openssl/core_dispatch.h>
#include <openssl/core_names.h>
#include <openssl/core_object.h>
#include <openssl/crypto.h>
#include <openssl/bio.h>
#include <openssl/err.h>
#include <openssl/buffer.h>
#include <openssl/params.h>
#include <openssl/decoder.h>
#include <openssl/store.h>       /* The OSSL_STORE_INFO type numbers */
#include "internal/o_dir.h"
#include "internal/pem.h"        /* For PVK and "blob" PEM headers */
#include "crypto/decoder.h"
#include "prov/implementations.h"
#include "prov/bio.h"
#include "prov/provider_ctx.h"
#include "prov/providercommonerr.h"
#include "file_store_local.h"

DEFINE_STACK_OF(OSSL_STORE_INFO)

#ifdef _WIN32
# define stat _stat
#endif

#ifndef S_ISDIR
# define S_ISDIR(a) (((a) & S_IFMT) == S_IFDIR)
#endif

static OSSL_FUNC_store_open_fn file_open;
static OSSL_FUNC_store_attach_fn file_attach;
static OSSL_FUNC_store_settable_ctx_params_fn file_settable_ctx_params;
static OSSL_FUNC_store_set_ctx_params_fn file_set_ctx_params;
static OSSL_FUNC_store_load_fn file_load;
static OSSL_FUNC_store_eof_fn file_eof;
static OSSL_FUNC_store_close_fn file_close;

/*
 * This implementation makes full use of OSSL_DECODER, and then some.
 * It uses its own internal decoder implementation that reads DER and
 * passes that on to the data callback; this decoder is created with
 * internal OpenSSL functions, thereby bypassing the need for a surrounding
 * provider.  This is ok, since this is a local decoder, not meant for
 * public consumption.  It also uses the libcrypto internal decoder
 * setup function ossl_decoder_ctx_setup_for_EVP_PKEY(), to allow the
 * last resort decoder to be added first (and thereby be executed last).
 * Finally, it sets up its own construct and cleanup functions.
 *
 * Essentially, that makes this implementation a kind of glorified decoder.
 */

struct file_ctx_st {
    void *provctx;
    char *uri;                   /* The URI we currently try to load */
    enum {
        IS_FILE = 0,             /* Read file and pass results */
        IS_DIR                   /* Pass directory entry names */
    } type;

    /* Flag bits */
    unsigned int flag_attached:1;
    unsigned int flag_buffered:1;

    union {
        /* Used with |IS_FILE| */
        struct {
            BIO *file;

            OSSL_DECODER_CTX *decoderctx;
            char *input_type;
            char *propq;    /* The properties we got as a parameter */
        } file;

        /* Used with |IS_DIR| */
        struct {
            OPENSSL_DIR_CTX *ctx;
            int end_reached;

            /*
             * When a search expression is given, these are filled in.
             * |search_name| contains the file basename to look for.
             * The string is exactly 8 characters long.
             */
            char search_name[9];

            /*
             * The directory reading utility we have combines opening with
             * reading the first name.  To make sure we can detect the end
             * at the right time, we read early and cache the name.
             */
            const char *last_entry;
            int last_errno;
        } dir;
    } _;

    /* Expected object type.  May be unspecified */
    int expected_type;
};

static void free_file_ctx(struct file_ctx_st *ctx)
{
    if (ctx == NULL)
        return;

    OPENSSL_free(ctx->uri);
    if (ctx->type != IS_DIR) {
        OSSL_DECODER_CTX_free(ctx->_.file.decoderctx);
        OPENSSL_free(ctx->_.file.propq);
        OPENSSL_free(ctx->_.file.input_type);
    }
    OPENSSL_free(ctx);
}

static struct file_ctx_st *new_file_ctx(int type, const char *uri,
                                        void *provctx)
{
    struct file_ctx_st *ctx = NULL;

    if ((ctx = OPENSSL_zalloc(sizeof(*ctx))) != NULL
        && (uri == NULL || (ctx->uri = OPENSSL_strdup(uri)) != NULL)) {
        ctx->type = type;
        ctx->provctx = provctx;
        return ctx;
    }
    free_file_ctx(ctx);
    return NULL;
}

static OSSL_DECODER_CONSTRUCT file_load_construct;
static OSSL_DECODER_CLEANUP file_load_cleanup;

/*-
 *  Opening / attaching streams and directories
 *  -------------------------------------------
 */

/*
 * Function to service both file_open() and file_attach()
 *
 *
 */
static struct file_ctx_st *file_open_stream(BIO *source, const char *uri,
                                            const char *input_type,
                                            void *provctx)
{
    struct file_ctx_st *ctx;

    if ((ctx = new_file_ctx(IS_FILE, uri, provctx)) == NULL
        || (input_type != NULL
            && (ctx->_.file.input_type =
                OPENSSL_strdup(input_type)) == NULL)) {
        ERR_raise(ERR_LIB_PROV, ERR_R_MALLOC_FAILURE);
        goto err;
    }

    ctx->_.file.file = source;

    return ctx;
 err:
    free_file_ctx(ctx);
    return NULL;
}

static void *file_open_dir(const char *path, const char *uri, void *provctx)
{
    struct file_ctx_st *ctx;

    if ((ctx = new_file_ctx(IS_DIR, uri, provctx)) == NULL) {
        ERR_raise(ERR_LIB_PROV, ERR_R_MALLOC_FAILURE);
        goto err;
    }

    ctx->_.dir.last_entry = OPENSSL_DIR_read(&ctx->_.dir.ctx, path);
    ctx->_.dir.last_errno = errno;
    if (ctx->_.dir.last_entry == NULL) {
        if (ctx->_.dir.last_errno != 0) {
            ERR_raise_data(ERR_LIB_SYS, ctx->_.dir.last_errno,
                           "Calling OPENSSL_DIR_read(\"%s\")", path);
            goto err;
        }
        ctx->_.dir.end_reached = 1;
    }
    return ctx;
 err:
    file_close(ctx);
    return NULL;
}

static void *file_open(void *provctx, const char *uri)
{
    struct file_ctx_st *ctx = NULL;
    struct stat st;
    struct {
        const char *path;
        unsigned int check_absolute:1;
    } path_data[2];
    size_t path_data_n = 0, i;
    const char *path;
    BIO *bio;

    ERR_set_mark();

    /*
     * First step, just take the URI as is.
     */
    path_data[path_data_n].check_absolute = 0;
    path_data[path_data_n++].path = uri;

    /*
     * Second step, if the URI appears to start with the 'file' scheme,
     * extract the path and make that the second path to check.
     * There's a special case if the URI also contains an authority, then
     * the full URI shouldn't be used as a path anywhere.
     */
    if (strncasecmp(uri, "file:", 5) == 0) {
        const char *p = &uri[5];

        if (strncmp(&uri[5], "//", 2) == 0) {
            path_data_n--;           /* Invalidate using the full URI */
            if (strncasecmp(&uri[7], "localhost/", 10) == 0) {
                p = &uri[16];
            } else if (uri[7] == '/') {
                p = &uri[7];
            } else {
                ERR_clear_last_mark();
                ERR_raise(ERR_LIB_PROV, PROV_R_URI_AUTHORITY_UNSUPPORTED);
                return NULL;
            }
        }

        path_data[path_data_n].check_absolute = 1;
#ifdef _WIN32
        /* Windows file: URIs with a drive letter start with a / */
        if (p[0] == '/' && p[2] == ':' && p[3] == '/') {
            char c = tolower(p[1]);

            if (c >= 'a' && c <= 'z') {
                p++;
                /* We know it's absolute, so no need to check */
                path_data[path_data_n].check_absolute = 0;
            }
        }
#endif
        path_data[path_data_n++].path = p;
    }


    for (i = 0, path = NULL; path == NULL && i < path_data_n; i++) {
        /*
         * If the scheme "file" was an explicit part of the URI, the path must
         * be absolute.  So says RFC 8089
         */
        if (path_data[i].check_absolute && path_data[i].path[0] != '/') {
            ERR_clear_last_mark();
            ERR_raise_data(ERR_LIB_PROV, PROV_R_PATH_MUST_BE_ABSOLUTE,
                           "Given path=%s", path_data[i].path);
            return NULL;
        }

        if (stat(path_data[i].path, &st) < 0) {
            ERR_raise_data(ERR_LIB_SYS, errno,
                           "calling stat(%s)",
                           path_data[i].path);
        } else {
            path = path_data[i].path;
        }
    }
    if (path == NULL) {
        ERR_clear_last_mark();
        return NULL;
    }

    /* Successfully found a working path, clear possible collected errors */
    ERR_pop_to_mark();

    if (S_ISDIR(st.st_mode))
        ctx = file_open_dir(path, uri, provctx);
    else if ((bio = BIO_new_file(path, "rb")) == NULL
             || (ctx = file_open_stream(bio, uri, NULL, provctx)) == NULL)
        BIO_free_all(bio);

    return ctx;
}

/*
 * Attached input streams must be treated very very carefully to avoid
 * nasty surprises.
 *
 * This implementation tries to support input streams that can't be reset,
 * such as standard input.  However, OSSL_DECODER assumes resettable streams,
 * and because the PEM decoder may read quite a bit of the input file to skip
 * past any non-PEM text that precedes the PEM block, we may need to detect
 * if the input stream is a PEM file early.
 *
 * If the input stream supports BIO_tell(), we assume that it also supports
 * BIO_seek(), making it a resettable stream and therefore safe to fully
 * unleash OSSL_DECODER.
 *
 * If the input stream doesn't support BIO_tell(), we must assume that we
 * have a non-resettable stream, and must tread carefully.  We do so by
 * trying to detect if the input is PEM, MSBLOB or PVK, and if not, we
 * assume that it's DER.
 *
 * To detect if an input stream is PEM, MSBLOB or PVK, we use the buffer BIO
 * filter, which allows us a 4KiB resettable read-ahead.  We *hope* that 4KiB
 * will be enough to find the start of the PEM block.
 *
 * It should be possible to use this same technique to detect other file
 * types as well.
 *
 * An alternative technique would be to have an endlessly caching BIO filter.
 * That would take away the need for all the detection here, and simply leave
 * it for OSSL_DECODER to find out on its own while supporting its demand for
 * resettable input streams.
 * That's a possible future development.
 */

# define INPUT_TYPE_ANY         NULL
# define INPUT_TYPE_DER         "DER"
# define INPUT_TYPE_PEM         "PEM"
# define INPUT_TYPE_MSBLOB      "MSBLOB"
# define INPUT_TYPE_PVK         "PVK"

void *file_attach(void *provctx, OSSL_CORE_BIO *cin)
{
    BIO *new_bio = bio_new_from_core_bio(provctx, cin);
    BIO *new_bio_tmp = NULL;
    BIO *buff = NULL;
    char peekbuf[4096] = { 0, };
    int loc;
    const char *input_type = NULL;
    unsigned int flag_attached = 1;
    unsigned int flag_buffered = 0;
    struct file_ctx_st *ctx = NULL;

    if (new_bio == NULL)
        return 0;

    /* Try to get the current position */
    loc = BIO_tell(new_bio);

    if ((buff = BIO_new(BIO_f_buffer())) == NULL
        || (new_bio_tmp = BIO_push(buff, new_bio)) == NULL)
        goto err;

    /* Assumption, if we can't detect PEM */
    input_type = INPUT_TYPE_DER;
    flag_buffered = 1;
    new_bio = new_bio_tmp;

    if (BIO_buffer_peek(new_bio, peekbuf, sizeof(peekbuf) - 1) > 0) {
#ifndef OPENSSL_NO_DSA
        const unsigned char *p = NULL;
        unsigned int magic = 0, bitlen = 0;
        int isdss = 0, ispub = -1;
# ifndef OPENSSL_NO_RC4
        unsigned int saltlen = 0, keylen = 0;
# endif
#endif

        peekbuf[sizeof(peekbuf) - 1] = '\0';
        if (strstr(peekbuf, "-----BEGIN ") != NULL)
            input_type = INPUT_TYPE_PEM;
#ifndef OPENSSL_NO_DSA
        else if (p = (unsigned char *)peekbuf,
                 ossl_do_blob_header(&p, sizeof(peekbuf), &magic, &bitlen,
                                     &isdss, &ispub))
            input_type = INPUT_TYPE_MSBLOB;
# ifndef OPENSSL_NO_RC4
        else if (p = (unsigned char *)peekbuf,
                 ossl_do_PVK_header(&p, sizeof(peekbuf), 0, &saltlen, &keylen))
            input_type = INPUT_TYPE_PVK;
# endif
#endif
    }

    /*
     * After peeking, we know that the underlying source BIO has moved ahead
     * from its earlier position and that if it supports BIO_tell(), that
     * should be a number that differs from |loc|.  Otherwise, we will get
     * the same value, which may one of:
     *
     * -   zero (the source BIO doesn't support BIO_tell() / BIO_seek() /
     *     BIO_reset())
     * -   -1 (the underlying operating system / C library routines do not
     *     support BIO_tell() / BIO_seek() / BIO_reset())
     *
     * If it turns out that the source BIO does support BIO_tell(), we pop
     * the buffer BIO filter and mark this input as |INPUT_TYPE_ANY|, which
     * fully unleashes OSSL_DECODER to do its thing.
     */
    if (BIO_tell(new_bio) != loc) {
        /* In this case, anything goes */
        input_type = INPUT_TYPE_ANY;

        /* Restore the source BIO like it was when entering this function */
        new_bio = BIO_pop(buff);
        BIO_free(buff);
        (void)BIO_seek(new_bio, loc);

        flag_buffered = 0;
    }

    if ((ctx = file_open_stream(new_bio, NULL, input_type, provctx)) == NULL)
        goto err;

    ctx->flag_attached = flag_attached;
    ctx->flag_buffered = flag_buffered;

    return ctx;
 err:
    if (flag_buffered) {
        new_bio = BIO_pop(buff);
        BIO_free(buff);
    }
    BIO_free(new_bio);           /* Removes the provider BIO filter */
    return NULL;
}

/*-
 *  Setting parameters
 *  ------------------
 */

static const OSSL_PARAM *file_settable_ctx_params(void *provctx)
{
    static const OSSL_PARAM known_settable_ctx_params[] = {
        OSSL_PARAM_utf8_string(OSSL_STORE_PARAM_PROPERTIES, NULL, 0),
        OSSL_PARAM_int(OSSL_STORE_PARAM_EXPECT, NULL),
        OSSL_PARAM_octet_string(OSSL_STORE_PARAM_SUBJECT, NULL, 0),
        OSSL_PARAM_END
    };
    return known_settable_ctx_params;
}

static int file_set_ctx_params(void *loaderctx, const OSSL_PARAM params[])
{
    struct file_ctx_st *ctx = loaderctx;
    const OSSL_PARAM *p;

    p = OSSL_PARAM_locate_const(params, OSSL_STORE_PARAM_PROPERTIES);
    if (p != NULL) {
        OPENSSL_free(ctx->_.file.propq);
        ctx->_.file.propq = NULL;
        if (!OSSL_PARAM_get_utf8_string(p, &ctx->_.file.propq, 0))
            return 0;
    }
    p = OSSL_PARAM_locate_const(params, OSSL_STORE_PARAM_EXPECT);
    if (p != NULL && !OSSL_PARAM_get_int(p, &ctx->expected_type))
        return 0;
    p = OSSL_PARAM_locate_const(params, OSSL_STORE_PARAM_SUBJECT);
    if (p != NULL) {
        const unsigned char *der = NULL;
        size_t der_len = 0;
        X509_NAME *x509_name;
        unsigned long hash;

        if (ctx->type != IS_DIR) {
            ERR_raise(ERR_LIB_PROV,
                      PROV_R_SEARCH_ONLY_SUPPORTED_FOR_DIRECTORIES);
            return 0;
        }

        if (!OSSL_PARAM_get_octet_string_ptr(p, (const void **)&der, &der_len)
            || (x509_name = d2i_X509_NAME(NULL, &der, der_len)) == NULL)
            return 0;
        hash = X509_NAME_hash(x509_name);
        BIO_snprintf(ctx->_.dir.search_name, sizeof(ctx->_.dir.search_name),
                     "%08lx", hash);
        X509_NAME_free(x509_name);
    }
    return 1;
}

/*-
 *  Loading an object from a stream
 *  -------------------------------
 */

struct file_load_data_st {
    OSSL_CALLBACK *object_cb;
    void *object_cbarg;
};

static int file_load_construct(OSSL_DECODER_INSTANCE *decoder_inst,
                               const OSSL_PARAM *params, void *construct_data)
{
    struct file_load_data_st *data = construct_data;

    /*
     * At some point, we may find it justifiable to recognise PKCS#12 and
     * handle it specially here, making |file_load()| return pass its
     * contents one piece at ta time, like |e_loader_attic.c| does.
     *
     * However, that currently means parsing them out, which converts the
     * DER encoded PKCS#12 into a bunch of EVP_PKEYs and X509s, just to
     * have to re-encode them into DER to create an object abstraction for
     * each of them.
     * It's much simpler (less churn) to pass on the object abstraction we
     * get to the load_result callback and leave it to that one to do the
     * work.  If that's libcrypto code, we know that it has much better
     * possibilities to handle the EVP_PKEYs and X509s without the extra
     * churn.
     */

    return data->object_cb(params, data->object_cbarg);
}

void file_load_cleanup(void *construct_data)
{
    /* Nothing to do */
}

static int file_setup_decoders(struct file_ctx_st *ctx)
{
    EVP_PKEY *dummy; /* for OSSL_DECODER_CTX_new_by_EVP_PKEY() */
    OPENSSL_CTX *libctx = PROV_CTX_get0_library_context(ctx->provctx);
    OSSL_DECODER *to_obj = NULL; /* Last resort decoder */
    OSSL_DECODER_INSTANCE *to_obj_inst = NULL;
    OSSL_DECODER_CLEANUP *old_cleanup = NULL;
    void *old_construct_data = NULL;
    int ok = 0;

    /* Setup for this session, so only if not already done */
    if (ctx->_.file.decoderctx == NULL) {
        if ((ctx->_.file.decoderctx = OSSL_DECODER_CTX_new()) == NULL) {
            ERR_raise(ERR_LIB_PROV, ERR_R_MALLOC_FAILURE);
            goto err;
        }

        /* Make sure the input type is set */
        if (!OSSL_DECODER_CTX_set_input_type(ctx->_.file.decoderctx,
                                             ctx->_.file.input_type)) {
            ERR_raise(ERR_LIB_PROV, ERR_R_OSSL_DECODER_LIB);
            goto err;
        }

        /*
         * Create the internal last resort decoder implementation together
         * with a "decoder instance".
         * The decoder doesn't need any identification or to be attached to
         * any provider, since it's only used locally.
         */
        to_obj = ossl_decoder_from_dispatch(0, &der_to_obj_algorithm, NULL);
        if (to_obj == NULL)
            goto err;
        to_obj_inst = ossl_decoder_instance_new(to_obj, ctx->provctx);
        if (to_obj_inst == NULL)
            goto err;

        if (!ossl_decoder_ctx_add_decoder_inst(ctx->_.file.decoderctx,
                                               to_obj_inst)) {
            ERR_raise(ERR_LIB_PROV, ERR_R_OSSL_DECODER_LIB);
            goto err;
        }

        /*
         * OSSL_DECODER_INSTANCE shouldn't be freed from this point on.
         * That's going to happen whenever the OSSL_DECODER_CTX is freed.
         */
        to_obj_inst = NULL;

        /*
         * Add on the usual decoder context for keys, with a dummy object.
         * Since we're setting up our own constructor, we don't need to care
         * more than that...
         */
        if (!ossl_decoder_ctx_setup_for_EVP_PKEY(ctx->_.file.decoderctx, &dummy,
                                                 libctx, ctx->_.file.propq)
            || !OSSL_DECODER_CTX_add_extra(ctx->_.file.decoderctx,
                                           libctx, ctx->_.file.propq)) {
            ERR_raise(ERR_LIB_PROV, ERR_R_OSSL_DECODER_LIB);
            goto err;
        }

        /*
         * Then we throw away the installed finalizer data, and install our
         * own instead.
         */
        old_cleanup = OSSL_DECODER_CTX_get_cleanup(ctx->_.file.decoderctx);
        old_construct_data =
            OSSL_DECODER_CTX_get_construct_data(ctx->_.file.decoderctx);
        if (old_cleanup != NULL)
            old_cleanup(old_construct_data);

        /*
         * Set the hooks.
         */
        if (!OSSL_DECODER_CTX_set_construct(ctx->_.file.decoderctx,
                                            file_load_construct)
            || !OSSL_DECODER_CTX_set_cleanup(ctx->_.file.decoderctx,
                                             file_load_cleanup)) {
            ERR_raise(ERR_LIB_PROV, ERR_R_OSSL_DECODER_LIB);
            goto err;
        }
    }

    ok = 1;
 err:
    OSSL_DECODER_free(to_obj);
    return ok;
}

static int file_load_file(struct file_ctx_st *ctx,
                          OSSL_CALLBACK *object_cb, void *object_cbarg,
                          OSSL_PASSPHRASE_CALLBACK *pw_cb, void *pw_cbarg)
{
    struct file_load_data_st data;

    /* Setup the decoders (one time shot per session */

    if (!file_setup_decoders(ctx))
        return 0;

    /* Setup for this object */

    data.object_cb = object_cb;
    data.object_cbarg = object_cbarg;
    OSSL_DECODER_CTX_set_construct_data(ctx->_.file.decoderctx, &data);
    OSSL_DECODER_CTX_set_passphrase_cb(ctx->_.file.decoderctx, pw_cb, pw_cbarg);

    /* Launch */

    return OSSL_DECODER_from_bio(ctx->_.file.decoderctx, ctx->_.file.file);
}

/*-
 *  Loading a name object from a directory
 *  --------------------------------------
 */

static int ends_with_dirsep(const char *uri)
{
    if (*uri != '\0')
        uri += strlen(uri) - 1;
#if defined(__VMS)
    if (*uri == ']' || *uri == '>' || *uri == ':')
        return 1;
#elif defined(_WIN32)
    if (*uri == '\\')
        return 1;
#endif
    return *uri == '/';
}

static char *file_name_to_uri(struct file_ctx_st *ctx, const char *name)
{
    char *data = NULL;

    assert(name != NULL);
    {
        const char *pathsep = ends_with_dirsep(ctx->uri) ? "" : "/";
        long calculated_length = strlen(ctx->uri) + strlen(pathsep)
            + strlen(name) + 1 /* \0 */;

        data = OPENSSL_zalloc(calculated_length);
        if (data == NULL) {
            ERR_raise(ERR_LIB_PROV, ERR_R_MALLOC_FAILURE);
            return NULL;
        }

        OPENSSL_strlcat(data, ctx->uri, calculated_length);
        OPENSSL_strlcat(data, pathsep, calculated_length);
        OPENSSL_strlcat(data, name, calculated_length);
    }
    return data;
}

static int file_name_check(struct file_ctx_st *ctx, const char *name)
{
    const char *p = NULL;

    /* If there are no search criteria, all names are accepted */
    if (ctx->_.dir.search_name[0] == '\0')
        return 1;

    /* If the expected type isn't supported, no name is accepted */
    if (ctx->expected_type != 0
        && ctx->expected_type != OSSL_STORE_INFO_CERT
        && ctx->expected_type != OSSL_STORE_INFO_CRL)
        return 0;

    /*
     * First, check the basename
     */
    if (strncasecmp(name, ctx->_.dir.search_name,
                    sizeof(ctx->_.dir.search_name) - 1) != 0
        || name[sizeof(ctx->_.dir.search_name) - 1] != '.')
        return 0;
    p = &name[sizeof(ctx->_.dir.search_name)];

    /*
     * Then, if the expected type is a CRL, check that the extension starts
     * with 'r'
     */
    if (*p == 'r') {
        p++;
        if (ctx->expected_type != 0
            && ctx->expected_type != OSSL_STORE_INFO_CRL)
            return 0;
    } else if (ctx->expected_type == OSSL_STORE_INFO_CRL) {
        return 0;
    }

    /*
     * Last, check that the rest of the extension is a decimal number, at
     * least one digit long.
     */
    if (!isdigit(*p))
        return 0;
    while (isdigit(*p))
        p++;

#ifdef __VMS
    /*
     * One extra step here, check for a possible generation number.
     */
    if (*p == ';')
        for (p++; *p != '\0'; p++)
            if (!ossl_isdigit(*p))
                break;
#endif

    /*
     * If we've reached the end of the string at this point, we've successfully
     * found a fitting file name.
     */
    return *p == '\0';
}

static int file_load_dir_entry(struct file_ctx_st *ctx,
                               OSSL_CALLBACK *object_cb, void *object_cbarg,
                               OSSL_PASSPHRASE_CALLBACK *pw_cb, void *pw_cbarg)
{
    /* Prepare as much as possible in advance */
    static const int object_type = OSSL_OBJECT_NAME;
    OSSL_PARAM object[] = {
        OSSL_PARAM_int(OSSL_OBJECT_PARAM_TYPE, (int *)&object_type),
        OSSL_PARAM_utf8_string(OSSL_OBJECT_PARAM_DATA, NULL, 0),
        OSSL_PARAM_END
    };
    char *newname = NULL;
    int ok;

    /* Loop until we get an error or until we have a suitable name */
    do {
        if (ctx->_.dir.last_entry == NULL) {
            if (!ctx->_.dir.end_reached) {
                assert(ctx->_.dir.last_errno != 0);
                ERR_raise(ERR_LIB_SYS, ctx->_.dir.last_errno);
            }
            /* file_eof() will tell if EOF was reached */
            return 0;
        }

        /* flag acceptable names */
        if (ctx->_.dir.last_entry[0] != '.'
            && file_name_check(ctx, ctx->_.dir.last_entry)) {

            /* If we can't allocate the new name, we fail */
            if ((newname =
                 file_name_to_uri(ctx, ctx->_.dir.last_entry)) == NULL)
                return 0;
        }

        /*
         * On the first call (with a NULL context), OPENSSL_DIR_read()
         * cares about the second argument.  On the following calls, it
         * only cares that it isn't NULL.  Therefore, we can safely give
         * it our URI here.
         */
        ctx->_.dir.last_entry = OPENSSL_DIR_read(&ctx->_.dir.ctx, ctx->uri);
        ctx->_.dir.last_errno = errno;
        if (ctx->_.dir.last_entry == NULL && ctx->_.dir.last_errno == 0)
            ctx->_.dir.end_reached = 1;
    } while (newname == NULL);

    object[1].data = newname;
    object[1].data_size = strlen(newname);
    ok = object_cb(object, object_cbarg);
    OPENSSL_free(newname);
    return ok;
}

/*-
 *  Loading, local dispatcher
 *  -------------------------
 */

static int file_load(void *loaderctx,
                     OSSL_CALLBACK *object_cb, void *object_cbarg,
                     OSSL_PASSPHRASE_CALLBACK *pw_cb, void *pw_cbarg)
{
    struct file_ctx_st *ctx = loaderctx;

    switch (ctx->type) {
    case IS_FILE:
        return file_load_file(ctx, object_cb, object_cbarg, pw_cb, pw_cbarg);
    case IS_DIR:
        return
            file_load_dir_entry(ctx, object_cb, object_cbarg, pw_cb, pw_cbarg);
    default:
        break;
    }

    /* ctx->type has an unexpected value */
    assert(0);
    return 0;
}

/*-
 *  Eof detection and closing
 *  -------------------------
 */

static int file_eof(void *loaderctx)
{
    struct file_ctx_st *ctx = loaderctx;

    switch (ctx->type) {
    case IS_DIR:
        return ctx->_.dir.end_reached;
    case IS_FILE:
        /*
         * BIO_pending() checks any filter BIO.
         * BIO_eof() checks the source BIO.
         */
        return !BIO_pending(ctx->_.file.file)
            && BIO_eof(ctx->_.file.file);
    }

    /* ctx->type has an unexpected value */
    assert(0);
    return 1;
}

static int file_close_dir(struct file_ctx_st *ctx)
{
    if (ctx->_.dir.ctx != NULL)
        OPENSSL_DIR_end(&ctx->_.dir.ctx);
    free_file_ctx(ctx);
    return 1;
}

static int file_close_stream(struct file_ctx_st *ctx)
{
    if (ctx->flag_buffered) {
        /*
         * file_attach() pushed a BIO_f_buffer() on top of the regular BIO.
         * Drop it.
         */
        BIO *buff = ctx->_.file.file;

        /* Detach buff */
        ctx->_.file.file = BIO_pop(ctx->_.file.file);

        BIO_free(buff);
    }

    /*
     * If it was attached, we only free the top, as that's the provider BIO
     * filter.  Otherwise, it was entirely allocated by this implementation,
     * and can safely be completely freed.
     */
    if (ctx->flag_attached)
        BIO_free(ctx->_.file.file);
    else
        BIO_free_all(ctx->_.file.file);

    /* To avoid double free */
    ctx->_.file.file = NULL;

    free_file_ctx(ctx);
    return 1;
}

static int file_close(void *loaderctx)
{
    struct file_ctx_st *ctx = loaderctx;

    switch (ctx->type) {
    case IS_DIR:
        return file_close_dir(ctx);
    case IS_FILE:
        return file_close_stream(ctx);
    }

    /* ctx->type has an unexpected value */
    assert(0);
    return 1;
}

const OSSL_DISPATCH file_store_functions[] = {
    { OSSL_FUNC_STORE_OPEN, (void (*)(void))file_open },
    { OSSL_FUNC_STORE_ATTACH, (void (*)(void))file_attach },
    { OSSL_FUNC_STORE_SETTABLE_CTX_PARAMS,
      (void (*)(void))file_settable_ctx_params },
    { OSSL_FUNC_STORE_SET_CTX_PARAMS, (void (*)(void))file_set_ctx_params },
    { OSSL_FUNC_STORE_LOAD, (void (*)(void))file_load },
    { OSSL_FUNC_STORE_EOF, (void (*)(void))file_eof },
    { OSSL_FUNC_STORE_CLOSE, (void (*)(void))file_close },
    { 0, NULL },
};
