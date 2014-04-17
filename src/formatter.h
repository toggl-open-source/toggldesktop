// Copyright 2014 Toggl Desktop developers.

#ifndef SRC_FORMATTER_H_
#define SRC_FORMATTER_H_

#include <string>
#include <ctime>

#include "./task.h"
#include "./project.h"
#include "./client.h"

namespace kopsik {

class Formatter {
 public:
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

    static std::string EscapeTabsAndLineBreaks(
        const std::string value);

    static bool ParseTimeInput(
        const std::string value,
        int *hours,
        int *minutes);

    static time_t ParseLastDate(
        const std::time_t last,
        const std::time_t current);

    static std::string FormatDurationInSecondsHHMMSS(
        const Poco::Int64 value);

    static std::string FormatDurationInSecondsHHMM(
        const Poco::Int64 value);

    static std::time_t Parse8601(
        const std::string iso_8601_formatted_date);

    static int ParseDurationString(
        const std::string value);

    static bool parseDurationStringHHMMSS(
        const std::string value,
        int *parsed_seconds);

    static bool parseDurationStringHHMM(
        const std::string value,
        int *parsed_seconds);

    static bool parseDurationStringMMSS(
        const std::string value,
        int *parsed_seconds);

    static int parseDurationFromDecimal(
        const std::string value);

    static std::string Format8601(
        const std::time_t date);

    static std::string FormatDateHeader(
        const std::time_t date);

    static std::string FormatDateWithTime(
        const std::time_t date);

    static std::string EscapeJSONString(
        const std::string input);
};

}  // namespace kopsik

#endif  // SRC_FORMATTER_H_
