//
//  TagTokenView.swift
//  TogglDesktop
//
//  Created by Nghia Tran on 4/5/19.
//  Copyright © 2019 Alari. All rights reserved.
//

import Cocoa

protocol TagTokenViewDelegate: class {

    func tagTokenShouldDelete(with tag: Tag, sender: TagTokenView)
    func tagTokenShouldOpenAutoCompleteView()
}

final class TagTokenView: NSView {

    // MARK: OUTLET

    @IBOutlet weak var boxContainerView: NSBox!
    @IBOutlet weak var titleLabel: NSTextField!
    @IBOutlet weak var closeButton: CursorButton!
    @IBOutlet weak var gradientView: NSImageView!
    @IBOutlet weak var actionButton: NSButton!
    
    // MARK: Variables

    weak var delegate: TagTokenViewDelegate?
    private var tagToken: Tag!

    // MARK: Views

    override func awakeFromNib() {
        super.awakeFromNib()

        initCommon()
        initTracking()
    }

    deinit {
        trackingAreas.forEach {
            removeTrackingArea($0)
        }
    }

    func render(_ tag: Tag) {
        self.tagToken = tag
        titleLabel.stringValue = tag.name
    }

    @IBAction func deleteBtnOnTap(_ sender: Any) {
        // Show autoComplete if it's .. tag
        if tagToken.isMoreTag {
            delegate?.tagTokenShouldOpenAutoCompleteView()
            return
        }

        // Delete
        delegate?.tagTokenShouldDelete(with: tagToken, sender: self)
    }

    @IBAction func clickedBtnOnTap(_ sender: Any) {
        delegate?.tagTokenShouldOpenAutoCompleteView()
    }

    override func mouseExited(with event: NSEvent) {
        super.mouseExited(with: event)
        boxContainerView.animator().alphaValue = 1.0
        closeButton.animator().alphaValue = 0.0
        gradientView.animator().alphaValue = 0
    }

    override func mouseEntered(with event: NSEvent) {
        super.mouseEntered(with: event)
        boxContainerView.animator().alphaValue = 0.0
        if let tag = tagToken, !tag.isMoreTag {
            closeButton.animator().alphaValue = 1.0
            gradientView.animator().alphaValue = 1.0
        }
    }

    fileprivate func initTracking() {
        let trackingArea = NSTrackingArea(rect: bounds,
                                          options: [.activeInActiveApp, .inVisibleRect, .mouseEnteredAndExited],
                                          owner: self,
                                          userInfo: nil)
        addTrackingArea(trackingArea)
    }
}

// MARK: Private

extension TagTokenView {

    fileprivate func initCommon() {
        boxContainerView.alphaValue = 1
        closeButton.alphaValue = 0
        gradientView.alphaValue = 0
        closeButton.cursor = .pointingHand
    }
}
