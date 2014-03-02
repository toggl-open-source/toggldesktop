// Copyright 2014 Toggl Desktop developers.

#ifndef SRC_BATCH_UPDATE_RESULT_H_
#define SRC_BATCH_UPDATE_RESULT_H_

#include <string>

#include "./types.h"

#include "Poco/Types.h"

namespace kopsik {

  class BatchUpdateResult {
    public:
      BatchUpdateResult()
        : StatusCode(0)
        , Body("")
        , GUID("")
        , ContentType("") {}
      Poco::Int64 StatusCode;
      std::string Body;
      std::string GUID;  // must match the BatchUpdate GUID
      std::string ContentType;
      std::string Method;

      error Error() const;
      std::string String() const;
      bool ResourceIsGone() const;
  };

}  // namespace kopsik

#endif  // SRC_BATCH_UPDATE_RESULT_H_
