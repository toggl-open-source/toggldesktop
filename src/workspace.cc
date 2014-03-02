// Copyright 2014 Toggl Desktop developers.

#include "./workspace.h"

#include <sstream>
#include <cstring>

namespace kopsik {

std::string Workspace::String() const {
  std::stringstream ss;
  ss  << "ID=" << ID()
      << " local_id=" << LocalID()
      << " name=" << name_;
  return ss.str();
}

void Workspace::SetName(const std::string value) {
  if (name_ != value) {
    name_ = value;
    SetDirty();
  }
}

void Workspace::SetPremium(const bool value) {
  if (premium_ != value) {
    premium_ = value;
    SetDirty();
  }
}

bool CompareWorkspaceByName(Workspace *a, Workspace *b) {
  return (strcmp(a->Name().c_str(), b->Name().c_str()) < 0);
}

void Workspace::LoadFromJSONNode(JSONNODE * const n) {
  poco_assert(n);

  JSONNODE_ITERATOR i = json_begin(n);
  JSONNODE_ITERATOR e = json_end(n);
  while (i != e) {
    json_char *node_name = json_name(*i);
    if (strcmp(node_name, "id") == 0) {
      SetID(json_as_int(*i));
    } else if (strcmp(node_name, "name") == 0) {
      SetName(std::string(json_as_string(*i)));
    } else if (strcmp(node_name, "premium") == 0) {
      SetPremium(json_as_bool(*i));
    }
    ++i;
  }
}

}   // namespace kopsik
