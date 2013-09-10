#include "Poco/Util/Application.h"

class Kopsik : public Poco::Util::Application {
protected:
    int main(const std::vector<std::string>& args);
};

POCO_APP_MAIN(Kopsik)
