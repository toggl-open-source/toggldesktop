// Copyright 2014 Toggl Desktop developers.

#include "./project.h"

#include <sstream>
#include <ctime>

#include "Poco/String.h"
#include "Poco/NumberParser.h"

namespace kopsik {

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
      << " guid=" << GUID()
      << " active=" << active_;
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
  poco_assert(data);

  JSONNODE_ITERATOR current_node = json_begin(data);
  JSONNODE_ITERATOR last_node = json_end(data);
  while (current_node != last_node) {
    json_char *node_name = json_name(*current_node);
    if (strcmp(node_name, "id") == 0) {
      SetID(json_as_int(*current_node));
    } else if (strcmp(node_name, "name") == 0) {
      SetName(std::string(json_as_string(*current_node)));
    } else if (strcmp(node_name, "guid") == 0) {
      SetGUID(std::string(json_as_string(*current_node)));
    } else if (strcmp(node_name, "wid") == 0) {
      SetWID(json_as_int(*current_node));
    } else if (strcmp(node_name, "cid") == 0) {
      SetCID(json_as_int(*current_node));
    } else if (strcmp(node_name, "color") == 0) {
      SetColor(std::string(json_as_string(*current_node)));
    } else if (strcmp(node_name, "active") == 0) {
      SetActive(json_as_bool(*current_node));
    } else if (strcmp(node_name, "billable") == 0) {
      SetBillable(json_as_bool(*current_node));
    }
    ++current_node;
  }
}

bool Project::IsDuplicateResourceError(const kopsik::error err) const {
  return (std::string::npos !=
      std::string(err).find("Name has already been taken"));
}

}   // namespace kopsik
