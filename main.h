// Copyright 2013 Tanel Lebedev

#ifndef MAIN_H_
#define MAIN_H_

#include <string>
#include <vector>

#include "Poco/Util/Application.h"
#include "Poco/Util/OptionSet.h"

#include "./database.h"

namespace kopsik {

    class Kopsik : public Poco::Util::Application {
    public:
        Kopsik() {}
        ~Kopsik() {}

    protected:
        int main(const std::vector<std::string>& args);
        virtual void initialize(Poco::Util::Application &self) { // NOLINT
            Poco::Util::Application::initialize(self);
        };
        virtual void uninitialize() {}
        virtual void reinitialize() {}
        virtual void defineOptions(Poco::Util::OptionSet& options) {} // NOLINT
        void handleOption(const std::string &name, const std::string &value) {}

    private:
        void usage();
    };

}  // namespace kopsik

POCO_APP_MAIN(kopsik::Kopsik)

#endif  // MAIN_H_
