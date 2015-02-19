// Copyright 2014 Toggl Desktop developers.

#ifndef SRC_ERROR_H_
#define SRC_ERROR_H_

#include <string>

#include "./types.h"

namespace toggl {

bool IsNetworkingError(const error);
bool IsUserError(const error);
std::string MakeErrorActionable(const error);

}  // namespace toggl

#endif  // SRC_ERROR_H_
