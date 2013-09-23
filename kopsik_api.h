// Copyright 2013 Tanel Lebedev

#ifndef KOPSIK_API_H_
#define KOPSIK_API_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <errno.h>
#include <stddef.h>
#include <stdio.h>

#define KOPSIK_EXPORT

struct KopsikTimeEntry {
  const char *Description;
};
typedef struct KopsikTimeEntry KopsikTimeEntry;

struct KopsikUser {
  const char *Fullname;
};
typedef struct KopsikUser KopsikUser;

struct KopsikDirtyModels {
  int TimeEntries;
};
typedef struct KopsikDirtyModels KopsikDirtyModels;

typedef int kopsik_api_result;
#define KOPSIK_API_SUCCESS 0
#define KOPSIK_API_FAILURE 1

KOPSIK_EXPORT void kopsik_version(
int *major, int *minor, int *patch);

KOPSIK_EXPORT kopsik_api_result kopsik_current_user(
  char *errmsg, unsigned int errlen, KopsikUser *out_user);

KOPSIK_EXPORT kopsik_api_result kopsik_set_api_token(
  char *errmsg, unsigned int errlen, const char *in_api_token);

KOPSIK_EXPORT kopsik_api_result kopsik_sync(
  char *errmsg, unsigned int errlen);

KOPSIK_EXPORT kopsik_api_result kopsik_running_time_entry(
  char *errmsg, unsigned int errlen, KopsikTimeEntry *out_time_entry);

KOPSIK_EXPORT kopsik_api_result kopsik_dirty_models(
  char *errmsg, unsigned int errlen, KopsikDirtyModels *out_dirty_models);

KOPSIK_EXPORT kopsik_api_result kopsik_start(
  char *errmsg, unsigned int errlen, KopsikTimeEntry *out_time_entry);

KOPSIK_EXPORT kopsik_api_result kopsik_stop(
  char *errmsg, unsigned int errlen, KopsikTimeEntry *out_time_entry);

#undef KOPSIK_EXPORT

#ifdef __cplusplus
}
#endif

#endif  // KOPSIK_API_H_
