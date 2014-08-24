// Copyright 2014 Toggl Desktop developers.

#include "./project.h"

#include <sstream>
#include <ctime>

#include "Poco/String.h"
#include "Poco/NumberParser.h"

#include "./formatter.h"

namespace toggl {

const char *known_colors[] = {
    "#4dc3ff", "#bc85e6", "#df7baa", "#f68d38", "#b27636",
    "#8ab734", "#14a88e", "#268bb5", "#6668b4", "#a4506c",
    "#67412c", "#3c6526", "#094558", "#bc2d07", "#999999"
};
template<typename T, size_t N> T *end(T (&ra)[N]) {
    return ra + N;
}
std::vector<std::string> Project::color_codes(known_colors, end(known_colors));

std::string Project::String() const {
    std::stringstream ss;
    ss  << "ID=" << ID()
        << " local_id=" << LocalID()
        << " name=" << name_
        << " wid=" << wid_
        << " cid=" << cid_
        << " guid=" << GUID()
        << " active=" << active_
        << " public=" << private_
        << " color=" << color_
        << " billable=" << billable_;
    return ss.str();
}

std::string Project::UppercaseName() const {
    return Poco::toUpper(name_);
}

void Project::SetActive(const bool value) {
    if (active_ != value) {
        active_ = value;
        SetDirty();
    }
}

void Project::SetPrivate(const bool value) {
    if (private_ != value) {
        private_ = value;
        SetDirty();
    }
}

void Project::SetName(const std::string value) {
    if (name_ != value) {
        name_ = value;
        SetDirty();
    }
}

void Project::SetBillable(const bool value) {
    if (billable_ != value) {
        billable_ = value;
        SetDirty();
    }
}

void Project::SetColor(const std::string value) {
    if (color_ != value) {
        color_ = value;
        SetDirty();
    }
}

std::string Project::ColorCode() const {
    int index(0);
    if (!Poco::NumberParser::tryParse(Color(), index)) {
        return color_codes.back();
    }
    if (!index) {
        return color_codes.back();
    }
    return color_codes[index % color_codes.size()];
}

void Project::SetWID(const Poco::UInt64 value) {
    if (wid_ != value) {
        wid_ = value;
        SetDirty();
    }
}

void Project::SetCID(const Poco::UInt64 value) {
    if (cid_ != value) {
        cid_ = value;
        SetDirty();
    }
}

void Project::LoadFromJSONNode(JSONNODE * const data) {
    poco_check_ptr(data);

    Poco::UInt64 id(0);
    std::string name("");
    Poco::UInt64 wid(0);
    Poco::UInt64 cid(0);
    std::string color("");
    bool active(false);
    bool billable(false);

    JSONNODE_ITERATOR current_node = json_begin(data);
    JSONNODE_ITERATOR last_node = json_end(data);
    while (current_node != last_node) {
        json_char *node_name = json_name(*current_node);
        if (strcmp(node_name, "id") == 0) {
            id = json_as_int(*current_node);
        } else if (strcmp(node_name, "name") == 0) {
            name = std::string(json_as_string(*current_node));
        } else if (strcmp(node_name, "guid") == 0) {
            SetGUID(std::string(json_as_string(*current_node)));
        } else if (strcmp(node_name, "wid") == 0) {
            wid = json_as_int(*current_node);
        } else if (strcmp(node_name, "cid") == 0) {
            cid = json_as_int(*current_node);
        } else if (strcmp(node_name, "color") == 0) {
            color = std::string(json_as_string(*current_node));
        } else if (strcmp(node_name, "active") == 0) {
            active = json_as_bool(*current_node) != 0;
        } else if (strcmp(node_name, "billable") == 0) {
            billable = json_as_bool(*current_node) != 0;
        }
        ++current_node;
    }

    SetID(id);
    SetName(name);
    SetWID(wid);
    SetCID(cid);
    SetColor(color);
    SetActive(active);
    SetBillable(billable);
}

JSONNODE *Project::SaveToJSONNode() const {
    JSONNODE *n = json_new(JSON_NODE);
    json_set_name(n, ModelName().c_str());
    if (ID()) {
        json_push_back(n, json_new_i("id", (json_int_t)ID()));
    }
    json_push_back(n, json_new_a("name",
                                 Formatter::EscapeJSONString(Name()).c_str()));
    json_push_back(n, json_new_i("wid", (json_int_t)WID()));
    json_push_back(n, json_new_a("guid", GUID().c_str()));
    json_push_back(n, json_new_i("cid", (json_int_t)CID()));
    json_push_back(n, json_new_b("billable", Billable()));
    json_push_back(n, json_new_b("is_private", IsPrivate()));
    json_push_back(n, json_new_i("ui_modified_at",
                                 (json_int_t)UIModifiedAt()));

    return n;
}

bool Project::DuplicateResource(const toggl::error err) const {
    return (std::string::npos !=
            std::string(err).find("Name has already been taken"));
}

bool Project::ResourceCannotBeCreated(const toggl::error err) const {
    return (std::string::npos != std::string(err).find(
        "User cannot add or edit projects in workspace"));
}

bool Project::clientIsInAnotherWorkspace(const toggl::error err) const {
    return (std::string::npos != std::string(err).find(
        "client is in another workspace"));
}

bool Project::ResolveError(const toggl::error err) {
    if (userCannotAccessWorkspace(err)) {
        SetWID(0);
        return true;
    }
    if (clientIsInAnotherWorkspace(err)) {
        SetCID(0);
        return true;
    }
    return false;
}

}   // namespace toggl
