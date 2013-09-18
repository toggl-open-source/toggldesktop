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

KOPSIK_EXPORT void kopsik_version (int *major, int *minor, int *patch);

struct HTimeEntry;
typedef struct HTimeEntry HTimeEntry;

KOPSIK_EXPORT void *kopsik_ctx_new (void);
KOPSIK_EXPORT void kopsik_ctx_shutdown (void *context);

#undef KOPSIK_EXPORT

#ifdef __cplusplus
}
#endif

#endif //
