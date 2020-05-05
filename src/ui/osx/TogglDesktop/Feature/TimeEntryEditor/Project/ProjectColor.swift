//
//  ProjectColor.swift
//  TogglDesktop
//
//  Created by Nghia Tran on 3/28/19.
//  Copyright © 2019 Alari. All rights reserved.
//

import Foundation

struct ProjectColor: Equatable {

    static let `default` = ProjectColor(colorHex: "#9e5bd9")
    static let defaultColors: [ProjectColor] = [ProjectColor(colorHex: "#0b83d9"),
                                                ProjectColor(colorHex: "#9e5bd9"),
                                                ProjectColor(colorHex: "#d94182"),
                                                ProjectColor(colorHex: "#e36a00"),
                                                ProjectColor(colorHex: "#bf7000"),
                                                ProjectColor(colorHex: "#c7af14"),
                                                ProjectColor(colorHex: "#d92b2b"),
                                                ProjectColor(colorHex: "#2da608"),
                                                ProjectColor(colorHex: "#06a893"),
                                                ProjectColor(colorHex: "#c9806b"),
                                                ProjectColor(colorHex: "#465bb3"),
                                                ProjectColor(colorHex: "#990099"),
                                                ProjectColor(colorHex: "#566614"),
                                                ProjectColor(colorHex: "#525266")]

    let colorHex: String

    static func random() -> ProjectColor {
        return defaultColors.randomElement() ?? .default
    }
}
