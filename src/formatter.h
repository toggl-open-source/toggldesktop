// Copyright 2014 Toggl Desktop developers.

#ifndef SRC_FORMATTER_H_
#define SRC_FORMATTER_H_

#include <string>
#include <ctime>

#include "./task.h"
#include "./project.h"
#include "./client.h"

namespace toggl {

class Formatter {
 public:
    // Format

    static std::string JoinTaskName(
        Task * const,
        Project * const,
        Client * const);

    static std::string JoinTaskNameReverse(
        Task * const,
        Project * const,
        Client * const);

    static std::string FormatDurationInSeconds(
        const Poco::Int64 value,
        const std::string format);

    static std::string FormatDurationInSecondsToHM(
        const Poco::Int64 value);

    static std::string FormatDurationInSecondsHHMMSS(
        const Poco::Int64 value);

    static std::string FormatDurationInSecondsHHMM(
        const Poco::Int64 value);

    static std::string FormatDurationInSecondsPrettyHHMM(
        const Poco::Int64 value);

    static std::string Format8601(
        const std::time_t date);

    static std::string FormatDateHeader(
        const std::time_t date);

    static std::string FormatTimeForTimeEntryEditor(
        const std::time_t date,
        const std::string timeofday_format);

    static std::string togglTimeOfDayToPocoFormat(
        const std::string toggl_format);

    // Parse

    static std::time_t Parse8601(
        const std::string iso_8601_formatted_date);

    static int ParseDurationString(
        const std::string value);

    static bool ParseTimeInput(
        const std::string value,
        int *hours,
        int *minutes);

    static time_t ParseLastDate(
        const std::time_t last,
        const std::time_t current);

    // Escape

    static std::string EscapeJSONString(
        const std::string input);

 private:
    static bool parseTimeInputAMPM(
        const std::string numbers,
        int *hours,
        int *minutes,
        const bool has_pm);

    static bool parseDurationStringHHMMSS(
        const std::string value,
        int *parsed_seconds);

    static bool parseDurationStringHHMM(
        const std::string value,
        int *parsed_seconds);

    static bool parseDurationStringHHhMMm(
        const std::string value,
        int *parsed_seconds);

    static bool parseDurationStringMMSS(
        const std::string value,
        int *parsed_seconds);

    static int parseDurationFromDecimal(
        const std::string value);
};

}  // namespace toggl

#endif  // SRC_FORMATTER_H_
