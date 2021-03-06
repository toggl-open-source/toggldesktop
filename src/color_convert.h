//
//  color_convert.hpp
//  TogglDesktopLibrary
//
//  Created by Nghia Tran on 5/22/20.
//  Copyright © 2020 Toggl. All rights reserved.
//

#ifndef color_convert_h
#define color_convert_h

#include <string>
#include <Poco/Types.h>
#include <stdio.h>

#include "types.h"
#include "toggl_api.h"

namespace toggl {

class TOGGL_INTERNAL_EXPORT ColorConverter {
 public:

    static TogglHsvColor GetAdaptiveColor(const std::string &hexColor, TogglAdaptiveColor type);
    static TogglHsvColor GetAdaptiveColor(const TogglRgbColor &rgbColor, TogglAdaptiveColor type);
    static TogglRgbColor GetRgbAdaptiveColor(const std::string &hexColor, TogglAdaptiveColor type);

 private:
    static TogglHsvColor adjustColor(const TogglHsvColor &hsvColor, TogglAdaptiveColor type);
    static TogglHsvColor rgbToHsv(const TogglRgbColor &rgbColor);
    static TogglRgbColor hexToRgb(std::string hex);
    static TogglRgbColor hsvToRgb(const TogglHsvColor &hsvColor);
};

}
#endif /* color_convert_hpp */
