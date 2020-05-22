//
//  color_convert.cpp
//  TogglDesktopLibrary
//
//  Created by Nghia Tran on 5/22/20.
//  Copyright © 2020 Toggl. All rights reserved.
//

#include "color_convert.h"
#include <math.h>
#include <sstream>
#include <iomanip>

namespace toggl {

HsvColor ColorConverter::Convert(std::string hexColor, ConvertType type) {
    RgbColor rbg = hexToRgb(hexColor);
    return Convert(rbg, type);
}

HsvColor ColorConverter::Convert(RgbColor rgbColor, ConvertType type) {
    HsvColor hsvColor = rgbToHsv(rgbColor);
    return adjustColor(hsvColor, type);
}

HsvColor ColorConverter::adjustColor(HsvColor hsvColor, ConvertType type) {
    switch (type) {
        case ConvertTypeShapeOnLightBackground:
            return { hsvColor.h, hsvColor.s, hsvColor.v };
        case ConvertTypeTextOnLightBackground:
            return { hsvColor.h, hsvColor.s, hsvColor.v - 0.15 };
        case ConvertTypeShapeOnDarkBackground:
            return { hsvColor.h, hsvColor.s * hsvColor.v, (hsvColor.v + 2.0) / 3.0 };
        case ConvertTypeTextOnDarkBackground:
            return { hsvColor.h, hsvColor.s * hsvColor.v, 0.05 + (hsvColor.v + 2.0) / 3.0 };
        default:
            return hsvColor;
    }
    return hsvColor;
}

#define min_f(a, b, c)  (fminf(a, fminf(b, c)))
#define max_f(a, b, c)  (fmaxf(a, fmaxf(b, c)))

HsvColor ColorConverter::rgbToHsv(RgbColor rgbColor)
{
    float r = rgbColor.r;
    float g = rgbColor.g;
    float b = rgbColor.b;

    float h, s, v; // h:0-360.0, s:0.0-1.0, v:0.0-1.0

    float max = max_f(r, g, b);
    float min = min_f(r, g, b);

    v = max;

    if (max == 0.0f) {
        s = 0;
        h = 0;
    }
    else if (max - min == 0.0f) {
        s = 0;
        h = 0;
    }
    else {
        s = (max - min) / max;

        if (max == r) {
            h = 60 * ((g - b) / (max - min)) + 0;
        }
        else if (max == g) {
            h = 60 * ((b - r) / (max - min)) + 120;
        }
        else {
            h = 60 * ((r - g) / (max - min)) + 240;
        }
    }

    if (h < 0) h += 360.0f;

    return {h / 360.0, s, v};
}

RgbColor ColorConverter::hexToRgb(std::string hex)
{
    // Convert to hex value
    std::istringstream converter(hex);
    unsigned int hexValue;
    converter >> std::hex >> hexValue;

    // Convert to rgb
    float r = ((hexValue >> 16) & 0xFF) / 255.0;
    float g = ((hexValue >> 8) & 0xFF) / 255.0;
    float b = ((hexValue) & 0xFF) / 255.0;

    return { r, g, b };
}

}
