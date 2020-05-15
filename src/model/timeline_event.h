// Copyright 2014 Toggl Desktop developers.

#ifndef SRC_TIMELINE_EVENT_H_
#define SRC_TIMELINE_EVENT_H_

#include <time.h>

#include <sstream>
#include <string>

#include "model/base_model.h"
#include "util/formatter.h"

namespace toggl {

class TOGGL_INTERNAL_EXPORT TimelineEvent : public BaseModel, public TimedEvent {
 public:
    TimelineEvent() : BaseModel() {}

    virtual ~TimelineEvent() {}

    virtual bool IsDirty() const override {
        return BaseModel::IsDirty() || IsAnyPropertyDirty(Title, Filename, StartTime, EndTime, Idle, Chunked, Uploaded);
    }
    virtual void ClearDirty() override {
        BaseModel::ClearDirty();
        AllPropertiesClearDirty(Title, Filename, StartTime, EndTime, Idle, Chunked, Uploaded);
    }

    Property<std::string> Title { "" };
    Property<std::string> Filename { "" };
    Property<Poco::Int64> StartTime { 0 };
    Property<Poco::Int64> EndTime { 0 };
    Property<bool> Idle { false };
    Property<bool> Chunked { false };
    Property<bool> Uploaded { false };

    bool VisibleToUser() const {
        return !Uploaded() && !DeletedAt() && Chunked();
    }

    // Override TimedEvent
    const Poco::Int64 &Start() const;
    // Duration is now actually computed each time, it's not necessary to store it
    const Poco::Int64 &Duration() const;

    // Override BaseModel

    std::string String() const override;
    std::string ModelName() const override;
    std::string ModelURL() const override;
    Json::Value SaveToJSON(int apiVersion = 8) const override;
};

}  // namespace toggl

#endif  // SRC_TIMELINE_EVENT_H_
