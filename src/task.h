// Copyright 2014 Toggl Desktop developers.

#ifndef SRC_TASK_H_
#define SRC_TASK_H_

#include <string>

#include "Poco/Types.h"

#include "./base_model.h"

namespace kopsik {

  class Task : public BaseModel {
  public:
    Task()
      : BaseModel()
      , name_("")
      , wid_(0)
      , pid_(0) {}

    std::string Name() const { return name_; }
    void SetName(const std::string value);

    Poco::UInt64 WID() const { return wid_; }
    void SetWID(const Poco::UInt64 value);

    Poco::UInt64 PID() const { return pid_; }
    void SetPID(const Poco::UInt64 value);

    std::string String() const;

    std::string ModelName() const { return "task"; }
    std::string ModelURL() const { return "/api/v8/tasks"; }

  private:
    std::string name_;
    Poco::UInt64 wid_;
    Poco::UInt64 pid_;
  };

}  // namespace kopsik

#endif  // SRC_TASK_H_
