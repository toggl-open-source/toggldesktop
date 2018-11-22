//
//  Rectangle.h
//  TogglDesktopLibrary
//
//  Created by Nghia Tran on 11/22/18.
//  Copyright © 2018 Toggl. All rights reserved.
//

#ifndef Rectangle_h
#define Rectangle_h

#include <string>
#include "Poco/Types.h"

namespace toggl {

class Rectangle {
    private:
        Poco::UInt64 width;
        Poco::UInt64 height;

    public:
        Rectangle(Poco::UInt64 w=0, Poco::UInt64 h=0);

        std::string actionStr() const;
    };
}

#endif /* Rectangle_h */
