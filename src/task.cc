// Copyright 2014 Toggl Desktop developers.

#include "./task.h"

#include <sstream>

namespace kopsik {

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

void Task::SetName(const std::string value) {
    if (name_ != value) {
        name_ = value;
        SetDirty();
    }
}

}   // namespace kopsik
