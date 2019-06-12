// Copyright 2014 Toggl Desktop developers.

#ifndef SRC_ERROR_H_
#define SRC_ERROR_H_

#include <string>

#include "./types.h"

namespace toggl {

bool IsNetworkingError(const error &err);
bool IsUserError(const error &err);
std::string MakeErrorActionable(const error &err);

}  // namespace toggl

#endif  // SRC_ERROR_H_
