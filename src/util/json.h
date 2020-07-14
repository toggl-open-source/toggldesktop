// Copyright 2020 Toggl Track developers.

// helper methods for Json usage

#ifndef JSON_H
#define JSON_H

#include <json/json.h>
#include <Poco/Types.h>

namespace toggl {

class JsonHelper {
public:
    template <class T>
    static T convert(const ::Json::Value &json);
};

template <> Poco::Int64 JsonHelper::convert(const ::Json::Value& json);
template <> Poco::UInt64 JsonHelper::convert(const ::Json::Value& json);
template <> std::string JsonHelper::convert(const ::Json::Value& json);
template <> bool JsonHelper::convert(const ::Json::Value& json);
template <> std::vector<std::string> JsonHelper::convert(const ::Json::Value &json);

} // namespace toggl

#endif // JSON_H
