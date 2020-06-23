// Copyright 2014 Toggl Desktop developers.

#ifndef SRC_TIME_ENTRY_H_
#define SRC_TIME_ENTRY_H_

#include <string>
#include <vector>

#include "model/base_model.h"
#include "util/formatter.h"
#include "types.h"

#include <Poco/Types.h>

namespace toggl {

class TOGGL_INTERNAL_EXPORT TimeEntry : public BaseModel, public TimedEvent {
 public:
    TimeEntry() : BaseModel() {}
    virtual ~TimeEntry() {}

    Property<std::string> Description { "" };
    Property<std::string> CreatedWith { "" };
    Property<std::string> ProjectGUID { "" };
    Property<std::vector<std::string>> TagNames;
    Property<Poco::UInt64> WID { 0 };
    Property<Poco::UInt64> PID { 0 };
    Property<Poco::UInt64> TID { 0 };
    Property<Poco::Int64> StartTime { 0 };
    Property<Poco::Int64> StopTime { 0 };
    Property<Poco::Int64> DurationInSeconds { 0 };
    Property<Poco::Int64> LastStartAt { 0 };
    Property<bool> Billable { false };
    Property<bool> DurOnly { false };
    Property<bool> SkipPomodoro { false };

    void SetDescription(const std::string &value, bool userModified);
    void SetCreatedWith(const std::string &value);
    void SetProjectGUID(const std::string &value, bool userModified);

    const std::string Tags() const;
    void SetTags(const std::string &tags, bool userModified);
    const std::string TagsHash() const;

    void SetWID(Poco::UInt64 value);
    void SetPID(Poco::UInt64 value, bool userModified);
    void SetTID(Poco::UInt64 value, bool userModified);

    std::string StartString() const;
    void SetStartString(const std::string &value, bool userModified);
    void SetStartTime(Poco::Int64 value, bool userModified);

    std::string StopString() const;
    void SetStopString(const std::string &value, bool userModified);
    void SetStopTime(Poco::Int64 value, bool userModified);

    void SetDurationInSeconds(const Poco::Int64 value, bool userModified);
    void SetLastStartAt(Poco::Int64 value);

    void SetBillable(bool value, bool userModified);
    void SetDurOnly(bool value);
    void SetSkipPomodoro(bool value);

    // Derived information and modifiers
    bool IsToday() const;

    bool IsTracking() const {
        return DurationInSeconds() < 0;
    }

    Poco::Int64 RealDurationInSeconds() const;

    void DiscardAt(const Poco::Int64);
    void StopTracking();

    static bool isNotFound(const error &err);

    const std::string GroupHash() const;

    // User-triggered changes to timer:
    void SetDurationUserInput(const std::string &);
    void SetStopUserInput(const std::string &);
    void SetStartUserInput(const std::string &, const bool);

    // Override BaseModel
    std::string ModelName() const override;
    std::string ModelURL() const override;
    std::string String() const override;
    virtual bool ResolveError(const error &err) override;
    void LoadFromJSON(Json::Value value) override;
    Json::Value SaveToJSON(int apiVersion = 8) const override;
    Json::Value SyncMetadata() const override;
    Json::Value SyncPayload() const override;

    // Implement TimedEvent
    virtual const Poco::Int64 &Start() const {
        return StartTime();
    }
    virtual const Poco::Int64 &Duration() const {
        return DurationInSeconds();
    }

 private:

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
