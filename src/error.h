// Copyright 2014 Toggl Desktop developers.

#ifndef SRC_ERROR_H_
#define SRC_ERROR_H_

#include "./types.h"

namespace toggl {

bool IsNetworkingError(const error);
bool IsUserError(const error);

}  // namespace toggl

#endif  // SRC_ERROR_H_
