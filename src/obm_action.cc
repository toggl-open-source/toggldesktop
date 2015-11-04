// Copyright 2015 Toggl Desktop developers.

#include "../src/obm_action.h"

#include <sstream>

#include "../src/formatter.h"

namespace toggl {

std::string ObmAction::String() const {
    std::stringstream ss;
    ss  << "ID=" << ID()
        << " local_id=" << LocalID()
        << " experiment_id=" << experiment_id_
        << " key=" << key_
        << " value=" << value_;
    return ss.str();
}

void ObmAction::SetExperimentID(const Poco::UInt64 value) {
    if (experiment_id_ != value) {
        experiment_id_ = value;
        SetDirty();
    }
}

void ObmAction::SetKey(const std::string value) {
    if (key_ != value) {
        key_ = value;
        SetDirty();
    }
}

void ObmAction::SetValue(const std::string value) {
    if (value_ != value) {
        value_ = value;
        SetDirty();
    }
}

std::string ObmAction::ModelName() const {
    return kModelObmAction;
}

std::string ObmAction::ModelURL() const {
    return "/api/v9/obm/actions";
}

Json::Value ObmAction::SaveToJSON() const {
    Json::Value n;
    n["experiment_id"] = Json::UInt64(ExperimentID());
    n["key"] = Formatter::EscapeJSONString(Key());
    n["value"] = Formatter::EscapeJSONString(Value());
    return n;
}

}   // namespace toggl
