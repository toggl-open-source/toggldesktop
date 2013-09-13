#include <string>

#include "Poco/Util/Application.h"
#include "Poco/Util/OptionSet.h"

#include "database.h"

namespace kopsik {

class Kopsik : public Poco::Util::Application {
public:
	Kopsik() {};
	~Kopsik() {};

protected:
	int main(const std::vector<std::string>& args);
	virtual void initialize(Poco::Util::Application &self) {
		Poco::Util::Application::initialize(self);
		db.Open();
	};
	virtual void uninitialize() {
		db.Close();
	};
	virtual void reinitialize() {
		db.Close();
		db.Open();
	};
	virtual void defineOptions(Poco::Util::OptionSet& options) {
	};
	void handleOption(const std::string &name, const std::string &value) {
	};

private:
	Database db;
};

}

POCO_APP_MAIN(kopsik::Kopsik)