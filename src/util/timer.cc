//
//  timer.cpp
//  TogglDesktopLibrary
//
//  Created by Nghia Tran on 4/13/20.
//  Copyright © 2020 Toggl. All rights reserved.
//

#include "timer.h"
#include <thread>

namespace toggl {

void Timer::SetTimeout(std::function<void ()> callback, int delay) {
    this->clear = false;
    std::thread t([=]() {
        if(this->clear) return;
        std::this_thread::sleep_for(std::chrono::milliseconds(delay));
        if(this->clear) return;
        callback();
    });
    t.detach();
}

void Timer::Stop() {
    this->clear = true;
}

}
