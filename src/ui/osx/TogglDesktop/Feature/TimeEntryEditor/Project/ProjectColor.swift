//
//  ProjectColor.swift
//  TogglDesktop
//
//  Created by Nghia Tran on 3/28/19.
//  Copyright © 2019 Alari. All rights reserved.
//

import Foundation

struct ProjectColor: Equatable {

    static let `default` = ProjectColor(colorHex: "#c56bff")
    static let defaultColors: [ProjectColor] = [ProjectColor(colorHex: "#06aaf5"),
                                                ProjectColor(colorHex: "#c56bff"),
                                                ProjectColor(colorHex: "#ea468d"),
                                                ProjectColor(colorHex: "#fb8b14"),
                                                ProjectColor(colorHex: "#c7741c"),
                                                ProjectColor(colorHex: "#f1c33f"),
                                                ProjectColor(colorHex: "#e20505"),
                                                ProjectColor(colorHex: "#4bc800"),
                                                ProjectColor(colorHex: "#04bb9b"),
                                                ProjectColor(colorHex: "#e19a86"),
                                                ProjectColor(colorHex: "#3750b5"),
                                                ProjectColor(colorHex: "#a01aa5"),
                                                ProjectColor(colorHex: "#205500"),
                                                ProjectColor(colorHex: "#000000")]

    let colorHex: String

    static func random() -> ProjectColor {
        return defaultColors.randomElement() ?? .default
    }
}
