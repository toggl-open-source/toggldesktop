#include <iostream>
#include <cstdlib>

#include "toggl_api_client.h"

#include "Poco/SimpleFileChannel.h"
#include "Poco/FormattingChannel.h"
#include "Poco/PatternFormatter.h"
#include "Poco/Util/Application.h"

class Kopsik : public Poco::Util::Application {
protected:
    int main(const std::vector<std::string>& args);
};

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

POCO_APP_MAIN(Kopsik)
