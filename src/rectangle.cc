//
//  rectangle.c
//  TogglDesktopLibrary
//
//  Created by Nghia Tran on 11/22/18.
//  Copyright © 2018 Toggl. All rights reserved.
//

#include "rectangle.h"
#include <sstream>

namespace toggl {

std::string Rectangle::str() const {
    std::stringstream ss;
    ss << width << "x" << height;
    return ss.str();
}

}
