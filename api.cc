
#include "include/kopsik.h"
#include "database.h"

void kopsik_version (int *major, int *minor, int *patch) {
	*major = 0;
	*minor = 1;
	*patch = 0;
}

kopsik_error kopsik_current_user (HUser *out) {
    char* apiToken = getenv("TOGGL_API_TOKEN");
    if (!apiToken) {
        return "Please set TOGGL_API_TOKEN in environment";
    }

	kopsik::Database db("kopsik.db");
 	kopsik::User user;

  	kopsik::error err = db.Load(apiToken, &user, true);
  	if (err != kopsik::noError) {
  		return err.c_str();
  	}
  	return NO_ERROR;
}
