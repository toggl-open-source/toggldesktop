// Copyright 2014 Toggl Desktop developers.

#include "./workspace.h"

#include <sstream>

namespace kopsik {

std::string Workspace::String() {
  std::stringstream ss;
  ss  << "ID=" << id_
      << " local_id=" << local_id_
      << " name=" << name_;
  return ss.str();
}

void Workspace::SetUID(Poco::UInt64 value) {
  if (uid_ != value) {
    uid_ = value;
    dirty_ = true;
  }
}

void Workspace::SetID(Poco::UInt64 value) {
  if (id_ != value) {
    id_ = value;
    dirty_ = true;
  }
}

void Workspace::SetName(std::string value) {
  if (name_ != value) {
    name_ = value;
    dirty_ = true;
  }
}

void Workspace::SetPremium(const bool value) {
  if (premium_ != value) {
    premium_ = value;
    dirty_ = true;
  }
}

bool CompareWorkspaceByName(Workspace *a, Workspace *b) {
  return (strcmp(a->Name().c_str(), b->Name().c_str()) < 0);
}

}   // namespace kopsik
