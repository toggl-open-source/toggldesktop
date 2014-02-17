// Copyright 2013 Tanel Lebedev

#ifndef SRC_UI_CMDLINE_MAIN_H_
#define SRC_UI_CMDLINE_MAIN_H_

#include <string>
#include <sstream>
#include <vector>
#include <iostream> // NOLINT

#include "../../kopsik_api.h"

#include "Poco/Util/Application.h"
#include "Poco/Util/OptionSet.h"
#include "Poco/ErrorHandler.h"

namespace command_line_client {

    std::string model_change_to_string(
            KopsikModelChange &change) {
        std::stringstream ss;
        ss  << "model_type=" << change.ModelType
            << ", change_type=" << change.ChangeType
            << ", model_id=" << change.ModelID
            << ", GUID=" << change.GUID;
        return ss.str();
    }

    void main_change_callback(
            kopsik_api_result result,
            const char *errmsg,
            KopsikModelChange *change) {
        if (KOPSIK_API_SUCCESS != result) {
            std::cerr << "main_change_callback "
                << std::string(errmsg)
                << std::endl;
            return;
        }
        std::cout << "main_change_callback "
            << model_change_to_string(*change)
            << std::endl;
        }

    void main_on_error_callback(
            const char *errmsg) {
        std::cerr << "main_on_error_callback "
            << std::string(errmsg)
            << std::endl;
    }

    void main_check_updates_callback(
            kopsik_api_result result,
            const char *errmsg,
            const int is_update_available,
            const char *url,
            const char *version) {
    }

    class Main : public Poco::Util::Application, Poco::ErrorHandler {
    public:
        Main() : ctx(0) {
            kopsik_set_log_path("kopsik.log");

            ctx = kopsik_context_init(
                "cmdline",
                "0.0.1",
                main_change_callback,
                main_on_error_callback,
                main_check_updates_callback);
            poco_assert(ctx);
        }
        ~Main() {
            kopsik_context_clear(ctx);
        }

        // ErrorHandler
        void exception(const Poco::Exception& exc) {
            std::cerr << exc.displayText() << std::endl;
        }
        void exception(const std::exception& exc) {
            std::cerr << exc.what() << std::endl;
        }
        void exception() {
            std::cerr << "unknown exception" << std::endl;
        }

    protected:
        int main(const std::vector<std::string>& args);
        virtual void initialize(Poco::Util::Application &self) { // NOLINT
            Poco::Util::Application::initialize(self);
        };
        virtual void uninitialize() {}
        virtual void defineOptions(Poco::Util::OptionSet& options); // NOLINT
        void handleOption(const std::string &name, const std::string &value) {}

    private:
        void usage();
        void *ctx;
};

}  // namespace command_line_client

POCO_APP_MAIN(command_line_client::Main)

#endif  // SRC_UI_CMDLINE_MAIN_H_
