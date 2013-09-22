
#include <cstring>

#include "include/kopsik.h"
#include "database.h"
#include "toggl_api_client.h"

void kopsik_version (int *major, int *minor, int *patch) {
	*major = 0;
	*minor = 1;
	*patch = 0;
}

int kopsik_current_user (char *errmsg, unsigned int errlen, HUser *out) {
    char* apiToken = getenv("TOGGL_API_TOKEN");
    if (!apiToken) {
      strncpy(errmsg, "Please set TOGGL_API_TOKEN in environment", errlen);
      return 1;
    }

    kopsik::Database db("kopsik.db");
    kopsik::User user;

    kopsik::error err = db.Load(apiToken, &user, true);
    if (err != kopsik::noError) {
        err.copy(errmsg, errlen);
        return 1;
    }
    return 0;
}
