// Copyright 2014 Toggl Desktop developers.

#include "./task.h"

#include <sstream>

namespace kopsik {

std::string Task::String() {
    std::stringstream ss;
    ss  << "ID=" << id_
        << " local_id=" << local_id_
        << " name=" << name_
        << " wid=" << wid_
        << " pid=" << pid_;
    return ss.str();
}

void Task::SetID(Poco::UInt64 value) {
    if (id_ != value) {
        id_ = value;
        dirty_ = true;
    }
}

void Task::SetPID(Poco::UInt64 value) {
    if (pid_ != value) {
        pid_ = value;
        dirty_ = true;
    }
}

void Task::SetWID(Poco::UInt64 value) {
    if (wid_ != value) {
        wid_ = value;
        dirty_ = true;
    }
}

void Task::SetUID(Poco::UInt64 value) {
    if (uid_ != value) {
        uid_ = value;
        dirty_ = true;
    }
}

void Task::SetName(std::string value) {
    if (name_ != value) {
        name_ = value;
        dirty_ = true;
    }
}

}   // namespace kopsik
