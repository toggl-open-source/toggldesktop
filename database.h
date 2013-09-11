
#include "toggl_api_client.h"

#include <string>
#include <vector>

#include "Poco/Logger.h"
#include "Poco/Data/Common.h"
#include "Poco/Data/SQLite/Connector.h"

namespace kopsik {

class Database  {
public:
    Database() : ses_(0) {
        Poco::Data::SQLite::Connector::registerConnector();
    }

    ~Database() {
        Poco::Data::SQLite::Connector::unregisterConnector();
    }

private:
    Poco::Data::Session *ses_;

    toggl::error initialize_tables();
    toggl::error save(toggl::User &user);

    toggl::error open_database();
    void close_database();

    Poco::Logger &logger();
};

}
