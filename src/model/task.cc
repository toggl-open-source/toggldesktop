// Copyright 2014 Toggl Track developers.

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

void Task::SetPID(Poco::UInt64 value) {
    if (PID.Set(value))
        SetDirty();
}

void Task::SetWID(Poco::UInt64 value) {
    if (WID.Set(value))
        SetDirty();
}

void Task::SetName(const std::string &value) {
    if (Name.Set(value))
        SetDirty();
}

void Task::SetActive(bool value) {
    if (Active.Set(value))
        SetDirty();
}

void Task::LoadFromJSON(const Json::Value &data) {
    SetID(data["id"].asUInt64());
    SetName(data["name"].asString());
    if (data.isMember("pid"))
        SetPID(data["pid"].asUInt64());
    else
        SetPID(data["project_id"].asUInt64());
    if (data.isMember("wid"))
        SetWID(data["wid"].asUInt64());
    else
        SetWID(data["workspace_id"].asUInt64());
    SetActive(data["active"].asBool());
}

std::string Task::ModelName() const {
    return kModelTask;
}

std::string Task::ModelURL() const {
    return "/api/v9/tasks";
}

}   // namespace toggl
