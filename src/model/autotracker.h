// Copyright 2015 Toggl Desktop developers.

#ifndef SRC_AUTOTRACKER_H_
#define SRC_AUTOTRACKER_H_

#include <string>
#include <sstream>
#include <vector>

#include <Poco/Types.h>

#include "base_model.h"
#include "timeline_event.h"

namespace toggl {

class TOGGL_INTERNAL_EXPORT AutotrackerRule : public BaseModel {
    AutotrackerRule(ProtectedBase *container)
        : BaseModel(container)
    {}
 public:
    friend class ProtectedBase;

    virtual ~AutotrackerRule() {}

    bool Matches(locked<TimelineEvent> &event) const;

    const std::string &Term() const;
    void SetTerm(const std::string &value);

    const Poco::UInt64 &PID() const;
    void SetPID(const Poco::UInt64 value);

    const Poco::UInt64 &TID() const;
    void SetTID(const Poco::UInt64 value);

    // Override BaseModel
    std::string String() const override;
    std::string ModelName() const override;
    std::string ModelURL() const override;

 private:
    std::string term_ { "" };
    Poco::UInt64 pid_ { 0 };
    Poco::UInt64 tid_ { 0 };
};

};  // namespace toggl

#endif  // SRC_AUTOTRACKER_H_
