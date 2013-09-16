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

        char* apiToken = getenv("TOGGL_API_TOKEN");
        if (!apiToken) {
            std::cerr << "Please set TOGGL_API_TOKEN in environment"
                << std::endl;
            return Poco::Util::Application::EXIT_USAGE;
        }
        if (args.empty()) {
            std::cout << "Recognized commands are: sync, start, stop"
                << std::endl;
            return Poco::Util::Application::EXIT_USAGE;
        }

        Database db("kopsik.db");

        // Load user - we know current user by the TOGGL_API_TOKEN parameter.
        User user;
        error err = db.Load(apiToken, &user, true);
        if (err != noError) {
            logger.error(err);
            return Poco::Util::Application::EXIT_SOFTWARE;
        }
        poco_assert(user.ID != 0);
        poco_assert(!user.APIToken.empty());

        // Run a command on the user that has been loaded
        if ("sync" == args[0]) {
            err = user.Fetch();
        } else if ("start" == args[0]) {
            err = user.Start();
        } else if ("stop" == args[0]) {
            err = user.Stop();
        }

        // Check command result
        if (err != noError) {
            logger.error(err);
            return Poco::Util::Application::EXIT_SOFTWARE;
        }

        // If still not blown up, save state and exit.
        err = db.Save(&user, true);
        if (err != noError) {
            logger.error(err);
            return Poco::Util::Application::EXIT_SOFTWARE;
        }

        return Poco::Util::Application::EXIT_OK;
    }

}  // namespace kopsik
