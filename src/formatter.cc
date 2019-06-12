// Copyright 2014 Toggl Desktop developers.

#include "../src/formatter.h"

#include <time.h>
#include <sstream>
#include <cctype>
#include <set>

#include "./client.h"
#include "./gui.h"
#include "./project.h"
#include "./task.h"
#include "./time_entry.h"
#include "./workspace.h"

#include "Poco/DateTimeFormat.h"
#include "Poco/DateTimeFormatter.h"
#include "Poco/DateTimeParser.h"
#include "Poco/LocalDateTime.h"
#include "Poco/Logger.h"
#include "Poco/NumberFormatter.h"
#include "Poco/NumberParser.h"
#include "Poco/String.h"
#include "Poco/StringTokenizer.h"
#include "Poco/Types.h"
#include "Poco/UTF8String.h"

namespace toggl {

const std::string &Format::Classic = std::string("classic");
const std::string &Format::Improved = std::string("improved");
const std::string &Format::Decimal = std::string("decimal");

std::string Formatter::TimeOfDayFormat = std::string("");
std::string Formatter::DurationFormat = Format::Improved;

std::string Formatter::togglTimeOfDayToPocoFormat(
    const std::string &toggl_format) {
    if ("h:mm A" == toggl_format) {
        return "%h:%M %A";
    }
    return "%H:%M";
}

std::string Formatter::JoinTaskName(
    Task * const t,
    Project * const p) {
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
        if (p->CID()) {
            if (!empty) {
                ss << ". ";
            }
            ss << p->ClientName();
        }
    }

    return ss.str();
}

std::string Formatter::FormatTimeForTimeEntryEditor(
    const std::time_t date) {
    if (!date) {
        return "";
    }
    Poco::Timestamp ts = Poco::Timestamp::fromEpochTime(date);
    Poco::LocalDateTime local(ts);
    std::string fmt = togglTimeOfDayToPocoFormat(TimeOfDayFormat);
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

    return Poco::DateTimeFormatter::format(datetime, "%w, %e %b");
}

bool Formatter::parseTimeInputAMPM(const std::string &numbers,
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
    const std::string &value,
    const std::string &am_symbol,
    const std::string &pm_symbol,
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

// Validate time output a bit. Else it will blow up
// in Poco DateTime constructor
bool timeIsWithinLimits(int *hours, int *minutes) {
    if ((*hours < 0) || (*hours > 23)) {
        return false;
    }
    if ((*minutes < 0) || (*minutes > 59)) {
        return false;
    }
    return true;
}

bool Formatter::ParseTimeInput(const std::string &input,
                               int *hours,
                               int *minutes) {
    std::string value = Poco::replace(Poco::UTF8::toUpper(input), " ", "");

    if (parseTimeInputAMPM(value, "DOP", "ODP", hours, minutes)) {
        return timeIsWithinLimits(hours, minutes);
    }

    if (parseTimeInputAMPM(value, "A", "P", hours, minutes)) {
        return timeIsWithinLimits(hours, minutes);
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

    return timeIsWithinLimits(hours, minutes);
}

bool Formatter::parseDurationStringHHMMSS(const std::string &value,
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

bool Formatter::parseDurationStringHHMM(const std::string &value,
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
    const std::string &delimiter,
    double *value,
    std::string *whatsleft) {

    size_t pos = whatsleft->find(delimiter);
    if (std::string::npos == pos) {
        return;
    }

    std::string token = whatsleft->substr(0, pos);

    if (token.length()) {
        double d(0);
        if (Poco::NumberParser::tryParseFloat(token, d)) {
            *value = d;
        }
        whatsleft->erase(0, whatsleft->find(delimiter) + delimiter.length());
    }
}

int Formatter::parseDurationStringHoursMinutesSeconds(
    std::string *whatsleft) {

    double hours = 0;
    take("hours", &hours, whatsleft);
    take("hour", &hours, whatsleft);
    take("hr", &hours, whatsleft);
    take("h", &hours, whatsleft);

    double minutes = 0;
    take("minutes", &minutes, whatsleft);
    take("minute", &minutes, whatsleft);
    take("min", &minutes, whatsleft);
    take("m", &minutes, whatsleft);

    double seconds = 0;
    take("seconds", &seconds, whatsleft);
    take("second", &seconds, whatsleft);
    take("sec", &seconds, whatsleft);
    take("s", &seconds, whatsleft);

    long period = static_cast<long>(hours * 3600.0) +
                  static_cast<long>(minutes * 60.0) +
                  static_cast<long>(seconds);
    return Poco::Timespan(period, 0).totalSeconds();
}

bool Formatter::parseDurationStringMMSS(const std::string &value,
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

int Formatter::ParseDurationString(const std::string &value) {
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

    int seconds = parseDurationStringHoursMinutesSeconds(&input);

    // 15
    if (input.find(".") == std::string::npos) {
        Poco::Int64 minutes = 0;
        if (Poco::NumberParser::tryParse64(input, minutes)) {
            if ((minutes * 60) > kMaxDurationSeconds) {
                return kMaxDurationSeconds;
            }
            return static_cast<int>(seconds + (minutes * 60));
        }
    }

    // 1,5
    double hours = 0;
    if (Poco::NumberParser::tryParseFloat(input, hours)) {
        return static_cast<int>(hours * 60 * 60);
    }

    return seconds;
}

Poco::Int64 Formatter::AbsDuration(const Poco::Int64 value) {
    Poco::Int64 duration = value;

    // Duration is negative when time is tracking
    if (duration < 0) {
        duration += time(nullptr);
    }
    // If after calculation time is still negative,
    // either computer clock is wrong or user
    // has set start time to the future. Render positive
    // duration only:
    if (duration < 0) {
        duration *= -1;
    }

    return duration;
}

std::string Formatter::FormatDurationForDateHeader(
    const Poco::Int64 value) {
    Poco::Int64 duration = AbsDuration(value);

    std::stringstream ss;

    Poco::Int64 hours = duration / 3600;
    ss << hours << " h ";

    Poco::Int64 minutes = (duration - (hours * 3600)) / 60;
    if (minutes < 10) {
        ss << "0";
    }
    ss << minutes << " min";

    return ss.str();
}

std::string Formatter::FormatDuration(
    const Poco::Int64 value,
    const std::string &format_name,
    const bool with_seconds) {
    Poco::Int64 duration = AbsDuration(value);

    if (Format::Decimal == format_name) {
        double hours = duration / 3600.0;
        // Following rounding up is needed
        // to be compatible with Toggl web site.
        double a = hours * 100.0;
        int b = static_cast<int>(a);
        double d = a - std::floor(a);
        if (d > 0.5) {
            b++;
        }
        double c = b / 100.0;
        std::stringstream ss;
        ss << Poco::NumberFormatter::format(c, 2) << " h";
        return ss.str();
    }

    if (Format::Classic == format_name) {
        if (duration < 60) {
            std::stringstream ss;
            ss << duration << " sec";
            return ss.str();
        }
        if (duration < 3600) {
            Poco::Timespan span(duration * Poco::Timespan::SECONDS);
            return Poco::DateTimeFormatter::format(span, "%M:%S min");
        }
        std::stringstream ss;
        Poco::Int64 hours = duration / 3600;
        // Poco DateTimeFormatter will not format hours above 24h.
        // So format hours by hand:
        if (hours < 10) {
            ss << "0";
        }
        ss << hours;
        ss << ":";
        Poco::Timespan span(duration * Poco::Timespan::SECONDS);
        ss << Poco::DateTimeFormatter::format(span, "%M:%S");
        return ss.str();
    }

    // Default, 'improved' format
    // Poco DateTimeFormatter will not format hours above 24h.
    // So format hours by hand:
    std::stringstream ss;
    Poco::Int64 hours = duration / 3600;
    ss << hours;
    ss << ":";
    Poco::Timespan span(duration * Poco::Timespan::SECONDS);
    if (with_seconds) {
        ss << Poco::DateTimeFormatter::format(span, "%M:%S");
    } else {
        ss << Poco::DateTimeFormatter::format(span, "%M");
    }
    return ss.str();
}

std::time_t Formatter::Parse8601(const std::string &iso_8601_formatted_date) {
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
    return Format8601(ts);
}

std::string Formatter::Format8601(const Poco::Timestamp ts) {
    return Poco::DateTimeFormatter::format(
        ts,
        Poco::DateTimeFormat::ISO8601_FORMAT);
}

std::string Formatter::EscapeJSONString(const std::string &input) {
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

error Formatter::CollectErrors(std::vector<error> * const errors) {
    std::stringstream ss;
    ss << "Errors encountered while syncing data: ";
    std::set<error> unique;
    for (std::vector<error>::const_iterator it = errors->begin();
            it != errors->end();
            it++) {
        error err = *it;
        if (!err.empty() && err[err.size() - 1] == '\n') {
            err[err.size() - 1] = '.';
        }
        // skip error if not unique
        if (unique.end() != unique.find(err)) {
            continue;
        }
        if (it != errors->begin()) {
            ss << " ";
        }
        ss << err;
        unique.insert(err);
    }
    return error(ss.str());
}

bool CompareClientByName(Client *a, Client *b) {
    return (Poco::UTF8::icompare(a->Name(), b->Name()) < 0);
}

bool CompareByStart(TimedEvent *a, TimedEvent *b) {
    return a->Start() < b->Start();
}

bool CompareWorkspaceByName(Workspace *a, Workspace *b) {
    return (Poco::UTF8::icompare(a->Name(), b->Name()) < 0);
}

bool CompareAutotrackerTitles(const std::string &a, const std::string &b) {
    return (Poco::UTF8::icompare(a, b) < 0);
}

bool CompareAutocompleteItems(
    view::Autocomplete a,
    view::Autocomplete b) {

    // Time entries first
    if (a.IsTimeEntry() && !b.IsTimeEntry()) {
        return true;
    }
    if (b.IsTimeEntry() && !(a.IsTimeEntry())) {
        return false;
    }

    // Then tasks
    if (a.IsTask() && !b.IsTask()) {
        return true;
    }
    if (b.IsTask() && !a.IsTask()) {
        return false;
    }

    // Then projects
    if (a.IsProject() && !b.IsProject()) {
        return true;
    }
    if (b.IsProject() && !a.IsProject()) {
        return false;
    }

    return (Poco::UTF8::icompare(a.Text, b.Text) < 0);
}

bool CompareStructuredAutocompleteItems(
    view::Autocomplete a,
    view::Autocomplete b) {

    if (a.WorkspaceName == b.WorkspaceName) {
        if (a.IsWorkspace() && !b.IsWorkspace()) {
            return true;
        }
        if (!a.IsWorkspace() && b.IsWorkspace()) {
            return false;
        }
        return (Poco::UTF8::icompare(a.Text, b.Text) < 0);
    }

    return (Poco::UTF8::icompare(a.WorkspaceName, b.WorkspaceName) < 0);
}

}   // namespace toggl
