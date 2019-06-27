// Copyright 2014 Toggl Desktop developers.

#ifndef SRC_RANDOM_H
#define SRC_RANDOM_H

#include "Poco/Random.h"

namespace toggl {

class Random
{
public:
    static inline Poco::UInt32 next(Poco::UInt32 n) {
        static bool initialized = false;
        if (!initialized) {
            _random.seed();
            initialized = true;
        }
        return _random.next(n);
    }
private:
    static Poco::Random _random;
};

}  // namespace toggl
#endif // SRC_RANDOM_H
