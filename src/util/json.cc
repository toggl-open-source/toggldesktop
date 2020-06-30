// Copyright 2020 Toggl Desktop developers.

#include "json.h"

#include "model/time_entry.h"

namespace toggl {

template<>
std::vector<std::string> JsonHelper::convert(const Json::Value &json) {
    std::vector<std::string> ret;
    for (auto i : json) {
        ret.push_back(i.asString());
    }
    return ret;
}

} // namespace toggl
