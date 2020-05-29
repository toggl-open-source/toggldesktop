// Copyright 2015 Toggl Desktop developers.

#include "model/obm_action.h"

#include <sstream>

#include "util/formatter.h"

namespace toggl {

std::string ObmAction::String() const {
    std::stringstream ss;
    ss  << "ID=" << ID()
        << " local_id=" << LocalID()
        << " experiment_id=" << ExperimentID()
        << " key=" << Key()
        << " value=" << Value();
    return ss.str();
}

void ObmAction::SetExperimentID(const Poco::UInt64 value) {
    if (ExperimentID() != value) {
        ExperimentID.Set(value);
        SetDirty();
    }
}

void ObmAction::SetKey(const std::string &value) {
    if (Key() != value) {
        Key.Set(value);
        SetDirty();
    }
}

void ObmAction::SetValue(const std::string &value) {
    if (Value() != value) {
        Value.Set(value);
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

std::string ObmExperiment::String() const {
    std::stringstream ss;
    ss  << "ID=" << ID()
        << " local_id=" << LocalID()
        << " nr=" << Nr()
        << " has seen=" << HasSeen()
        << " included=" << Included()
        << " actions=" << Actions();
    return ss.str();
}

void ObmExperiment::SetNr(const Poco::UInt64 value) {
    if (Nr() != value) {
        Nr.Set(value);
        SetDirty();
    }
}

void ObmExperiment::SetHasSeen(const bool value) {
    if (HasSeen() != value) {
        HasSeen.Set(value);
        SetDirty();
    }
}

void ObmExperiment::SetIncluded(const bool value) {
    if (Included() != value) {
        Included.Set(value);
        SetDirty();
    }
}

void ObmExperiment::SetActions(const std::string &value) {
    if (Actions() != value) {
        Actions.Set(value);
        SetDirty();
    }
}

std::string ObmExperiment::ModelName() const {
    return kModelObmExperiment;
}

std::string ObmExperiment::ModelURL() const {
    return "";
}

}   // namespace toggl
