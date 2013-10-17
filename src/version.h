// Copyright 2013 Tanel Lebedev

#ifndef SRC_VERSION_H_
#define SRC_VERSION_H_

#include <string>
#include <sstream>

namespace kopsik {
  namespace version {
    static const unsigned int Major = 1;
    static const unsigned int Minor = 0;
    static const unsigned int Patch = 0;
  }
  static std::string UserAgent() {
    std::stringstream ss;
    ss  << "libkopsik/"
        << version::Major
        << "."
        << version::Minor
        << "."
        << version::Patch;
    return ss.str();
  }
}  // namespace kopsik

#endif  // SRC_VERSION_H_
