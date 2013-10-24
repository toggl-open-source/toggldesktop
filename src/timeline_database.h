
// Copyright (c) 2013 Toggl

#ifndef SRC_TIMELINE_DATABASE_H_
#define SRC_TIMELINE_DATABASE_H_

#include "./timeline_event.h"
#include "./timeline_notifications.h"

#include <string>
#include <vector>

#include "Poco/NotificationCenter.h"
#include "Poco/Observer.h"
#include "Poco/Data/Common.h"
#include "Poco/Data/SQLite/Connector.h"
#include "Poco/Util/Subsystem.h"
#include "Poco/Util/Application.h"

class TimelineDatabase {
 public:
    TimelineDatabase() :
            ses_(0),
            desktop_id_(""),
            database_location_("") {
        Poco::Data::SQLite::Connector::registerConnector();

        Poco::NotificationCenter& nc =
            Poco::NotificationCenter::defaultCenter();

        Poco::Observer<TimelineDatabase, TimelineEventNotification>
            observeCreate(*this,
                &TimelineDatabase::handleTimelineEventNotification);
        nc.addObserver(observeCreate);

        Poco::Observer<TimelineDatabase, CreateTimelineBatchNotification>
            observeSelect(*this,
                &TimelineDatabase::handleCreateTimelineBatchNotification);
        nc.addObserver(observeSelect);

        Poco::Observer<TimelineDatabase, DeleteTimelineBatchNotification>
            observeDelete(*this,
                &TimelineDatabase::handleDeleteTimelineBatchNotification);
        nc.addObserver(observeDelete);
    }

    ~TimelineDatabase() {
        Poco::Data::SQLite::Connector::unregisterConnector();
    }

 protected:
    // Subsystem overrides
    const char* name() const { return "timeline_database"; }
    void initialize(Poco::Util::Application &app); // NOLINT
    void uninitialize();
    void defineOptions(Poco::Util::OptionSet& options); // NOLINT

    // Handle notifications
    void handleTimelineEventNotification(
        TimelineEventNotification* notification);
    void handleCreateTimelineBatchNotification(
        CreateTimelineBatchNotification *notification);
    void handleDeleteTimelineBatchNotification(
        DeleteTimelineBatchNotification *notification);

    // Handle command line params
    void handleConfigDataPath(const std::string& name,
        const std::string& value);

 private:
    // Returns the database for the functions in this interface.
    Poco::Data::Session *ses_;

    // UUID for this timeline install. UUID will be (re)set
    // upon timeline database creation.
    std::string desktop_id_;

    // Absolute path to database file location.
    std::string database_location_;

    // Actual data manipulation API
    void initialize_timeline_tables();
    void insert(const TimelineEvent& info);
    void select_batch(const int user_id,
        std::vector<TimelineEvent> *timeline_events);
    void delete_batch(const std::vector<TimelineEvent> &timeline_events);

    // Open/close actual sqlite database
    void open_database();
    void close_database();
};

#endif  // SRC_TIMELINE_DATABASE_H_
