//
//  ColorViewItem.swift
//  TogglDesktop
//
//  Created by Nghia Tran on 3/28/19.
//  Copyright © 2019 Alari. All rights reserved.
//

import Cocoa

final class ColorViewItem: NSCollectionViewItem {

    // MARK: OUTLET

    @IBOutlet weak var boxView: NSBox!

    // MARK: View Cycle

    override func viewDidLoad() {
        super.viewDidLoad()

        initCommon()
    }

    // MARK: Public

    func render(_ color: ProjectColor) {
        boxView.fillColor = ConvertHexColor.hexCode(toNSColor: color.colorHex)!
    }

    private func initCommon() {

    }
}
