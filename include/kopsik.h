// Copyright 2013 Tanel Lebedev

#ifndef KOPSIK_H_INCLUDED_
#define KOPSIK_H_INCLUDED_

#ifdef __cplusplus
extern "C" {
#endif

#include <errno.h>
#include <stddef.h>
#include <stdio.h>

#define KOPSIK_EXPORT

struct HTimeEntry;
typedef struct HTimeEntry HTimeEntry;

struct HUser {
  const char *Fullname;
};

typedef struct HUser HUser;

KOPSIK_EXPORT void kopsik_version (int *major, int *minor, int *patch);
KOPSIK_EXPORT int kopsik_current_user (char *errmsg, unsigned int errlen, HUser *out);

/*
KOPSIK_EXPORT int kopsik_time_entry_start (char *errmsg, unsigned int errlen, HTimeEntry *out);
KOPSIK_EXPORT int kopsik_time_entry_stop (char *errmsg, unsigned int errlen, HTimeEntry *out);
KOPSIK_EXPORT int kopsik_time_entry_running (char *errmsg, unsigned int errlen, HTimeEntry *out);
KOPSIK_EXPORT int kopsik_time_entry_list (char *errmsg, unsigned int errlen, HTimeEntry *out[]);
*/

#undef KOPSIK_EXPORT

#ifdef __cplusplus
}
#endif

#endif //
