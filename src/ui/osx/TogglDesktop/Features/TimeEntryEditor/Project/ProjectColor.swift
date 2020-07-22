//
//  ProjectColor.swift
//  TogglDesktop
//
//  Created by Nghia Tran on 3/28/19.
//  Copyright © 2019 Alari. All rights reserved.
//

import Foundation

final class ProjectColorPool {

    static let shared = ProjectColorPool()

    // MARK: Variables

    private(set) var colors: [ProjectColor] = []
    let defaultColor = ProjectColor(hex: "#9e5bd9")
    let defaultCustomColor = HSV(h: 210, s: 0.74, v: 0.75) // #3177BE

    // MARK: Public

    func updateDefaultColors(_ colors: [String]) {
        self.colors = colors.map { ProjectColor(hex: $0) }
    }

    func random() -> ProjectColor {
        return colors.randomElement() ?? defaultColor
    }
}

struct ProjectColor: Equatable {
    let hex: String

    static func == (lhs: Self, rhs: Self) -> Bool {
        return lhs.hex == rhs.hex
    }
}

@objcMembers
final class ProjectColorPoolObjc: NSObject {

    class func updateDefaultColors(_ colors: [String]) {
        ProjectColorPool.shared.updateDefaultColors(colors)
    }
}
