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

    static HsvColor getAdaptiveColor(std::string hexColor, ConvertType type);
    static HsvColor getAdaptiveColor(RgbColor rgbColor, ConvertType type);

private:
    static HsvColor adjustColor(HsvColor hsvColor, ConvertType type);
    static HsvColor rgbToHsv(RgbColor rgbColor);
    static RgbColor hexToRgb(std::string hex);
};

}
#endif /* color_convert_hpp */
