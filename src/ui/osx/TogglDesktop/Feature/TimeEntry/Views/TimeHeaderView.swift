//
//  TimeHeaderView.swift
//  TogglDesktop
//
//  Created by Nghia Tran on 2/20/19.
//  Copyright © 2019 Alari. All rights reserved.
//

import Cocoa

protocol TimeHeaderViewDelegate: class {

    func togglSection(at section: Int)
}

final class TimeHeaderView: NSView {

    // MARK: Varibles

    @IBOutlet weak var dateLbl: NSTextField!
    @IBOutlet weak var totalDurationLbl: NSTextField!
    @IBOutlet weak var arrowBtn: NSButton!

    private var section = -1
    weak var delegate: TimeHeaderViewDelegate?
    var cursor: NSCursor? {
        didSet {
            resetCursorRects()
        }
    }

    // MARK: Public

    override func resetCursorRects() {
        if let cursor = cursor {
            addCursorRect(bounds, cursor: cursor)
        } else {
            super.resetCursorRects()
        }
    }

    override func awakeFromNib() {
        super.awakeFromNib()
        initCommon()
        initDefaultValue()
    }

    override func prepareForReuse() {
        super.prepareForReuse()
        initDefaultValue()
    }

    func config(_ header: TimeEntryHeader, section: Int) {
        self.section = section
        dateLbl.stringValue = header.date
        totalDurationLbl.stringValue = header.totalTime
    }

    override func mouseDown(with event: NSEvent) {
        super.mouseDown(with: event)
        openSectionOnTap(arrowBtn)
    }

    @IBAction func openSectionOnTap(_ sender: Any) {
        delegate?.togglSection(at: section)
    }

    override func mouseEntered(with event: NSEvent) {
        super.mouseEntered(with: event)
        arrowBtn.isHidden = false
    }

    override func mouseExited(with event: NSEvent) {
        super.mouseExited(with: event)
        arrowBtn.isHidden = true
    }
}

// MARK: Private

extension TimeHeaderView {

    fileprivate func initDefaultValue() {
        cursor = .pointingHand
        arrowBtn.isHidden = true
    }

    fileprivate func initCommon() {
        let bounds = NSRect(x: 0, y: 0, width: NSScreen.main?.frame.width ?? frame.width, height: frame.height)
        let trackingArea = NSTrackingArea(rect: bounds, options: [.activeInKeyWindow, .inVisibleRect, .mouseEnteredAndExited], owner: self, userInfo: nil)
        addTrackingArea(trackingArea)
    }
}
