// Copyright 2013 Tanel Lebedev

#include <iostream> // NOLINT
#include <cstdlib>
#include <vector>

#include "./main.h"
#include "./toggl_api_client.h"
#include "./database.h"

#include "Poco/FileStream.h"
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

            err = user.Save(&db);
            if (err != noError) {
                logger.error(err);
                return Poco::Util::Application::EXIT_SOFTWARE;
            }
        }

        // FIXME: move out, later
        if ("test" == args[0]) {
            Poco::FileStream fis("test/me.json", std::ios::binary);
            poco_assert(fis.good());
            std::stringstream ss;
            ss << fis.rdbuf();
            fis.close();
            std::string json = ss.str();
            poco_assert(!json.empty());

            User user;
            error err = user.Load(json);
            if (err != noError) {
                logger.error(err);
                return Poco::Util::Application::EXIT_SOFTWARE;
            }

            poco_assert(user.Projects.size() == 2);
            Project p1 = user.Projects[0];
            poco_assert(p1.ID == 2598305);
            poco_assert(p1.GUID == "2f0b8f51-f898-d992-3e1a-6bc261fc41ef");
            poco_assert(p1.WID == 123456789);
            poco_assert(p1.CID == 0);
            poco_assert(p1.Name == "Testing stuff");
            Project p2 = user.Projects[1];
            poco_assert(p2.ID == 2567324);
            poco_assert(p2.GUID == "");
            poco_assert(p2.WID == 123456789);
            poco_assert(p2.CID == 1129807);
            poco_assert(p2.Name == "Even more work");

            poco_assert(user.TimeEntries.size() == 3);
            TimeEntry te1 = user.TimeEntries[0];
            poco_assert(te1.ID == 89818605);
            poco_assert(te1.GUID == "07fba193-91c4-0ec8-2894-820df0548a8f");
            poco_assert(te1.WID == 123456789);
            poco_assert(te1.PID == 2567324);
            poco_assert(te1.Start == "2013-09-05T06:33:50+00:00");
            poco_assert(te1.Stop == "2013-09-05T08:19:46+00:00");
            poco_assert(te1.DurationInSeconds == 6356);
            poco_assert(te1.Description == "Important things");
            poco_assert(te1.DurOnly == false);
            TimeEntry te2 = user.TimeEntries[1];
            poco_assert(te2.ID == 89833438);
            poco_assert(te2.GUID == "6c97dc31-582e-7662-1d6f-5e9d623b1685");
            TimeEntry te3 = user.TimeEntries[2];
            poco_assert(te3.ID == 89837259);
            poco_assert(te3.GUID == "6a958efd-0e9a-d777-7e19-001b2d7ced92");

            poco_assert(user.Workspaces.size() == 2);
            Workspace w1 = user.Workspaces[0];
            poco_assert(w1.ID == 123456788);
            poco_assert(w1.Name == "stuff");
            Workspace w2 = user.Workspaces[1];
            poco_assert(w2.ID == 123456789);
            poco_assert(w2.Name == "Toggl");

            poco_assert(user.Tasks.size() == 2);
            Task task1 = user.Tasks[0];
            poco_assert(task1.ID == 1894794);
            poco_assert(task1.Name == "blog (writing)");
            poco_assert(task1.WID == 123456789);
            poco_assert(task1.PID == 2585208);
            Task task2 = user.Tasks[1];
            poco_assert(task2.ID == 1879027);
            poco_assert(task2.Name == "dadsad");
            poco_assert(task2.WID == 123456789);
            poco_assert(task2.PID == 2707058);

            poco_assert(user.Clients.size() == 2);
            Client c1 = user.Clients[0];
            poco_assert(c1.ID == 1385144);
            poco_assert(c1.Name == "ABC");
            Client c2 = user.Clients[1];
            poco_assert(c2.ID == 878318);
            poco_assert(c2.Name == "Big Client");
            poco_assert(c2.GUID == "59b464cd-0f8e-e601-ff44-f135225a6738");
        }
    }

    return Poco::Util::Application::EXIT_OK;
}

}   // namespace kopsik
