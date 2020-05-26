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

    private lazy var boxView = NSBox(frame: NSRect.zero)
    private lazy var colorCheckImageView = NSImageView(frame: NSRect.zero)

    // MARK: Public

    override func viewDidLoad() {
        super.viewDidLoad()
        initCommon()
    }

    override func loadView() {
      self.view = NSView()
      self.view.wantsLayer = true
    }
    
    func render(_ color: ProjectColor) {
        guard let color = ConvertHexColor.hexCode(toNSColor: color.hex) else { return }
        boxView.fillColor = color
    }

    override var isSelected: Bool {
        didSet {
            colorCheckImageView.isHidden = !isSelected
        }
    }
}

extension ColorViewItem {

    private func initCommon() {
        boxView.boxType = .custom
        boxView.cornerRadius = 12
        boxView.borderType = .noBorder
        view.addSubview(boxView)
        boxView.edgesToSuperView()

        colorCheckImageView.imageAlignment = .alignCenter
        colorCheckImageView.image = NSImage(named: "color-check")!
        colorCheckImageView.isHidden = true
        view.addSubview(colorCheckImageView)
        colorCheckImageView.edgesToSuperView()
    }
}
