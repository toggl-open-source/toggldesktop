//
//  TimeDecoratorView.swift
//  TogglDesktop
//
//  Created by Nghia Tran on 2/25/19.
//  Copyright © 2019 Alari. All rights reserved.
//

import Cocoa

final class TimeDecoratorView: NSView {

    @IBOutlet weak var backgroundBox: NSView!

    override func awakeFromNib() {
        super.awakeFromNib()

        wantsLayer = true
        layer?.masksToBounds = false
        backgroundBox.wantsLayer = true
        backgroundBox.layer?.masksToBounds = false
        backgroundBox.shadow = NSShadow()
        backgroundBox.layer?.shadowColor = NSColor.init(white: 0, alpha: 0.2).cgColor
        backgroundBox.layer?.shadowOpacity = 0.5
        backgroundBox.layer?.shadowOffset = CGSize(width: 0, height: -2)
        backgroundBox.layer?.shadowRadius = 8
        backgroundBox.layer?.cornerRadius = 4
    }

    override func updateLayer() {
        super.updateLayer()
        if #available(OSX 10.13, *) {
            backgroundBox.layer?.backgroundColor = NSColor(named: NSColor.Name("white-background-color"))?.cgColor
        } else {
            backgroundBox.layer?.backgroundColor = NSColor.white.cgColor
        }
    }
}
