// Copyright 2014 Toggl Desktop developers.

#ifndef SRC_FORMATTER_H_
#define SRC_FORMATTER_H_

#include <string>
#include <ctime>
#include <vector>

#include "./task.h"
#include "./project.h"
#include "./client.h"
#include "./time_entry.h"
#include "./autocomplete_item.h"

namespace toggl {

class Format {
 public:
    static std::string Classic;
    static std::string Improved;
    static std::string Decimal;
};

class Formatter {
 public:
    // Format

    static std::string TimeOfDayFormat;
    static std::string DurationFormat;

    static std::string JoinTaskName(
        Task * const,
        Project * const,
        Client * const);

    static std::string JoinTaskNameReverse(
        Task * const,
        Project * const,
        Client * const);

    static std::string FormatDuration(
        const Poco::Int64 value,
        const std::string format_name,
        const bool with_seconds = true);

    static std::string FormatDurationForDateHeader(
        const Poco::Int64 value);

    static std::string Format8601(
        const std::time_t date);

    static std::string FormatDateHeader(
        const std::time_t date);

    static std::string FormatTimeForTimeEntryEditor(
        const std::time_t date);

    static error CollectErrors(
        std::vector<error> * const errors);

    // Parse

    static std::time_t Parse8601(
        const std::string iso_8601_formatted_date);

    static int ParseDurationString(
        const std::string value);

    static bool ParseTimeInput(
        const std::string value,
        int *hours,
        int *minutes);

    // Escape

    static std::string EscapeJSONString(
        const std::string input);

 private:
    static std::string togglTimeOfDayToPocoFormat(
        const std::string toggl_format);

    static void take(
        const std::string delimiter,
        double &value,
        std::string &whatsleft);

    static bool parseTimeInputAMPM(
        const std::string numbers,
        int *hours,
        int *minutes,
        const bool has_pm);

    static bool parseTimeInputAMPM(
        const std::string value,
        const std::string am_symbol,
        const std::string pm_symbol,
        int *hours,
        int *minutes);

    static bool parseDurationStringHHMMSS(
        const std::string value,
        int *parsed_seconds);

    static bool parseDurationStringHHMM(
        const std::string value,
        int *parsed_seconds);

    static bool parseDurationStringMMSS(
        const std::string value,
        int *parsed_seconds);

    static int parseDurationStringHoursMinutesSeconds(
        std::string &whatsleft);
};

bool CompareClientByName(Client *a, Client *b);
bool CompareTimeEntriesByStart(TimeEntry *a, TimeEntry *b);
bool CompareAutocompleteItems(AutocompleteItem a, AutocompleteItem b);
bool CompareStructuredAutocompleteItems(
    AutocompleteItem a, AutocompleteItem b);

}  // namespace toggl

#endif  // SRC_FORMATTER_H_
