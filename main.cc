#include <iostream>
#include <cstdlib>

#include "main.h"
#include "toggl_api_client.h"
#include "database.h"

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
				std::cerr << "Please set TOGGL_API_TOKEN in environment" << std::endl;
				return Poco::Util::Application::EXIT_USAGE;
			}

			User user;
			user.APIToken = std::string(apiToken);

			error err = user.Fetch();
			if (err != noError) {
				logger.error(err);
				return Poco::Util::Application::EXIT_SOFTWARE;
			}

			err = user.Save(db);
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
			TimeEntry te2 = user.TimeEntries[1];
			TimeEntry te3 = user.TimeEntries[2];

			poco_assert(user.Workspaces.size() == 2);
			Workspace w1 = user.Workspaces[0];
			Workspace w2 = user.Workspaces[1];

			poco_assert(user.Tasks.size() == 2);
			Task task1 = user.Tasks[0];
			Task task2 = user.Tasks[1];

			poco_assert(user.Clients.size() == 2);
			Client c1 = user.Clients[0];
			Client c2 = user.Clients[1];
		}
	}

	return Poco::Util::Application::EXIT_OK;
}

}