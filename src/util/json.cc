// Copyright 2020 Toggl Track developers.

#include "json.h"

#include "model/time_entry.h"

namespace toggl {

template<>
std::vector<std::string> JsonHelper::convert(const Json::Value &json) {
    std::vector<std::string> ret;
    for (auto i : json) {
        if (!i.asString().empty())
            ret.push_back(i.asString());
    }
    return ret;
}

template<>
Poco::UInt64 JsonHelper::convert(const Json::Value& json) {
    return json.asUInt64();
}

template<>
Poco::Int64 JsonHelper::convert(const Json::Value& json) {
    return json.asInt64();
}

template<>
std::string JsonHelper::convert(const Json::Value& json) {
    return json.asString();
}

template<>
bool JsonHelper::convert(const Json::Value& json) {
    return json.asBool();
}

} // namespace toggl
