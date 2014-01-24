//
//  CustomerErrorHandler.h
//  kopsik
//
//  Created by Tanel Lebedev on 26/12/2013.
//  Copyright (c) 2013 kopsik developers. All rights reserved.
//

#ifndef SRC_CUSTOMERRORHANDLER_H_
#define SRC_CUSTOMERRORHANDLER_H_

#include "Poco/ErrorHandler.h"
#include <iostream> // NOLINT

class CustomErrorHandler : public Poco::ErrorHandler {
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

#endif  // SRC_CUSTOMERRORHANDLER_H_
