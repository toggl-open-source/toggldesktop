//
//  color_convert.hpp
//  TogglTrackLibrary
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

    static TogglHsvColor GetAdaptiveColor(std::string hexColor, TogglAdaptiveColor type);
    static TogglHsvColor GetAdaptiveColor(TogglRgbColor rgbColor, TogglAdaptiveColor type);
    static TogglRgbColor GetRgbAdaptiveColor(std::string hexColor, TogglAdaptiveColor type);

 private:
    static TogglHsvColor adjustColor(TogglHsvColor hsvColor, TogglAdaptiveColor type);
    static TogglHsvColor rgbToHsv(TogglRgbColor rgbColor);
    static TogglRgbColor hexToRgb(std::string hex);
    static std::string rgbToHex(TogglRgbColor rbg);
    static TogglRgbColor hsvToRgb(TogglHsvColor hsvColor);
};

}
#endif /* color_convert_hpp */
