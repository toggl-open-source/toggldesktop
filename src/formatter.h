// Copyright 2014 Toggl Desktop developers.

#ifndef SRC_FORMATTER_H_
#define SRC_FORMATTER_H_

#include <string>
#include <ctime>
#include <vector>

#include "Poco/Timestamp.h"

#include "./types.h"

namespace toggl {

namespace view {
class Autocomplete;
}  // namespace view

class Format {
 public:
    static const std::string &Classic;
    static const std::string &Improved;
    static const std::string &Decimal;
};

class Client;
class Project;
class Task;
class Workspace;

class TimedEvent {
 public:
    TimedEvent() {}
    virtual ~TimedEvent() {}

    virtual const Poco::Int64 &Start() const = 0;
    virtual Poco::Int64 Duration() const = 0;
};

class Formatter {
 public:
    // Format

    static std::string TimeOfDayFormat;
    static std::string DurationFormat;

    static std::string JoinTaskName(
        Task * const,
        Project * const);

    static std::string FormatDuration(
        const Poco::Int64 value,
        const std::string &format_name,
        const bool with_seconds = true);

    static std::string FormatDurationForDateHeader(
        const Poco::Int64 value);

    static std::string Format8601(
        const std::time_t date);

    static std::string Format8601(
        const Poco::Timestamp ts);

    static std::string FormatDateHeader(
        const std::time_t date);

    static std::string FormatTimeForTimeEntryEditor(
        const std::time_t date);

    static error CollectErrors(
        std::vector<error> * const errors);

    // Time

    static Poco::Int64 AbsDuration(const Poco::Int64 value);

    // Parse

    static std::time_t Parse8601(
        const std::string &iso_8601_formatted_date);

    static int ParseDurationString(
        const std::string &value);

    static bool ParseTimeInput(
        const std::string &value,
        int *hours,
        int *minutes);

    // Escape

    static std::string EscapeJSONString(
        const std::string &input);

 private:
    static std::string togglTimeOfDayToPocoFormat(
        const std::string &toggl_format);

    static void take(
        const std::string &delimiter,
        double *value,
        std::string *whatsleft);

    static bool parseTimeInputAMPM(
        const std::string &numbers,
        int *hours,
        int *minutes,
        const bool has_pm);

    static bool parseTimeInputAMPM(
        const std::string &value,
        const std::string &am_symbol,
        const std::string &pm_symbol,
        int *hours,
        int *minutes);

    static bool parseDurationStringHHMMSS(
        const std::string &value,
        int *parsed_seconds);

    static bool parseDurationStringHHMM(
        const std::string &value,
        int *parsed_seconds);

    static bool parseDurationStringMMSS(
        const std::string &value,
        int *parsed_seconds);

    static int parseDurationStringHoursMinutesSeconds(
        std::string *whatsleft);
};

bool CompareClientByName(
    Client *a,
    Client *b);
bool CompareByStart(
    TimedEvent *a,
    TimedEvent *b);
bool CompareAutocompleteItems(
    view::Autocomplete a,
    view::Autocomplete b);
bool CompareStructuredAutocompleteItems(
    view::Autocomplete a,
    view::Autocomplete b);
bool CompareWorkspaceByName(
    Workspace *a,
    Workspace *b);
bool CompareAutotrackerTitles(
    const std::string &a,
    const std::string &b);

bool timeIsWithinLimits(
    int *hours,
    int *minutes);

}  // namespace toggl

#endif  // SRC_FORMATTER_H_
