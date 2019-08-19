// Copyright 2014 Toggl Desktop developers.

#include "user.h"

#include "client.h"
#include "const.h"
#include "formatter.h"
#include "obm_action.h"
#include "project.h"
#include "tag.h"
#include "task.h"
#include "time_entry.h"
#include "timeline_event.h"
#include "urls.h"

#include <ctime>
#include <sstream>

#include <Poco/Base64Decoder.h>
#include <Poco/Base64Encoder.h>
#include <Poco/Crypto/Cipher.h>
#include <Poco/Crypto/CipherFactory.h>
#include <Poco/Crypto/CipherKey.h>
#include <Poco/Crypto/CryptoStream.h>
#include <Poco/DigestStream.h>
#include <Poco/Logger.h>
#include <Poco/Random.h>
#include <Poco/RandomStream.h>
#include <Poco/SHA1Engine.h>
#include <Poco/Stopwatch.h>
#include <Poco/Timestamp.h>
#include <Poco/Timespan.h>
#include <Poco/UTF8String.h>

namespace toggl {

User::~User() {
}


void User::SetFullname(const std::string value) {
    if (fullname_ != value) {
        fullname_ = value;
        SetDirty();
    }
}

void User::SetTimeOfDayFormat(const std::string value) {
    Formatter::TimeOfDayFormat = value;
    if (timeofday_format_ != value) {
        timeofday_format_ = value;
        SetDirty();
    }
}

void User::SetDurationFormat(const std::string value) {
    Formatter::DurationFormat = value;
    if (duration_format_ != value) {
        duration_format_ = value;
        SetDirty();
    }
}

void User::SetOfflineData(const std::string value) {
    if (offline_data_ != value) {
        offline_data_ = value;
        SetDirty();
    }
}

void User::SetStoreStartAndStopTime(const bool value) {
    if (store_start_and_stop_time_ != value) {
        store_start_and_stop_time_ = value;
        SetDirty();
    }
}

void User::SetRecordTimeline(const bool value) {
    if (record_timeline_ != value) {
        record_timeline_ = value;
        SetDirty();
    }
}

void User::SetEmail(const std::string value) {
    if (email_ != value) {
        email_ = value;
        SetDirty();
    }
}

void User::SetAPIToken(const std::string value) {
    // API token is not saved into DB, so no
    // no dirty checking needed for it.
    api_token_ = value;
}

void User::SetSince(const Poco::Int64 value) {
    if (since_ != value) {
        since_ = value;
        SetDirty();
    }
}

void User::SetDefaultWID(const Poco::UInt64 value) {
    if (default_wid_ != value) {
        default_wid_ = value;
        SetDirty();
    }
}

void User::SetDefaultPID(const Poco::UInt64 value) {
    if (default_pid_ != value) {
        default_pid_ = value;
        SetDirty();
    }
}

void User::SetDefaultTID(const Poco::UInt64 value) {
    if (default_tid_ != value) {
        default_tid_ = value;
        SetDirty();
    }
}

void User::SetCollapseEntries(const bool value) {
    if (collapse_entries_ != value) {
        collapse_entries_ = value;
        SetDirty();
    }
}


bool User::HasValidSinceDate() const {
    // has no value
    if (!Since()) {
        return false;
    }

    // too old
    Poco::Timestamp ts = Poco::Timestamp::fromEpochTime(time(nullptr))
                         - (60 * Poco::Timespan::DAYS);
    Poco::Int64 min_allowed = ts.epochTime();
    if (Since() < min_allowed) {
        return false;
    }

    return true;
}

std::string User::String() const {
    std::stringstream ss;
    ss  << "ID=" << ID()
        << " local_id=" << LocalID()
        << " default_wid=" << default_wid_
        << " api_token=" << api_token_
        << " since=" << since_
        << " record_timeline=" << record_timeline_;
    return ss.str();
}

bool User::LoadUserPreferencesFromJSON(
    Json::Value data) {
    if (data.isMember("CollapseTimeEntries")
            && data["CollapseTimeEntries"].asBool() != CollapseEntries()) {
        SetCollapseEntries(data["CollapseTimeEntries"].asBool());
        return true;
    }
    return false;
}

error User::UserID(
    const std::string json_data_string,
    Poco::UInt64 *result) {
    *result = 0;
    Json::Value root;
    Json::Reader reader;
    bool ok = reader.parse(json_data_string, root);
    if (!ok) {
        return error("error parsing UserID JSON");
    }
    *result = root["data"]["id"].asUInt64();
    return noError;
}

error User::LoginToken(
    const std::string json_data_string,
    std::string *result) {
    *result = "";
    Json::Value root;
    Json::Reader reader;
    bool ok = reader.parse(json_data_string, root);
    if (!ok) {
        return error("error parsing UserID JSON");
    }
    *result = root["login_token"].asString();
    return noError;
}

std::string User::ModelName() const {
    return kModelUser;
}

std::string User::ModelURL() const {
    return "/api/v9/me";
}

}  // namespace toggl
