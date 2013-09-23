// Copyright 2013 Tanel Lebedev

#include <cstring>

#include "include/kopsik.h"
#include "./database.h"

void kopsik_version(int *major, int *minor, int *patch) {
    *major = 0;
    *minor = 1;
    *patch = 0;
}

int kopsik_current_user(char *errmsg, unsigned int errlen, HUser *out) {
    char* apiToken = getenv("TOGGL_API_TOKEN");
    if (!apiToken) {
      strncpy(errmsg, "Please set TOGGL_API_TOKEN in environment", errlen);
      return 1;
    }

    kopsik::User user;

    kopsik::Database db("kopsik.db");

    kopsik::error err = db.LoadUserByAPIToken(apiToken, &user, true);
    if (err != kopsik::noError) {
        err.copy(errmsg, errlen);
        return 1;
    }
    return 0;
}
