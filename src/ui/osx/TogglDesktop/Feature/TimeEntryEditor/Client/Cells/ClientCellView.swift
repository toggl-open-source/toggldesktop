//
//  ClientCellView.swift
//  TogglDesktop
//
//  Created by Nghia Tran on 3/29/19.
//  Copyright © 2019 Alari. All rights reserved.
//

import Cocoa

final class ClientCellView: NSTableCellView {

    static let cellHeight: CGFloat = 34.0

    // MARK: OUTLET

    @IBOutlet weak var contentContainerView: NSBox!
    @IBOutlet weak var titleTextField: NSTextField!

    // MARK: Public

    override func awakeFromNib() {
        super.awakeFromNib()

        initCommon()
        initTracking()
    }

    func render(_ item: Client) {

    }

    override func mouseExited(with event: NSEvent) {
        super.mouseExited(with: event)
        NSCursor.arrow.set()
        contentContainerView.animator().alphaValue = 0.0
    }

    override func mouseEntered(with event: NSEvent) {
        super.mouseEntered(with: event)
        NSCursor.pointingHand.set()
        contentContainerView.animator().alphaValue = 1.0
    }

    fileprivate func initCommon() {
        contentContainerView.alphaValue = 0
    }

    fileprivate func initTracking() {
        let trackingArea = NSTrackingArea(rect: bounds,
                                          options: [.activeInKeyWindow, .inVisibleRect, .mouseEnteredAndExited],
                                          owner: self,
                                          userInfo: nil)
        addTrackingArea(trackingArea)
    }
}
