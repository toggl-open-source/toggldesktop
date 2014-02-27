// Copyright 2014 Toggl Desktop developers.

#ifndef SRC_WORKSPACE_H_
#define SRC_WORKSPACE_H_

#include <string>

#include "Poco/Types.h"

#include "./base_model.h"

namespace kopsik {

  class Workspace : public BaseModel {
  public:
    Workspace()
      : BaseModel()
      , name_("")
      , premium_(false) {}

    std::string String() const;

    std::string Name() const { return name_; }
    void SetName(const std::string value);

    bool Premium() const { return premium_; }
    void SetPremium(const bool value);

  private:
    std::string name_;
    bool premium_;
  };

  bool CompareWorkspaceByName(Workspace *a, Workspace *b);

}  // namespace kopsik

#endif  // SRC_WORKSPACE_H_
