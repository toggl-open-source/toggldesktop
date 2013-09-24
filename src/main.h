// Copyright 2013 Tanel Lebedev

#ifndef SRC_MAIN_H_
#define SRC_MAIN_H_

#include <string>
#include <vector>

#include "Poco/Util/Application.h"
#include "Poco/Util/OptionSet.h"

namespace command_line_client {

    class Main : public Poco::Util::Application {
    public:
        Main() {}
        ~Main() {}

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

}  // namespace command_line_client

POCO_APP_MAIN(command_line_client::Main)

#endif  // SRC_MAIN_H_
