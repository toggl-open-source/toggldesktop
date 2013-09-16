// Copyright 2013 Tanel Lebedev

#include <iostream> // NOLINT
#include <cstdlib>
#include <vector>

#include "./toggl_api_client.h"
#include "./database.h"
#include "./main.h"

#include "Poco/Message.h"
#include "Poco/Logger.h"
#include "Poco/Util/Application.h"

namespace kopsik {

    int Kopsik::main(const std::vector<std::string>& args) {
        Poco::Logger &logger = Poco::Logger::get("");
        logger.setLevel(Poco::Message::PRIO_DEBUG);

        if (!args.empty()) {
            if ("sync" == args[0]) {
                char* apiToken = getenv("TOGGL_API_TOKEN");
                if (!apiToken) {
                    std::cerr << "Please set TOGGL_API_TOKEN in environment"
                        << std::endl;
                    return Poco::Util::Application::EXIT_USAGE;
                }

                User user;
                user.APIToken = std::string(apiToken);

                error err = user.Fetch();
                if (err != noError) {
                    logger.error(err);
                    return Poco::Util::Application::EXIT_SOFTWARE;
                }

                Database db("kopsik.db");

                err = db.Save(&user, true);
                if (err != noError) {
                    logger.error(err);
                    return Poco::Util::Application::EXIT_SOFTWARE;
                }
            }
        }

         return Poco::Util::Application::EXIT_OK;
    }

}  // namespace kopsik
