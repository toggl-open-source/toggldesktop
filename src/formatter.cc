// Copyright 2014 Toggl Desktop developers.

#include "./formatter.h"

#include <sstream>

#include "Poco/Types.h"
#include "Poco/String.h"
#include "Poco/Timestamp.h"
#include "Poco/DateTimeFormatter.h"
#include "Poco/DateTimeFormat.h"
#include "Poco/NumberParser.h"
#include "Poco/StringTokenizer.h"
#include "Poco/DateTimeParser.h"

namespace kopsik {

std::string Formatter::JoinTaskName(Task *t, Project *p, Client *c) {
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

std::string Formatter::FormatDateWithTime(const std::time_t date) {
    poco_assert(date);
    Poco::Timestamp ts = Poco::Timestamp::fromEpochTime(date);
    return Poco::DateTimeFormatter::format(ts, "%w %d. %b %H:%M");
}

std::string Formatter::FormatDateHeader(const std::time_t date) {
    poco_assert(date);

    Poco::Timestamp ts = Poco::Timestamp::fromEpochTime(date);
    Poco::LocalDateTime datetime(ts);

    Poco::LocalDateTime today;
    if (today.year() == datetime.year() &&
            today.month() == datetime.month() &&
            today.day() == datetime.day()) {
        return "Today";
    }

    Poco::LocalDateTime yesterday = today -
        Poco::Timespan(24 * Poco::Timespan::HOURS);
    if (yesterday.year() == datetime.year() &&
            yesterday.month() == datetime.month() &&
            yesterday.day() == datetime.day()) {
        return "Yesterday";
    }

    return Poco::DateTimeFormatter::format(ts, "%w %d. %b");
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
    Poco::Timespan span(0, hours, minutes, seconds, 0);

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
    Poco::Timespan span(0, hours, minutes, 0, 0);

    *parsed_seconds = span.totalSeconds();

    return true;
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
    Poco::Timespan span(0, 0, minutes, seconds, 0);

    *parsed_seconds = span.totalSeconds();

    return true;
}

int Formatter::ParseDurationString(const std::string value) {
    if (value.empty()) {
      return 0;
    }

    size_t separator_pos = value.find(":");
    if (separator_pos != std::string::npos) {
        int parsed_seconds = 0;

        // Parse duration in seconds HH:MM:SS
        if (parseDurationStringHHMMSS(value, &parsed_seconds)) {
            return parsed_seconds;
        }

        // Parse duration in seconds HH:MM
        if (parseDurationStringHHMM(value, &parsed_seconds)) {
            return parsed_seconds;
        }

        // 05:22 min
        size_t pos = value.find(" min");
        if (pos != std::string::npos) {
            std::string numbers = value.substr(0, pos);
            if (parseDurationStringMMSS(numbers, &parsed_seconds)) {
                return parsed_seconds;
            }
        }
    }

    // 1,5 hours
    size_t pos = value.find(" hour");
    if (pos != std::string::npos) {
        std::string numbers = value.substr(0, pos);
        numbers = Poco::replace(numbers, ",", ".");
        double hours = 0;
        if (Poco::NumberParser::tryParseFloat(numbers, hours)) {
            return hours * 60 * 60;
        }
    }

    // 1,5 minutes
    pos = value.find(" min");
    if (pos != std::string::npos) {
        std::string numbers = value.substr(0, pos);
        numbers = Poco::replace(numbers, ",", ".");
        double minutes = 0;
        if (Poco::NumberParser::tryParseFloat(numbers, minutes)) {
            return minutes * 60;
        }
    }

    // 1,5 seconds
    pos = value.find(" sec");
    if (pos != std::string::npos) {
        std::string numbers = value.substr(0, pos);
        numbers = Poco::replace(numbers, ",", ".");
        double seconds = 0;
        if (Poco::NumberParser::tryParseFloat(numbers, seconds)) {
            return seconds;
        }
    }

    // 1.5h
    pos = value.find("h");
    if (pos != std::string::npos) {
        std::string numbers = value.substr(0, pos);
        numbers = Poco::replace(numbers, ",", ".");
        double hours = 0;
        if (Poco::NumberParser::tryParseFloat(numbers, hours)) {
            return hours * 60 * 60;
        }
    }

    // 15m
    pos = value.find("m");
    if (pos != std::string::npos) {
        std::string numbers = value.substr(0, pos);
        numbers = Poco::replace(numbers, ",", ".");
        double minutes = 0;
        if (Poco::NumberParser::tryParseFloat(numbers, minutes)) {
            return minutes * 60;
        }
    }

    // 25s
    pos = value.find("s");
    if (pos != std::string::npos) {
        std::string numbers = value.substr(0, pos);
        numbers = Poco::replace(numbers, ",", ".");
        double seconds = 0;
        if (Poco::NumberParser::tryParseFloat(numbers, seconds)) {
            return seconds;
        }
    }

    // 1,5
    // (we're gonna parse it as minutes)
    double minutes = 0;
    if (Poco::NumberParser::tryParseFloat(value, minutes)) {
        return minutes * 60;
    }

    return 0;
}

std::string Formatter::FormatDurationInSeconds(const Poco::Int64 value,
        const std::string format) {
    Poco::Int64 duration = value;
    if (duration < 0) {
        duration += time(0);
    }
    Poco::Timespan span(duration * Poco::Timespan::SECONDS);
    return Poco::DateTimeFormatter::format(span, format);
}

std::string Formatter::FormatDurationInSecondsHHMMSS(const Poco::Int64 value) {
    return FormatDurationInSeconds(value, "%H:%M:%S");
}

std::string Formatter::FormatDurationInSecondsHHMM(const Poco::Int64 value,
        const int type) {
    if (type == 1) {
        return FormatDurationInSeconds(value, "%H %M");
    } else if (type == 2) {
        return FormatDurationInSeconds(value, "%Hh:%Mm");
    }
    return FormatDurationInSeconds(value, "%H:%M");
}

std::time_t Formatter::Parse8601(const std::string iso_8601_formatted_date) {
    if ("null" == iso_8601_formatted_date) {
        return 0;
    }
    int tzd;
    Poco::DateTime dt;
    Poco::DateTimeParser::parse(Poco::DateTimeFormat::ISO8601_FORMAT,
        iso_8601_formatted_date, dt, tzd);
    dt.makeUTC(tzd);
    Poco::Timestamp ts = dt.timestamp();
    return ts.epochTime();
}

std::string Formatter::Format8601(const std::time_t date) {
    if (!date) {
        return "null";
    }
    Poco::Timestamp ts = Poco::Timestamp::fromEpochTime(date);
    return Poco::DateTimeFormatter::format(ts,
        Poco::DateTimeFormat::ISO8601_FORMAT);
}

// http://stackoverflow.com/questions/7724448/simple-json-string-escape-for-c
std::string Formatter::EscapeJSONString(const std::string input) {
    std::ostringstream ss;
    for (std::string::const_iterator iter = input.begin();
            iter != input.end();
            iter++) {
        switch (*iter) {
            case '\\': ss << "\\\\"; break;
            case '"': ss << "\\\""; break;
            case '/': ss << "\\/"; break;
            case '\b': ss << "\\b"; break;
            case '\f': ss << "\\f"; break;
            case '\n': ss << "\\n"; break;
            case '\r': ss << "\\r"; break;
            case '\t': ss << "\\t"; break;
            default: ss << *iter; break;
        }
    }
    return ss.str();
}

}   // namespace kopsik
