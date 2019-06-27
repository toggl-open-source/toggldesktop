// Copyright 2019 Toggl Desktop developers.

#ifndef SRC_JSON_HELPER_H_
#define SRC_JSON_HELPER_H_

#include <memory>
#include <json/json.h>

namespace toggl {

class JsonReader {
public:
    JsonReader(Json::CharReader *internal);

    bool parse(std::string document, Json::Value *value, std::string *errors = nullptr);
private:
    Json::CharReader *internal;
};

class JsonWriter {
public:
    JsonWriter(Json::StreamWriter *internal);

    std::string write(const Json::Value &value);
private:
    Json::StreamWriter *internal;
};

class JsonHelper {
public:
    static toggl::JsonReader *reader();
    static toggl::JsonWriter *writer();
};

}  // namespace toggl

#endif // SRC_JSON_HELPER_H_
