//
//  Rectangle.c
//  TogglDesktopLibrary
//
//  Created by Nghia Tran on 11/22/18.
//  Copyright © 2018 Toggl. All rights reserved.
//

#include "Rectangle.h"
#include <sstream>

namespace toggl {

std::string Rectangle::actionStr() const {
    std::stringstream ss;
    ss << "winsize-{"
        << width
        << "}x{"
        << height
        << "}";
    return ss.str();
}
    
}
