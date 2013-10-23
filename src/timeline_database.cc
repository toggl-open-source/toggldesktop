// Copyright (c) 2013 Toggl

#include "./timeline_database.h"

#include <limits>
#include <string>
#include <vector>

#include "Poco/Data/Common.h"
#include "Poco/Data/RecordSet.h"
#include "Poco/Data/Statement.h"
#include "Poco/Data/Binding.h"
#include "Poco/UUID.h"
#include "Poco/UUIDGenerator.h"
#include "Poco/Util/Option.h"
#include "Poco/Util/OptionCallback.h"

void TimelineDatabase::initialize(Poco::Util::Application & app) {
    open_database();
}

void TimelineDatabase::uninitialize() {
    close_database();
}

void TimelineDatabase::open_database() {
    if (!database_location_.empty()) {
        std::stringstream out;
        out << "open_database will open " << database_location_;
        Poco::Logger &logger = Poco::Logger::get("timeline_database");
        logger.debug(out.str());

        ses_ = new Poco::Data::Session("SQLite", database_location_);
        initialize_timeline_tables();
    }
}

void TimelineDatabase::close_database() {
    if (ses_) {
        delete ses_;
        ses_ = 0;
    }
}

void TimelineDatabase::initialize_timeline_tables() {
    std::string table_name;
    *ses_ << "SELECT name "
        "FROM sqlite_master "
        "WHERE type='table' "
        "AND name='timeline_installation'",
        Poco::Data::into(table_name),
        Poco::Data::limit(1),
        Poco::Data::now;

    if (table_name.length() == 0) {
        *ses_ << "CREATE TABLE timeline_installation("
            "id INTEGER PRIMARY KEY, "
            "desktop_id VARCHAR NOT NULL"
            ")", Poco::Data::now;
        Poco::UUIDGenerator& generator =
            Poco::UUIDGenerator::defaultGenerator();
        Poco::UUID uuid(generator.createRandom());
        desktop_id_ = uuid.toString();
        *ses_ << "INSERT INTO timeline_installation(desktop_id) "
            "VALUES(:desktop_id)",
            Poco::Data::use(desktop_id_),
            Poco::Data::now;
        *ses_ << "CREATE TABLE timeline_events("
            "id INTEGER PRIMARY KEY, "
            "user_id INTEGER NOT NULL, "
            "title VARCHAR, "
            "filename VARCHAR, "
            "start_time INTEGER NOT NULL, "
            "end_time INTEGER, "
            "idle INTEGER NOT NULL"
            ")",
            Poco::Data::now;
    } else {
        *ses_ << "SELECT desktop_id FROM timeline_installation",
            Poco::Data::into(desktop_id_),
            Poco::Data::lowerLimit(1),
            Poco::Data::now;
    }

    Poco::Logger &logger = Poco::Logger::get("timeline_database");
    logger.debug("desktop_id = " + desktop_id_);
    poco_assert(!desktop_id_.empty());
}

void TimelineDatabase::select_batch(
        const int user_id,
        std::vector<TimelineEvent> *timeline_events) {
    std::stringstream out;
    out << "select_batch, user_id = " << user_id;
    Poco::Logger &logger = Poco::Logger::get("timeline_database");
    logger.debug(out.str());

    poco_assert(user_id > 0);
    poco_assert(timeline_events->empty());
    if (!ses_) {
        logger.warning("select_batch database is not open, ignoring request");
        return;
    }
    Poco::Data::Statement select(*ses_);
    select << "SELECT id, title, filename, start_time, end_time, idle "
        "FROM timeline_events WHERE user_id = :user_id LIMIT 100",
        Poco::Data::use(user_id);
    Poco::Data::RecordSet rs(select);
    while (!select.done()) {
        select.execute();
        bool more = rs.moveFirst();
        while (more) {
            TimelineEvent event;
            event.id = rs[0].convert<unsigned int>();
            event.title = rs[1].convert<std::string>();
            event.filename = rs[2].convert<std::string>();
            event.start_time = rs[3].convert<int>();
            event.end_time = rs[4].convert<int>();
            event.idle = rs[5].convert<bool>();
            event.user_id = user_id;
            timeline_events->push_back(event);
            more = rs.moveNext();
        }
    }

    std::stringstream event_count;
    event_count << "select_batch found " << timeline_events->size()
        <<  " events.";
    logger.debug(event_count.str());
}

void TimelineDatabase::insert(const TimelineEvent& event) {
    std::stringstream out;
    out << "insert " << event.start_time << ";" << event.end_time << ";"
        << event.filename << ";" << event.title;
    Poco::Logger &logger = Poco::Logger::get("timeline_database");
    logger.information(out.str());

    poco_assert(event.user_id > 0);
    poco_assert(event.start_time > 0);
    poco_assert(event.end_time > 0);
    if (!ses_) {
        logger.information("insert database is not open, ignoring request");
        return;
    }
    *ses_ << "INSERT INTO timeline_events("
        "user_id, title, filename, start_time, end_time, idle"
        ") VALUES ("
        ":user_id, :title, :filename, :start_time, :end_time, :idle"
        ")",
        Poco::Data::use(event.user_id),
        Poco::Data::use(event.title),
        Poco::Data::use(event.filename),
        Poco::Data::use(event.start_time),
        Poco::Data::use(event.end_time),
        Poco::Data::use(event.idle),
        Poco::Data::now;
}

void TimelineDatabase::delete_batch(
        const std::vector<TimelineEvent> &timeline_events) {
    std::stringstream out;
    out << "delete_batch " << timeline_events.size() << " events.";
    Poco::Logger &logger = Poco::Logger::get("timeline_database");
    logger.debug(out.str());

    poco_assert(!timeline_events.empty());
    if (!ses_) {
        logger.warning("delete_batch database is not open, ignoring request");
        return;
    }
    std::vector<int> ids;
    for (std::vector<TimelineEvent>::const_iterator i = timeline_events.begin();
            i != timeline_events.end();
            ++i) {
        const TimelineEvent &event = *i;
        ids.push_back(event.id);
    }
    *ses_ << "DELETE FROM timeline_events WHERE id = :id",
        Poco::Data::use(ids),
        Poco::Data::now;
}

void TimelineDatabase::handleTimelineEventNotification(
        TimelineEventNotification* notification) {
    Poco::Logger &logger = Poco::Logger::get("timeline_database");
    logger.debug("handleTimelineEventNotification");
    insert(notification->event);
}

void TimelineDatabase::handleCreateTimelineBatchNotification(
        CreateTimelineBatchNotification* notification) {
    Poco::Logger &logger = Poco::Logger::get("timeline_database");
    logger.debug("handleCreateTimelineBatchNotification");
    std::vector<TimelineEvent> batch;
    select_batch(notification->user_id, &batch);
    Poco::NotificationCenter& nc = Poco::NotificationCenter::defaultCenter();
    TimelineBatchReadyNotification response(
        notification->user_id, batch, desktop_id_);
    Poco::AutoPtr<TimelineBatchReadyNotification> ptr(&response);
    nc.postNotification(ptr);
}

void TimelineDatabase::handleDeleteTimelineBatchNotification(
        DeleteTimelineBatchNotification* notification) {
    Poco::Logger &logger = Poco::Logger::get("timeline_database");
    logger.debug("handleDeleteTimelineBatchNotification");
    delete_batch(notification->batch);
}

void TimelineDatabase::defineOptions(Poco::Util::OptionSet& options) { // NOLINT
    options.addOption(
        Poco::Util::Option("database_path", "", "location of timeline database")
            .required(true)
            .repeatable(false)
            .argument("path")
            .callback(Poco::Util::OptionCallback<TimelineDatabase>(this,
                    &TimelineDatabase::handleConfigDataPath)));
}

void TimelineDatabase::handleConfigDataPath(const std::string& name,
        const std::string& value) {
    database_location_ = value;
}
