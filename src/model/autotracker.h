// Copyright 2015 Toggl Desktop developers.

#ifndef SRC_AUTOTRACKER_H_
#define SRC_AUTOTRACKER_H_

#include <string>
#include <sstream>
#include <vector>

#include <Poco/Types.h>

#include "model/base_model.h"
#include "model/timeline_event.h"

namespace toggl {

class TOGGL_INTERNAL_EXPORT AutotrackerRule : public BaseModel {
 public:
    AutotrackerRule() : BaseModel() {}
    virtual ~AutotrackerRule() {}

    virtual bool IsDirty() const override {
        return BaseModel::IsDirty() || IsAnyPropertyDirty(Term, PID, TID);
    }
    virtual void ClearDirty() override {
        BaseModel::ClearDirty();
        AllPropertiesClearDirty(Term, PID, TID);
    }

    Property<std::string> Term { "" };
    Property<Poco::UInt64> PID { 0 };
    Property<Poco::UInt64> TID { 0 };

    bool Matches(const TimelineEvent &event) const;

    // Override BaseModel
    std::string String() const override;
    std::string ModelName() const override;
    std::string ModelURL() const override;
};

};  // namespace toggl

#endif  // SRC_AUTOTRACKER_H_
