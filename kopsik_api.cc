// Copyright 2013 Tanel Lebedev

#include <cstring>

#include "./kopsik_api.h"
#include "./database.h"
#include "./toggl_api_client.h"

#define DBNAME "kopsik.db"

void kopsik_version(int *major, int *minor, int *patch) {
    *major = 0;
    *minor = 1;
    *patch = 0;
}

kopsik_api_result kopsik_current_user(char *errmsg, unsigned int errlen,
        KopsikUser *out_user) {
    if (!out_user) {
        strncpy(errmsg, "Invalid user pointer", errlen);
        return KOPSIK_API_FAILURE;
    }
    kopsik::Database db(DBNAME);
    std::string api_token("");
    kopsik::error err = db.CurrentAPIToken(&api_token);
    if (err != kopsik::noError) {
        strncpy(errmsg, "Please log in", errlen);
        return KOPSIK_API_FAILURE;
    }
    kopsik::User user;
    err = db.LoadUserByAPIToken(api_token, &user, true);
    if (err != kopsik::noError) {
        err.copy(errmsg, errlen);
        return KOPSIK_API_FAILURE;
    }
    // FIXME:
    return KOPSIK_API_SUCCESS;
}

kopsik_api_result kopsik_set_api_token(char *errmsg, unsigned int errlen,
        const char *in_api_token) {
    if (!in_api_token) {
        strncpy(errmsg, "Invalid api_token pointer", errlen);
        return KOPSIK_API_FAILURE;
    }
    std::string api_token(in_api_token);
    if (api_token.empty()) {
        strncpy(errmsg, "Emtpy API token", errlen);
        return KOPSIK_API_FAILURE;
    }
    kopsik::Database db(DBNAME);
    kopsik::error err = db.SetCurrentAPIToken(api_token);
    if (err != kopsik::noError) {
        err.copy(errmsg, errlen);
        return KOPSIK_API_FAILURE;
    }
    return KOPSIK_API_SUCCESS;
}

kopsik_api_result kopsik_sync(char *errmsg, unsigned int errlen) {
    // FIXME:
    return KOPSIK_API_SUCCESS;
}

kopsik_api_result kopsik_running_time_entry(char *errmsg, unsigned int errlen,
        KopsikTimeEntry *out_time_entry, int *is_tracking) {
    if (!out_time_entry) {
        strncpy(errmsg, "Invalid time entry pointer", errlen);
        return KOPSIK_API_FAILURE;
    }
    if (!is_tracking) {
        strncpy(errmsg, "Invalid is_tracking pointer", errlen);
        return KOPSIK_API_FAILURE;
    }
    *is_tracking = 0;
    // FIXME:
    return KOPSIK_API_SUCCESS;
}

kopsik_api_result kopsik_dirty_models(char *errmsg, unsigned int errlen,
        KopsikDirtyModels *out_dirty_models) {
    if (!out_dirty_models) {
        strncpy(errmsg, "Invalid dirty models pointer", errlen);
        return KOPSIK_API_FAILURE;
    }
    // FIXME:
    return KOPSIK_API_SUCCESS;
}

kopsik_api_result kopsik_start(char *errmsg, unsigned int errlen,
        KopsikTimeEntry *out_time_entry) {
    if (!out_time_entry) {
        strncpy(errmsg, "Invalid time entry pointer", errlen);
        return KOPSIK_API_FAILURE;
    }
    // FIXME:
    return KOPSIK_API_SUCCESS;
}

kopsik_api_result kopsik_stop(char *errmsg, unsigned int errlen,
        KopsikTimeEntry *out_time_entry) {
    if (!out_time_entry) {
        strncpy(errmsg, "Invalid time entry pointer", errlen);
        return KOPSIK_API_FAILURE;
    }
    // FIXME:
    return KOPSIK_API_SUCCESS;
}
