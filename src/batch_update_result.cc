// Copyright 2014 Toggl Desktop developers.

#include "./batch_update_result.h"

#include <sstream>
#include <cstring>

#include "./base_model.h"

#include "Poco/Logger.h"

namespace kopsik {

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

void BatchUpdateResult::LoadFromJSONNode(JSONNODE * const n) {
    poco_check_ptr(n);

    StatusCode = 0;
    Body = "";
    GUID = "";
    ContentType = "";
    JSONNODE_ITERATOR i = json_begin(n);
    JSONNODE_ITERATOR e = json_end(n);
    while (i != e) {
        json_char *node_name = json_name(*i);
        if (strcmp(node_name, "status") == 0) {
            StatusCode = json_as_int(*i);
        } else if (strcmp(node_name, "body") == 0) {
            Body = std::string(json_as_string(*i));
        } else if (strcmp(node_name, "guid") == 0) {
            GUID = std::string(json_as_string(*i));
        } else if (strcmp(node_name, "content_type") == 0) {
            ContentType = std::string(json_as_string(*i));
        } else if (strcmp(node_name, "method") == 0) {
            Method = std::string(json_as_string(*i));
        }
        ++i;
    }
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

    Poco::Logger &logger = Poco::Logger::get("json");
    for (std::vector<BatchUpdateResult>::const_iterator it = results->begin();
            it != results->end();
            it++) {
        BatchUpdateResult result = *it;

        logger.debug(result.String());

        poco_assert(!result.GUID.empty());
        BaseModel *model = (*models)[result.GUID];
        poco_assert(model);

        error err = model->ApplyBatchUpdateResult(&result);
        if (err != noError) {
            errors->push_back(err);
        }
    }
}

void BatchUpdateResult::ParseResponseArray(
    const std::string response_body,
    std::vector<BatchUpdateResult> *responses) {

    poco_check_ptr(responses);

    Poco::Logger &logger = Poco::Logger::get("json");

    // There seem to be cases where response body is 0.
    // Must investigate further.
    if (response_body.empty()) {
        logger.warning("Response is empty!");
        return;
    }

    logger.debug(response_body);

    JSONNODE *response_array = json_parse(response_body.c_str());
    JSONNODE_ITERATOR i = json_begin(response_array);
    JSONNODE_ITERATOR e = json_end(response_array);
    while (i != e) {
        BatchUpdateResult result;
        result.LoadFromJSONNode(*i);
        responses->push_back(result);
        ++i;
    }
    json_delete(response_array);
}

}   // namespace kopsik
