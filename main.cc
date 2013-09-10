#include <iostream>
#include <cstdlib>

#include "main.h"
#include "toggl_api_client.h"

#include "Poco/Message.h"
#include "Poco/Logger.h"
#include "Poco/Util/Application.h"

int Kopsik::main(const std::vector<std::string>& args) {
	char* apiToken = getenv("TOGGL_API_TOKEN");
	if (!apiToken) {
		std::cerr << "Please set TOGGL_API_TOKEN in environment" << std::endl;
		return Poco::Util::Application::EXIT_USAGE;
	}

	Poco::Logger &logger = Poco::Logger::get("");
	logger.setLevel(Poco::Message::PRIO_DEBUG);

	toggl::User user;
	user.APIToken = std::string(apiToken);
	toggl::error err = user.fetch();
	if (err != toggl::noError) {
		std::cerr << err << std::endl;
		return Poco::Util::Application::EXIT_SOFTWARE;
	}

	return Poco::Util::Application::EXIT_OK;
}
