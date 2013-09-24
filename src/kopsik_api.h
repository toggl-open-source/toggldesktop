// Copyright 2013 Tanel Lebedev

#ifndef SRC_KOPSIK_API_H_
#define SRC_KOPSIK_API_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <errno.h>
#include <stddef.h>
#include <stdio.h>

#define KOPSIK_EXPORT

typedef struct {
  char *Description;
} TogglTimeEntry;

typedef struct {
  char *Fullname;
} TogglUser;

typedef struct {
  int TimeEntries;
} TogglDirtyModels;

typedef int kopsik_api_result;
#define KOPSIK_API_SUCCESS 0
#define KOPSIK_API_FAILURE 1

KOPSIK_EXPORT void kopsik_version(
  int *major, int *minor, int *patch);

// User API

KOPSIK_EXPORT TogglUser *kopsik_user_new();
KOPSIK_EXPORT void kopsik_user_delete(TogglUser *user);

KOPSIK_EXPORT kopsik_api_result kopsik_current_user(
  char *errmsg, unsigned int errlen, TogglUser *out_user);

KOPSIK_EXPORT kopsik_api_result kopsik_set_api_token(
  char *errmsg, unsigned int errlen, const char *in_api_token);

// Sync

KOPSIK_EXPORT kopsik_api_result kopsik_sync(
  char *errmsg, unsigned int errlen);
KOPSIK_EXPORT kopsik_api_result kopsik_dirty_models(
  char *errmsg, unsigned int errlen, TogglDirtyModels *out_dirty_models);

// Time tracking API

KOPSIK_EXPORT TogglTimeEntry *kopsik_time_entry_new();
KOPSIK_EXPORT kopsik_api_result kopsik_running_time_entry(
  char *errmsg, unsigned int errlen,
  TogglTimeEntry *out_time_entry, int *is_tracking);
KOPSIK_EXPORT kopsik_api_result kopsik_start(
  char *errmsg, unsigned int errlen, TogglTimeEntry *out_time_entry);
KOPSIK_EXPORT kopsik_api_result kopsik_stop(
  char *errmsg, unsigned int errlen, TogglTimeEntry *out_time_entry);
KOPSIK_EXPORT void kopsik_time_entry_delete(TogglTimeEntry *in_time_entry);

#undef KOPSIK_EXPORT

#ifdef __cplusplus
}
#endif

#endif  // SRC_KOPSIK_API_H_
