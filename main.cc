#include <iostream>
#include <cstdlib>

#include "main.h"
#include "toggl_api_client.h"
#include "database.h"

#include "Poco/Message.h"
#include "Poco/Logger.h"
#include "Poco/Util/Application.h"

namespace kopsik {

int Kopsik::main(const std::vector<std::string>& args) {
	char* apiToken = getenv("TOGGL_API_TOKEN");
	if (!apiToken) {
		std::cerr << "Please set TOGGL_API_TOKEN in environment" << std::endl;
		return Poco::Util::Application::EXIT_USAGE;
	}

	Poco::Logger &logger = Poco::Logger::get("");
	logger.setLevel(Poco::Message::PRIO_DEBUG);

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

	return Poco::Util::Application::EXIT_OK;
}

}