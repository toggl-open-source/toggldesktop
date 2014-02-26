// Copyright 2014 Toggl Desktop developers.

#include "./client.h"

#include <sstream>
#include <cstring>

namespace kopsik {

std::string Client::String() {
  std::stringstream ss;
  ss  << "ID=" << id_
  << " local_id=" << local_id_
  << " name=" << name_
  << " wid=" << wid_
  << " guid=" << guid_;
  return ss.str();
}

void Client::SetID(Poco::UInt64 value) {
  if (id_ != value) {
    id_ = value;
    dirty_ = true;
  }
}

void Client::SetName(std::string value) {
  if (name_ != value) {
    name_ = value;
    dirty_ = true;
  }
}

void Client::SetUID(Poco::UInt64 value) {
  if (uid_ != value) {
    uid_ = value;
    dirty_ = true;
  }
}

void Client::SetGUID(std::string value) {
  if (guid_ != value) {
    guid_ = value;
    dirty_ = true;
  }
}

void Client::SetWID(Poco::UInt64 value) {
  if (wid_ != value) {
    wid_ = value;
    dirty_ = true;
  }
}

bool CompareClientByName(Client *a, Client *b) {
  return (strcmp(a->Name().c_str(), b->Name().c_str()) < 0);
}

}   // namespace kopsik
