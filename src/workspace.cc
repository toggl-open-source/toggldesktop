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

}   // namespace kopsik
