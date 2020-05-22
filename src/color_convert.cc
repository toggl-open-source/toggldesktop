//
//  color_convert.cpp
//  TogglDesktopLibrary
//
//  Created by Nghia Tran on 5/22/20.
//  Copyright © 2020 Toggl. All rights reserved.
//

#include "color_convert.h"

namespace toggl {

HsvColor ColorConverter::Convert(RgbColor rgbColor, ConvertType type) {
    HsvColor hsvColor = { 1, 1, 1 };
    return hsvColor;
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
}
