// Copyright 2014 Toggl Desktop developers.

#include "./formatter.h"

#include <time.h>
#include <sstream>
#include <cctype>

#include "./time_entry.h"

#include "Poco/Types.h"
#include "Poco/String.h"
#include "Poco/Timestamp.h"
#include "Poco/DateTimeFormatter.h"
#include "Poco/DateTimeFormat.h"
#include "Poco/NumberParser.h"
#include "Poco/StringTokenizer.h"
#include "Poco/DateTimeParser.h"
#include "Poco/LocalDateTime.h"

namespace toggl {

std::string Formatter::togglTimeOfDayToPocoFormat(
    const std::string toggl_format) {
    if ("h:mm A" == toggl_format) {
        return "%h:%M %A";
    }
    return "%H:%M";
}

std::string Formatter::JoinTaskName(
    Task * const t,
    Project * const p,
    Client * const c) {
    std::stringstream ss;
    bool empty = true;
    if (t) {
        ss << t->Name();
        empty = false;
    }
    if (p) {
        if (!empty) {
            ss << ". ";
        }
        ss << p->Name();
        empty = false;
    }
    if (c) {
        if (!empty) {
            ss << ". ";
        }
        ss << c->Name();
    }
    return ss.str();
}

std::string Formatter::JoinTaskNameReverse(
    Task * const t,
    Project * const p,
    Client * const c) {
    std::stringstream ss;
    bool empty = true;
    if (c) {
        ss << c->Name();
        empty = false;
    }
    if (p) {
        if (!empty) {
            ss << ". ";
        }
        ss << p->Name();
        empty = false;
    }
    if (t) {
        if (!empty) {
            ss << ". ";
        }
        ss << t->Name();
    }
    return ss.str();
}

std::string Formatter::FormatTimeForTimeEntryEditor(
    const std::time_t date,
    const std::string timeofday_format) {
    if (!date) {
        return "";
    }
    Poco::Timestamp ts = Poco::Timestamp::fromEpochTime(date);
    Poco::LocalDateTime local(ts);
    std::string fmt = togglTimeOfDayToPocoFormat(timeofday_format);
    return Poco::DateTimeFormatter::format(local, fmt);
}

std::string Formatter::FormatDateHeader(const std::time_t date) {
    if (!date) {
        return "";
    }

    Poco::Timestamp ts = Poco::Timestamp::fromEpochTime(date);
    Poco::LocalDateTime datetime(ts);

    Poco::LocalDateTime today;
    if (today.year() == datetime.year() &&
            today.month() == datetime.month() &&
            today.day() == datetime.day()) {
        return "Today";
    }

    Poco::LocalDateTime yesterday =
        today - Poco::Timespan(24 * Poco::Timespan::HOURS);
    if (yesterday.year() == datetime.year() &&
            yesterday.month() == datetime.month() &&
            yesterday.day() == datetime.day()) {
        return "Yesterday";
    }

    return Poco::DateTimeFormatter::format(datetime, "%w %d. %b");
}

bool Formatter::parseTimeInputAMPM(const std::string numbers,
                                   int *hours,
                                   int *minutes,
                                   const bool has_pm) {
    *hours = 0;
    *minutes = 0;

    // Handle formats: HHa, HHmma, HH:mm a
    if (numbers.length() > 2) {
        Poco::StringTokenizer tokenizer(numbers, ":");
        if (2 == tokenizer.count()) {
            if (!Poco::NumberParser::tryParse(tokenizer[0], *hours)) {
                return false;
            }
            if (!Poco::NumberParser::tryParse(tokenizer[1], *minutes)) {
                return false;
            }
        } else if (!Poco::NumberParser::tryParse(
            numbers.substr(0, numbers.length()-2), *hours)
                   || !Poco::NumberParser::tryParse(
                       numbers.substr((numbers.length()-2), 2), *minutes)
                  ) {
            return false;
        }

    } else if (!Poco::NumberParser::tryParse(numbers, *hours)) {
        return false;
    }

    if (has_pm && *hours < 12) {
        *hours = *hours + 12;
    } else if (*hours == 12 && !has_pm) {
        *hours = 0;
    }

    return true;
}

bool Formatter::parseTimeInputAMPM(
    const std::string value,
    const std::string am_symbol,
    const std::string pm_symbol,
    int *hours,
    int *minutes) {

    std::size_t pos = value.find(am_symbol);
    bool am_pm = pos != std::string::npos;
    if (!am_pm) {
        pos = value.find(pm_symbol);
        am_pm = pos != std::string::npos;
    }
    if (am_pm) {
        std::string numbers = value.substr(0, pos);
        bool pm = value.find(pm_symbol) != std::string::npos;
        return parseTimeInputAMPM(numbers, hours, minutes, pm);
    }
    return false;
}

bool Formatter::ParseTimeInput(const std::string input,
                               int *hours,
                               int *minutes) {
    std::string value = Poco::replace(Poco::toUpper(input), " ", "");

    if (parseTimeInputAMPM(value, "DOP", "ODP", hours, minutes)) {
        return true;
    }

    if (parseTimeInputAMPM(value, "A", "P", hours, minutes)) {
        return true;
    }

    // Handle formats: HH:mm, H:mm etc
    if (value.find(":") != std::string::npos) {
        Poco::StringTokenizer tokenizer(value, ":");
        if (2 != tokenizer.count()) {
            return false;
        }
        if (!Poco::NumberParser::tryParse(tokenizer[0], *hours)) {
            return false;
        }
        if (!Poco::NumberParser::tryParse(tokenizer[1], *minutes)) {
            return false;
        }
        // Handle formats: HHmm, Hmm
    } else if (value.length() > 2) {
        std::string hours_part = value.substr(0, value.length()-2);
        if (!Poco::NumberParser::tryParse(hours_part, *hours)) {
            return false;
        }
        std::string minutes_part = value.substr((value.length()-2), 2);
        if (!Poco::NumberParser::tryParse(minutes_part, *minutes)) {
            return false;
        }
        // Handle formats: HH, H
    } else if (!Poco::NumberParser::tryParse(value, *hours)) {
        return false;
    }
    return true;
}

time_t Formatter::ParseLastDate(const std::time_t last,
                                const std::time_t current) {
    Poco::Timestamp last_ts = Poco::Timestamp::fromEpochTime(last);
    Poco::DateTime last_date(last_ts);

    Poco::Timestamp current_ts = Poco::Timestamp::fromEpochTime(current);
    Poco::DateTime current_date(current_ts);

    current_date.assign(last_date.year(), last_date.month(), last_date.day());

    return current_date.timestamp().epochTime();
}

bool Formatter::parseDurationStringHHMMSS(const std::string value,
        int *parsed_seconds) {
    *parsed_seconds = 0;

    Poco::StringTokenizer tokenizer(value, ":");
    if (3 != tokenizer.count()) {
        return false;
    }
    int hours = 0;
    if (!Poco::NumberParser::tryParse(tokenizer[0], hours)) {
        return false;
    }
    int minutes = 0;
    if (!Poco::NumberParser::tryParse(tokenizer[1], minutes)) {
        return false;
    }
    int seconds = 0;
    if (!Poco::NumberParser::tryParse(tokenizer[2], seconds)) {
        return false;
    }

    Poco::Timespan span(hours*3600 + minutes*60 + seconds, 0);

    *parsed_seconds = span.totalSeconds();

    return true;
}

bool Formatter::parseDurationStringHHMM(const std::string value,
                                        int *parsed_seconds) {
    *parsed_seconds = 0;

    Poco::StringTokenizer tokenizer(value, ":");
    if (2 != tokenizer.count()) {
        return false;
    }
    int hours = 0;
    if (!Poco::NumberParser::tryParse(tokenizer[0], hours)) {
        return false;
    }
    int minutes = 0;
    if (!Poco::NumberParser::tryParse(tokenizer[1], minutes)) {
        return false;
    }
    Poco::Timespan span(hours*3600 + minutes*60, 0);

    *parsed_seconds = span.totalSeconds();

    return true;
}

void Formatter::take(
    const std::string delimiter,
    double &value,
    std::string &whatsleft) {

    size_t pos = whatsleft.find(delimiter);
    if (std::string::npos == pos) {
        return;
    }

    std::string token = whatsleft.substr(0, pos);

    if (token.length()) {
        double d(0);
        if (Poco::NumberParser::tryParseFloat(token, d)) {
            value = d;
        }
        whatsleft.erase(0, whatsleft.find(delimiter) + delimiter.length());
    }
}

int Formatter::parseDurationStringHoursMinutesSeconds(
    std::string &whatsleft) {

    double hours = 0;
    take("hours", hours, whatsleft);
    take("hour", hours, whatsleft);
    take("hr", hours, whatsleft);
    take("h", hours, whatsleft);

    double minutes = 0;
    take("minutes", minutes, whatsleft);
    take("minute", minutes, whatsleft);
    take("min", minutes, whatsleft);
    take("m", minutes, whatsleft);

    double seconds = 0;
    take("seconds", seconds, whatsleft);
    take("second", seconds, whatsleft);
    take("sec", seconds, whatsleft);
    take("s", seconds, whatsleft);

    return Poco::Timespan(hours*3600 + minutes*60 + seconds, 0).totalSeconds();
}

bool Formatter::parseDurationStringMMSS(const std::string value,
                                        int *parsed_seconds) {
    *parsed_seconds = 0;

    Poco::StringTokenizer tokenizer(value, ":");
    if (2 != tokenizer.count()) {
        return false;
    }
    int minutes = 0;
    if (!Poco::NumberParser::tryParse(tokenizer[0], minutes)) {
        return false;
    }
    int seconds = 0;
    if (!Poco::NumberParser::tryParse(tokenizer[1], seconds)) {
        return false;
    }
    Poco::Timespan span(minutes*60 + seconds, 0);

    *parsed_seconds = span.totalSeconds();

    return true;
}

int Formatter::ParseDurationString(const std::string value) {
    std::string input = Poco::replace(value, " ", "");
    input = Poco::replace(input, ",", ".");

    if (input.find(":") != std::string::npos) {
        int parsed_seconds = 0;

        // Parse duration in seconds HH:MM:SS
        if (parseDurationStringHHMMSS(input, &parsed_seconds)) {
            return parsed_seconds;
        }

        // Parse duration in seconds HH:MM
        if (parseDurationStringHHMM(input, &parsed_seconds)) {
            return parsed_seconds;
        }

        // 05:22 min
        size_t pos = input.find("min");
        if (pos != std::string::npos) {
            std::string numbers = input.substr(0, pos);
            if (parseDurationStringMMSS(numbers, &parsed_seconds)) {
                return parsed_seconds;
            }
        }
    }

    int seconds = parseDurationStringHoursMinutesSeconds(input);

    // 15
    if (input.find(".") == std::string::npos) {
        int minutes = 0;
        if (Poco::NumberParser::tryParse(input, minutes)) {
            return seconds + (minutes * 60);
        }
    }

    // 1,5
    double hours = 0;
    if (Poco::NumberParser::tryParseFloat(input, hours)) {
        return static_cast<int>(hours * 60 * 60);
    }

    return seconds;
}

std::string Formatter::FormatDurationInSeconds(
    const Poco::Int64 value,
    const std::string format) {
    Poco::Int64 duration = TimeEntry::AbsDuration(value);
    Poco::Timespan span(duration * Poco::Timespan::SECONDS);
    // Poco DateTimeFormatter will not format hours above 24h.
    // So format hours by hand:
    std::stringstream ss;
    Poco::Int64 hours = duration / 3600;
    if (hours < 10) {
        ss << "0";
    }
    ss << hours;
    ss << ":";
    ss << Poco::DateTimeFormatter::format(span, format);
    return ss.str();
}

std::string Formatter::FormatDurationInSecondsToHM(
    const Poco::Int64 value) {
    Poco::Int64 duration = TimeEntry::AbsDuration(value);
    Poco::Timespan span(duration * Poco::Timespan::SECONDS);
    // Poco DateTimeFormatter will not format hours above 24h.
    // So format hours by hand:
    std::stringstream ss;
    Poco::Int64 hours = duration / 3600;
    Poco::Int64 minutes = (duration - (hours * 3600)) / 60;
    if (hours > 0) {
        ss << hours << " h ";
    }
    ss << minutes << " min";

    return ss.str();
}

std::string Formatter::FormatDurationInSecondsHHMMSS(const Poco::Int64 value) {
    return FormatDurationInSeconds(value, "%M:%S");
}

std::string Formatter::FormatDurationInSecondsHHMM(const Poco::Int64 value) {
    return FormatDurationInSeconds(value, "%M");
}

std::time_t Formatter::Parse8601(const std::string iso_8601_formatted_date) {
    if ("null" == iso_8601_formatted_date) {
        return 0;
    }
    if (iso_8601_formatted_date.empty()) {
        return 0;
    }
    int tzd;
    Poco::DateTime dt;
    if (!Poco::DateTimeParser::tryParse(Poco::DateTimeFormat::ISO8601_FORMAT,
                                        iso_8601_formatted_date, dt, tzd)) {
        return 0;
    }
    dt.makeUTC(tzd);
    Poco::Timestamp ts = dt.timestamp();
    time_t epoch_time = ts.epochTime();

    // Sun  9 Sep 2001 03:46:40 EET
    if (epoch_time < 1000000000) {
        Poco::Logger &logger = Poco::Logger::get("Formatter");
        std::stringstream ss;
        ss  << "Parsed timestamp is too small, will interpret as 0: "
            << epoch_time;
        logger.warning(ss.str());
        return 0;
    }

    if (epoch_time > 2000000000) {
        Poco::Logger &logger = Poco::Logger::get("Formatter");
        std::stringstream ss;
        ss  << "Parsed timestamp is too large, will interpret as 0: "
            << epoch_time;
        logger.warning(ss.str());
        return 0;
    }

    return epoch_time;
}

std::string Formatter::Format8601(const std::time_t date) {
    if (!date) {
        return "null";
    }
    Poco::Timestamp ts = Poco::Timestamp::fromEpochTime(date);
    return Poco::DateTimeFormatter::format(
        ts,
        Poco::DateTimeFormat::ISO8601_FORMAT);
}

std::string Formatter::EscapeJSONString(const std::string input) {
    std::ostringstream ss;
    for (std::string::const_iterator iter = input.begin();
            iter != input.end();
            iter++) {
        switch (*iter) {
        case '"':
            ss << "\"";
            break;
        case '\b':
        case '\f':
        case '\n':
        case '\r':
        case '\t':
            ss << " ";
            break;
        default:
            if (iscntrl(*iter)) {
                ss << " ";
            } else {
                ss << *iter;
            }
            break;
        }
    }
    return ss.str();
}

}   // namespace toggl
