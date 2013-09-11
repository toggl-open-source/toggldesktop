
#include "database.h"

#include <limits>
#include <string>
#include <vector>
#include <iostream>

#include "Poco/Logger.h"
#include "Poco/Data/Common.h"
#include "Poco/Data/RecordSet.h"
#include "Poco/Data/Common.h"
#include "Poco/Data/Statement.h"
#include "Poco/Data/Binding.h"

namespace kopsik {

Poco::Logger &Database::logger() {
    return Poco::Logger::get("database");
}

toggl::error Database::open_database() {
    ses_ = new Poco::Data::Session("SQLite", "kopsik.db");
    return initialize_tables();
}

void Database::close_database() {
    if (ses_) {
        delete ses_;
        ses_ = 0;
    }
}

toggl::error Database::initialize_tables() {
    poco_assert(!ses_);
    return toggl::noError;
}

toggl::error Database::save(toggl::User &user) {
    poco_assert(!ses_);
    return toggl::noError;

}

};