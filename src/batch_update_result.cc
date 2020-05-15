// Copyright 2014 Toggl Desktop developers.

#include "batch_update_result.h"

#include <sstream>
#include <cstring>

#include "model/base_model.h"

namespace toggl {

error BatchUpdateResult::Error() const {
    if (StatusCode >= 200 && StatusCode < 300) {
        return noError;
    }
    if ("null" != Body) {
        return Body;
    }
    std::stringstream ss;
    ss  << "Request failed with status code "
        << StatusCode;
    return ss.str();
}

std::string BatchUpdateResult::String() const {
    std::stringstream ss;
    ss  << "batch update result GUID: " << GUID
        << ", StatusCode: " << StatusCode
        << ", ContentType: " << ContentType
        << ", Body: " << Body;
    return ss.str();
}

bool BatchUpdateResult::ResourceIsGone() const {
    return ("DELETE" == Method || 404 == StatusCode);
}

void BatchUpdateResult::LoadFromJSON(Json::Value n) {
    StatusCode = n["status"].asInt();
    Body = n["body"].asString();
    GUID = n["guid"].asString();
    ContentType = n["content_type"].asString();
    Method = n["method"].asString();
}

// Iterate through response array, parse response bodies.
// Collect errors into a vector.
void BatchUpdateResult::ProcessResponseArray(
    std::vector<BatchUpdateResult> * const results,
    std::map<std::string, BaseModel *> *models,
    std::vector<error> *errors) {

    poco_check_ptr(results);
    poco_check_ptr(models);
    poco_check_ptr(errors);

    for (std::vector<BatchUpdateResult>::const_iterator it = results->begin();
            it != results->end();
            ++it) {
        BatchUpdateResult result = *it;

        logger().debug(result.String());

        if (result.GUID.empty()) {
            logger().error("Batch update result has no GUID");
            continue;
        }

        BaseModel *model = (*models)[result.GUID];
        if (!model) {
            logger().warning("Server response includes a model we don't have! GUID=", result.GUID);
            continue;
        }
        error err = model->ApplyBatchUpdateResult(&result);
        if (err != noError) {
            model->Unsynced.Set(true);
            errors->push_back(err);
        }
    }
}

Logger BatchUpdateResult::logger() {
    return { "BatchUpdateResult" };
}

error BatchUpdateResult::ParseResponseArray(
    const std::string &response_body,
    std::vector<BatchUpdateResult> *responses) {

    poco_check_ptr(responses);

    // There seem to be cases where response body is 0.
    // Must investigate further.
    if (response_body.empty()) {
        logger().warning("Response is empty!");
        return noError;
    }

    logger().debug(response_body);

    Json::Value root;
    Json::Reader reader;
    if (!reader.parse(response_body, root)) {
        return error("error parsing batch update response");
    }

    for (unsigned int i = 0; i < root.size(); i++) {
        BatchUpdateResult result;
        result.LoadFromJSON(root[i]);
        responses->push_back(result);
    }

    return noError;
}

}   // namespace toggl
