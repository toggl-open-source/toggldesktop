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

typedef const char * kopsik_error;

#define NO_ERROR ""

KOPSIK_EXPORT void kopsik_version (int *major, int *minor, int *patch);

KOPSIK_EXPORT kopsik_error kopsik_current_user (HUser *out);

KOPSIK_EXPORT kopsik_error kopsik_time_entry_start (HTimeEntry *out);

KOPSIK_EXPORT kopsik_error kopsik_time_entry_stop (HTimeEntry *out);

KOPSIK_EXPORT kopsik_error kopsik_time_entry_running (HTimeEntry *out);

KOPSIK_EXPORT kopsik_error kopsik_time_entry_list (HTimeEntry *out[]);

#undef KOPSIK_EXPORT

#ifdef __cplusplus
}
#endif

#endif //
