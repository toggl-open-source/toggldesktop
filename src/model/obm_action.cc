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

std::string ObmAction::ModelName() const {
    return kModelObmAction;
}

std::string ObmAction::ModelURL() const {
    return "/api/v9/obm/actions";
}

Json::Value ObmAction::SaveToJSON(int apiVersion) const {
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

std::string ObmExperiment::ModelName() const {
    return kModelObmExperiment;
}

std::string ObmExperiment::ModelURL() const {
    return "";
}

}   // namespace toggl
