// Copyright 2014 Toggl Desktop developers.

#ifndef SRC_FORMATTER_H_
#define SRC_FORMATTER_H_

#include "types.h"
#include "model/base_model.h"

#include <string>
#include <ctime>
#include <vector>

#include <Poco/Timestamp.h>

namespace toggl {

namespace view {
class Autocomplete;
}  // namespace view

class Format {
 public:
    static const std::string Classic;
    static const std::string Improved;
    static const std::string Decimal;
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

    static std::string JoinTaskName(locked<const Task> &t, locked<const Project> &p);

    static std::string FormatDuration(int64_t value, const std::string &format_name, bool with_seconds = true);

    static std::string FormatDurationForDateHeader(int64_t value);

    static std::string Format8601(std::time_t date);

    static std::string Format8601(Poco::Timestamp ts);

    static std::string FormatDateHeader(std::time_t date);

    static std::string FormatTimeForTimeEntryEditor(std::time_t date);

    static error CollectErrors(const std::vector<error> &errors);

    // Time

    static Poco::Int64 AbsDuration(int64_t value);

    // Parse

    static std::time_t Parse8601(const std::string &iso_8601_formatted_date);

    static int ParseDurationString(const std::string &value);

    static bool ParseTimeInput(const std::string &value, int *hours, int *minutes);

    // Escape

    static std::string EscapeJSONString(const std::string &input);

 private:
    static std::string togglTimeOfDayToPocoFormat(const std::string &toggl_format);

    static void take(const std::string &delimiter, double *value, std::string *whatsleft);

    static bool parseTimeInputAMPM(const std::string &numbers, int *hours, int *minutes, bool has_pm);

    static bool parseTimeInputAMPM(const std::string &value, const std::string &am_symbol, const std::string &pm_symbol, int *hours, int *minutes);

    static bool parseDurationStringHHMMSS(const std::string &value, int *parsed_seconds);

    static bool parseDurationStringHHMM(const std::string &value, int *parsed_seconds);

    static bool parseDurationStringMMSS(const std::string &value, int *parsed_seconds);

    static int parseDurationStringHoursMinutesSeconds(std::string *whatsleft);
};

bool CompareClientByName(Client *a, Client *b);
bool CompareByStart(TimedEvent *a, TimedEvent *b);
bool CompareAutocompleteItems(view::Autocomplete a, view::Autocomplete b);
bool CompareStructuredAutocompleteItems(view::Autocomplete a, view::Autocomplete b);
bool CompareWorkspaceByName(Workspace *a, Workspace *b);
bool CompareAutotrackerTitles(const std::string &a, const std::string &b);

bool timeIsWithinLimits(int *hours, int *minutes);

}  // namespace toggl

#endif  // SRC_FORMATTER_H_
