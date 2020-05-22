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

namespace toggl {

enum ConvertType {
    ConvertTypeShapeOnLightBackground,
    ConvertTypeShapeOnDarkBackground,
    ConvertTypeTextOnLightBackground,
    ConvertTypeTextOnDarkBackground
};

// Range values from [0..1]
struct RgbColor {
    double r;
    double g;
    double b;
};

// Range values from [0..1]
struct HsvColor {
    double h;
    double s;
    double v;
};

class TOGGL_INTERNAL_EXPORT ColorConverter {
public:

    static HsvColor Convert(std::string hexColor, ConvertType type);
    static HsvColor Convert(RgbColor rgbColor, ConvertType type);

private:
    static HsvColor adjustColor(HsvColor hsvColor, ConvertType type);
    static HsvColor rgbToHsv(RgbColor rgbColor);
    static RgbColor hexToRgb(std::string hex);
};

}
#endif /* color_convert_hpp */
