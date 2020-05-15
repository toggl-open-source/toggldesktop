// Copyright 2014 Toggl Desktop developers.

#include "model/tag.h"

#include <sstream>

#include "const.h"

namespace toggl {

std::string Tag::String() const {
    std::stringstream ss;
    ss  << "ID=" << ID()
        << " local_id=" << LocalID()
        << " name=" << Name()
        << " wid=" << WID()
        << " guid=" << GUID();
    return ss.str();
}

void Tag::LoadFromJSON(Json::Value data) {
    ID.Set(data["id"].asUInt64());
    Name.Set(data["name"].asString());
    WID.Set(data["wid"].asUInt64());
}

std::string Tag::ModelName() const {
    return kModelTag;
}

std::string Tag::ModelURL() const {
    return "/api/v9/tags";
}

}   // namespace toggl
