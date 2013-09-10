#include <string>

#include "Poco/Util/Application.h"
#include "Poco/Util/OptionSet.h"

class Kopsik : public Poco::Util::Application {
public:
	Kopsik() {};
	~Kopsik() {};
protected:
	int main(const std::vector<std::string>& args);
	virtual void initialize(Poco::Util::Application &self) {
		Poco::Util::Application::initialize(self);
	};
	virtual void uninitialize() {};
	virtual void reinitialize() {};
	virtual void defineOptions(Poco::Util::OptionSet& options) {};
	void handleOption(const std::string &name, const std::string &value) {};
};

POCO_APP_MAIN(Kopsik)
