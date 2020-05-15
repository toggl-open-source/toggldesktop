// Copyright 2014 Toggl Desktop developers.

#include "model/task.h"

#include <sstream>

namespace toggl {

std::string Task::String() const {
    std::stringstream ss;
    ss  << "ID=" << ID()
        << " local_id=" << LocalID()
        << " name=" << Name()
        << " wid=" << WID()
        << " pid=" << PID();
    return ss.str();
}

void Task::LoadFromJSON(Json::Value data) {
    ID.Set(data["id"].asUInt64());
    Name.Set(data["name"].asString());
    PID.Set(data["pid"].asUInt64());
    WID.Set(data["wid"].asUInt64());
    Active.Set(data["active"].asBool());
}

std::string Task::ModelName() const {
    return kModelTask;
}

std::string Task::ModelURL() const {
    return "/api/v9/tasks";
}

}   // namespace toggl
