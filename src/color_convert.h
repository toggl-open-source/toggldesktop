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

    static HsvColor GetAdaptiveColor(std::string hexColor, AdaptiveColor type);
    static HsvColor GetAdaptiveColor(RgbColor rgbColor, AdaptiveColor type);
    static std::string GetHexAdaptiveColor(std::string hexColor, AdaptiveColor type);

private:
    static HsvColor adjustColor(HsvColor hsvColor, AdaptiveColor type);
    static HsvColor rgbToHsv(RgbColor rgbColor);
    static RgbColor hexToRgb(std::string hex);
    static std::string rgbToHex(RgbColor rbg);
    static RgbColor hsvToRgb(HsvColor hsvColor);
};

}
#endif /* color_convert_hpp */
