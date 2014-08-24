// Copyright 2014 Toggl Desktop developers.

#ifndef SRC_CUSTOM_ERROR_HANDLER_H_
#define SRC_CUSTOM_ERROR_HANDLER_H_

#include "Poco/ErrorHandler.h"
#include <iostream> // NOLINT

class custom_error_handler : public Poco::ErrorHandler {
 public:
    void exception(const Poco::Exception& exc) {
        std::cerr << "unhandled exception! " << exc.displayText() << std::endl;
    }
    void exception(const std::exception& exc) {
        std::cerr << "unhandled exception! " << exc.what() << std::endl;
    }
    void exception() {
        std::cerr << "unhandled exception! unknown exception" << std::endl;
    }
};

#endif  // SRC_CUSTOM_ERROR_HANDLER_H_
