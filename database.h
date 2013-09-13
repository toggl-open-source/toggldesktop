#ifndef _kopsik_database_h_
#define _kopsik_database_h_

#include <string>
#include <vector>

#include "Poco/Logger.h"
#include "Poco/Data/Common.h"
#include "Poco/Data/SQLite/Connector.h"

#include "types.h"
#include "toggl_api_client.h"

namespace kopsik {

class Database  {
public:
    Database() : session(0) {
        Poco::Data::SQLite::Connector::registerConnector();
    }

    ~Database() {
        Poco::Data::SQLite::Connector::unregisterConnector();
    }

    Poco::Data::Session *session;

    error Open();
    void Close();

    error Save(const User &user);

private:
    error initialize_tables();
    error migrate(std::string name, std::string sql);
};

}

#endif // _kopsik_database_h_