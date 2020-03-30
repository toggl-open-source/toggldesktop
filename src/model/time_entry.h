// Copyright 2014 Toggl Desktop developers.

#ifndef SRC_TIME_ENTRY_H_
#define SRC_TIME_ENTRY_H_

#include <string>
#include <vector>

#include "base_model.h"
#include "types.h"
#include "util/formatter.h"

#include <Poco/Types.h>

namespace toggl {

class TOGGL_INTERNAL_EXPORT TimeEntry : public BaseModel, public TimedEvent {
    inline static const std::string modelName{ kModelTimeEntry };
    inline static const Query query{
        Query::Table{"time_entries"},
        Query::Columns {
            { "description", false },
            { "wid", true },
            { "pid", false },
            { "tid", false },
            { "billable", true },
            { "duronly", true },
            { "ui_modified_at", false },
            { "start", true },
            { "stop", false },
            { "duration", true },
            { "tags", false },
            { "created_with", false },
            { "deleted_at", false },
            { "updated_at", false },
            { "project_guid", false },
            { "validation_error", false }
        },
        Query::Join{},
        Query::OrderBy{"start DESC"},
        &BaseModel::query
    };
    TimeEntry(ProtectedBase *container, Poco::Data::RecordSet &rs)
        : BaseModel(container, rs)
    {
        size_t ptr{ query.Offset() };
        load(rs, query.IsRequired(ptr), ptr, description_);
        ptr++;
        load(rs, query.IsRequired(ptr), ptr, wid_);
        ptr++;
        load(rs, query.IsRequired(ptr), ptr, pid_);
        ptr++;
        load(rs, query.IsRequired(ptr), ptr, tid_);
        ptr++;
        load(rs, query.IsRequired(ptr), ptr, billable_);
        ptr++;
        load(rs, query.IsRequired(ptr), ptr, duronly_);
        ptr++;
        // HACKS AHEAD
        Poco::Int64 modifiedAt;
        load(rs, query.IsRequired(ptr), ptr, modifiedAt);
        SetUIModifiedAt(modifiedAt);
        ptr++;
        load(rs, query.IsRequired(ptr), ptr, start_);
        ptr++;
        load(rs, query.IsRequired(ptr), ptr, stop_);
        ptr++;
        load(rs, query.IsRequired(ptr), ptr, duration_in_seconds_);
        ptr++;
        // MORE HACKS
        std::string tags;
        load(rs, query.IsRequired(ptr), ptr, tags);
        SetTags(tags);
        ptr++;
        load(rs, query.IsRequired(ptr), ptr, created_with_);
        ptr++;
        // HACK
        Poco::Int64 deletedAt;
        load(rs, query.IsRequired(ptr), ptr, deletedAt);
        SetDeletedAt(deletedAt);
        ptr++;
        // HACK
        Poco::Int64 updatedAt;
        load(rs, query.IsRequired(ptr), ptr, updatedAt);
        SetUpdatedAt(updatedAt);
        ptr++;
        load(rs, query.IsRequired(ptr), ptr, project_guid_);
        ptr++;
        error validationError;
        load(rs, query.IsRequired(ptr), ptr, validationError);
        SetValidationError(validationError);
        ptr++;

        ClearDirty();
        EnsureGUID();
        if (Dirty())
            SetUIModified();
    }
    TimeEntry(ProtectedBase *container)
        : BaseModel(container)
    {}
 public:
    friend class ProtectedBase;

    virtual ~TimeEntry() {}

    const Poco::Int64 &LastStartAt() const {
        return last_start_at_;
    }
    void SetLastStartAt(const Poco::Int64 value);

    std::vector<std::string> TagNames;

    const std::string Tags() const;
    void SetTags(const std::string &tags);

    const std::string TagsHash() const;

    const Poco::UInt64 &WID() const {
        return wid_;
    }
    void SetWID(const Poco::UInt64 value);

    const Poco::UInt64 &PID() const {
        return pid_;
    }
    void SetPID(const Poco::UInt64 value);

    const Poco::UInt64 &TID() const {
        return tid_;
    }
    void SetTID(const Poco::UInt64 value);

    const bool &Billable() const {
        return billable_;
    }
    void SetBillable(const bool value);

    const Poco::Int64 &DurationInSeconds() const {
        return duration_in_seconds_;
    }
    void SetDurationInSeconds(const Poco::Int64 value);

    const bool &DurOnly() const {
        return duronly_;
    }
    void SetDurOnly(const bool value);

    const std::string &Description() const {
        return description_;
    }
    void SetDescription(const std::string &value);

    std::string StartString() const;
    void SetStartString(const std::string &value);

    const Poco::Int64 &Start() const override {
        return start_;
    }
    void SetStart(const Poco::Int64 value);

    std::string StopString() const;
    void SetStopString(const std::string &value);

    const Poco::Int64 &Stop() const {
        return stop_;
    }
    void SetStop(const Poco::Int64 value);

    const std::string &CreatedWith() const {
        return created_with_;
    }
    void SetCreatedWith(const std::string &value);

    void DiscardAt(const Poco::Int64);

    bool IsToday() const;

    const std::string &ProjectGUID() const {
        return project_guid_;
    }
    void SetProjectGUID(const std::string &);

    // User-triggered changes to timer:
    void SetDurationUserInput(const std::string &);
    void SetStopUserInput(const std::string &);
    void SetStartUserInput(const std::string &, const bool);

    bool IsTracking() const {
        return duration_in_seconds_ < 0;
    }

    void StopTracking();

    // Override BaseModel

    std::string ModelName() const override;
    std::string ModelURL() const override;
    std::string String() const override;
    virtual error ResolveError(const error &err) override;
    void LoadFromJSON(Json::Value value) override;
    Json::Value SaveToJSON() const override;

    // Implement TimedEvent

    virtual const Poco::Int64 &Duration() const {
        return DurationInSeconds();
    }

    Poco::Int64 RealDurationInSeconds() const;

    bool isNotFound(const error &err) const;

    const std::string GroupHash() const;

 private:
    std::string description_ { "" };
    std::string created_with_ { "" };
    std::string project_guid_ { "" };
    Poco::UInt64 wid_ { 0 };
    Poco::UInt64 pid_ { 0 };
    Poco::UInt64 tid_ { 0 };
    Poco::Int64 start_ { 0 };
    Poco::Int64 stop_ { 0 };
    Poco::Int64 duration_in_seconds_ { 0 };
    Poco::Int64 last_start_at_ { 0 };
    bool billable_ { false };
    bool duronly_ { false };
    bool unsynced_ { false };

    bool setDurationStringHHMMSS(const std::string &value);
    bool setDurationStringHHMM(const std::string &value);
    bool setDurationStringMMSS(const std::string &value);

    void loadTagsFromJSON(Json::Value value);

    bool durationTooLarge(const error &err) const;
    bool startTimeWrongYear(const error &err) const;
    bool stopTimeMustBeAfterStartTime(const error &err) const;
    bool userCannotAccessTheSelectedProject(const error &err) const;
    bool userCannotAccessSelectedTask(const error &err) const;
    bool billableIsAPremiumFeature(const error &err) const;
    bool isMissingCreatedWith(const error &err) const;
};

}  // namespace toggl

#endif  // SRC_TIME_ENTRY_H_
