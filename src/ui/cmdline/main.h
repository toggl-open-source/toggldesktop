// Copyright 2013 Tanel Lebedev

#ifndef SRC_UI_CMDLINE_MAIN_H_
#define SRC_UI_CMDLINE_MAIN_H_

#include <string>
#include <vector>
#include <iostream> // NOLINT

#include "../../kopsik_api.h"

#include "Poco/Util/Application.h"
#include "Poco/Util/OptionSet.h"
#include "Poco/ErrorHandler.h"

namespace command_line_client {

class Main : public Poco::Util::Application, Poco::ErrorHandler {
 public:
    Main();
    ~Main();

    // ErrorHandler
    void exception(const Poco::Exception& exc);
    void exception(const std::exception& exc);
    void exception();

 protected:
    // Application
    int main(const std::vector<std::string>& args);
    virtual void initialize(Poco::Util::Application &self); // NOLINT
    virtual void uninitialize() {}
    virtual void defineOptions(Poco::Util::OptionSet& options); // NOLINT
    void handleOption(const std::string &name, const std::string &value) {}

 private:
    void *ctx_;

    void usage() const;

    int sync();
    int continueTimeEntry();
    int status();
    int listenToWebSocket();
    int showPushableData();
    int listTimeEntries();
    int startTimeEntry();
    int stopTimeEntry();

    static std::string modelChangeToString(KopsikModelChange * const);
    static std::string timeEntryToString(KopsikTimeEntryViewItem * const);
};

}  // namespace command_line_client

POCO_APP_MAIN(command_line_client::Main)

#endif  // SRC_UI_CMDLINE_MAIN_H_
