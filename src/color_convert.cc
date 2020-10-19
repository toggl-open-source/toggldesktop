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
#include <algorithm>

namespace toggl {

#define min_f(a, b, c)  (fminf(a, fminf(b, c)))
#define max_f(a, b, c)  (fmaxf(a, fmaxf(b, c)))
#define safe_range_f(value, min, max) (fmin(max, fmax(value, min)))

TogglHsvColor ColorConverter::GetAdaptiveColor(const std::string &hexColor, TogglAdaptiveColor type) {
    TogglRgbColor rbg = hexToRgb(hexColor);
    return GetAdaptiveColor(rbg, type);
}

TogglHsvColor ColorConverter::GetAdaptiveColor(const TogglRgbColor &rgbColor, TogglAdaptiveColor type) {
    TogglHsvColor hsvColor = rgbToHsv(rgbColor);
    return adjustColor(hsvColor, type);
}

TogglRgbColor ColorConverter::GetRgbAdaptiveColor(const std::string &hexColor, TogglAdaptiveColor type) {
    TogglHsvColor hsv = GetAdaptiveColor(hexColor, type);
    return hsvToRgb(hsv);
}

TogglHsvColor ColorConverter::adjustColor(TogglHsvColor hsvColor, TogglAdaptiveColor type) {
    switch (type) {
    case AdaptiveColorShapeOnLightBackground:
        return { hsvColor.h, hsvColor.s, hsvColor.v };
    case AdaptiveColorTextOnLightBackground:
        return { hsvColor.h, hsvColor.s, safe_range_f(hsvColor.v - 0.15, 0.0f, 1.0f) };
    case AdaptiveColorShapeOnDarkBackground:
        return { hsvColor.h, hsvColor.s * hsvColor.v, safe_range_f((hsvColor.v + 2.0) / 3.0, 0.0f, 1.0f) };
    case AdaptiveColorTextOnDarkBackground:
        return { hsvColor.h, hsvColor.s * hsvColor.v, safe_range_f(0.05 + (hsvColor.v + 2.0) / 3.0, 0.0f, 1.0f) };
    default:
        return hsvColor;
    }
    return hsvColor;
}

TogglHsvColor ColorConverter::rgbToHsv(TogglRgbColor rgbColor)
{
    double r = rgbColor.r;
    double g = rgbColor.g;
    double b = rgbColor.b;

    double h, s, v; // h:0-360.0, s:0.0-1.0, v:0.0-1.0

    double max = max_f(r, g, b);
    double min = min_f(r, g, b);

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

    return {h / 360.0, s, v}; // Range from 0..1
}

TogglRgbColor ColorConverter::hexToRgb(std::string hex)
{
    // Convert to hex value
    std::istringstream converter(hex);
    unsigned int hexValue;
    converter >> std::hex >> hexValue;

    // Convert to rgb
    double r = ((hexValue >> 16) & 0xFF) / 255.0;
    double g = ((hexValue >> 8) & 0xFF) / 255.0;
    double b = ((hexValue) & 0xFF) / 255.0;

    return { r, g, b };
}

TogglRgbColor ColorConverter::hsvToRgb(TogglHsvColor hsvColor) {
    double h = hsvColor.h;
    double s = hsvColor.s;
    double v = hsvColor.v;
    double r = 0.0, g = 0.0, b = 0.0;

    int i = int(h * 6);
    double f = h * 6 - i;
    double p = v * (1 - s);
    double q = v * (1 - f * s);
    double t = v * (1 - (1 - f) * s);

    switch(i % 6) {
    case 0: {
        r = v;
        g = t;
        b = p;
        break;
    }
    case 1: {
        r = q;
        g = v;
        b = p;
        break;
    }
    case 2: {
        r = p;
        g = v;
        b = t;
        break;
    }
    case 3: {
        r = p;
        g = q;
        b = v;
        break;
    }
    case 4: {
        r = t;
        g = p;
        b = v;
        break;
    }
    case 5: {
        r = v;
        g = p;
        b = q;
        break;
    }
    }
    return { r, g, b };
}

}
