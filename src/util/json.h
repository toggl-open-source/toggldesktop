// Copyright 2020 Toggl Desktop developers.

// helper methods for Json usage

#ifndef JSON_H
#define JSON_H

#include <json/json.h>

namespace toggl {

class JsonHelper {
public:
    template <class T>
    static T convert(const ::Json::Value &json) {
        return json.as<T>();
    }
};

template <> std::vector<std::string> JsonHelper::convert(const ::Json::Value &json);

} // namespace toggl

#endif // JSON_H
