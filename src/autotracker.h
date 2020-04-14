// Copyright 2015 Toggl Desktop developers.

#ifndef SRC_AUTOTRACKER_H_
#define SRC_AUTOTRACKER_H_

#include <string>
#include <sstream>
#include <vector>
#include <bitset>

#include <Poco/Types.h>

#include "base_model.h"
#include "timeline_event.h"

namespace toggl {

class TOGGL_INTERNAL_EXPORT AutotrackerRule : public BaseModel {
 public:
    AutotrackerRule()
        : BaseModel()
    {}

    virtual ~AutotrackerRule() {}

    bool Matches(const TimelineEvent &event) const;

    const std::vector<std::string> &Terms() const;
    void SetTerms(const std::string &value);
    const std::string TermsString() const;

    void SetDaysOfWeek(const Poco::UInt8 daysOfWeek);
    Poco::UInt8 DaysOfWeek() const;

    const std::string &StartTime() const;
    void SetStartTime(const std::string &value);

    const std::string &EndTime() const;
    void SetEndTime(const std::string &value);

    const Poco::UInt64 &PID() const;
    void SetPID(const Poco::UInt64 value);

    const Poco::UInt64 &TID() const;
    void SetTID(const Poco::UInt64 value);

    static Poco::UInt8 DaysOfWeekIntoUInt8(
        const bool sunday,
        const bool monday,
        const bool tuesday,
        const bool wednesday,
        const bool thursday,
        const bool friday,
        const bool saturday);

    // Override BaseModel
    std::string String() const override;
    std::string ModelName() const override;
    std::string ModelURL() const override;

 private:
    std::vector<std::string> terms_;
    Poco::UInt8 days_of_week_ { 0 };
    std::string start_time_ { "" };
    std::string end_time_ { "" };
    Poco::UInt64 pid_ { 0 };
    Poco::UInt64 tid_ { 0 };
};

};  // namespace toggl

#endif  // SRC_AUTOTRACKER_H_
