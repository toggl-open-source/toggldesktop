// Copyright 2014 Toggl Desktop developers.

#ifndef SRC_BATCH_UPDATE_RESULT_H_
#define SRC_BATCH_UPDATE_RESULT_H_

#include "types.h"

#include <string>
#include <vector>
#include <map>

#include <json/json.h>

#include <Poco/Types.h>

namespace toggl {

class BaseModel;

class BatchUpdateResult {
 public:
    BatchUpdateResult()
        : StatusCode(0)
    , Body("")
    , GUID("")
    , ContentType("") {}
    int64_t StatusCode;
    std::string Body;
    std::string GUID;  // must match the BatchUpdate GUID
    std::string ContentType;
    std::string Method;

    error Error() const;
    std::string String() const;
    bool ResourceIsGone() const;

    void LoadFromJSON(Json::Value value);

    static error ParseResponseArray(
        const std::string response_body,
        std::vector<BatchUpdateResult> *responses);
    static void ProcessResponseArray(
        std::vector<BatchUpdateResult> * const results,
        std::map<std::string, BaseModel *> *models,
        std::vector<error> *errors);
};

}  // namespace toggl

#endif  // SRC_BATCH_UPDATE_RESULT_H_
