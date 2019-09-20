// Copyright 2014 Toggl Desktop developers.

#include "../src/task.h"

#include <sstream>

namespace toggl {

std::string Task::String() const {
    std::stringstream ss;
    ss  << "ID=" << ID()
        << " local_id=" << LocalID()
        << " name=" << name_
        << " wid=" << wid_
        << " pid=" << pid_;
    return ss.str();
}

void Task::SetPID(const Poco::UInt64 value) {
    if (pid_ != value) {
        pid_ = value;
        SetDirty();
    }
}

void Task::SetWID(const Poco::UInt64 value) {
    if (wid_ != value) {
        wid_ = value;
        SetDirty();
    }
}

void Task::SetName(const std::string &value) {
    if (name_ != value) {
        name_ = value;
        SetDirty();
    }
}

void Task::SetActive(const bool value) {
    if (active_ != value) {
        active_ = value;
        SetDirty();
    }
}

void Task::LoadFromJSON(Json::Value data) {
    SetID(data["id"].asUInt64());
    SetName(data["name"].asString());
    SetPID(data["pid"].asUInt64());
    SetWID(data["wid"].asUInt64());
    SetActive(data["active"].asBool());
}

std::string Task::ModelName() const {
    return kModelTask;
}

std::string Task::ModelURL() const {
    return "/api/v9/tasks";
}

}   // namespace toggl
