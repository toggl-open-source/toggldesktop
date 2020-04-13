//
//  timer.hpp
//  TogglDesktopLibrary
//
//  Created by Nghia Tran on 4/13/20.
//  Copyright © 2020 Toggl. All rights reserved.
//

#ifndef timer_hpp
#define timer_hpp

#include <string>
#include <Poco/Types.h>
#include <stdio.h>

namespace toggl {

class Timer {
    bool clear = false;

public:
    void SetTimeout(std::function<void ()> callback, int delay);
    void Stop();
};
}
#endif /* timer_hpp */
