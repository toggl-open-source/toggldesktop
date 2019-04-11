//
//  TagCellView.swift
//  TogglDesktop
//
//  Created by Nghia Tran on 4/5/19.
//  Copyright © 2019 Alari. All rights reserved.
//

import Cocoa

protocol TagCellViewDelegate: class {

    func tagSelectionStateOnChange(with tag: Tag, isSelected: Bool)
}

final class TagCellView: NSTableCellView {

    static let cellHeight: CGFloat = 34.0

    weak var delegate: TagCellViewDelegate?
    private var tagItem: Tag?
    private var isSelected = false
    private lazy var backgroundColor: NSColor = {
        if #available(OSX 10.13, *) {
            return NSColor(named: NSColor.Name("tag-selection-background-color"))!
        } else {
            return ConvertHexColor.hexCode(toNSColor: "#e5f9e8")
        }
    }()
    private lazy var hoverBackgroundColor: NSColor = {
        if #available(OSX 10.13, *) {
            return NSColor(named: NSColor.Name("auto-complete-cell-hover"))!
        } else {
            return NSColor(calibratedRed: 177.0/255.0, green: 177.0/255.0, blue: 177.0/255.0, alpha: 0.2)
        }
    }()

    // MARK: OUTLET

    @IBOutlet weak var checkButton: NSButton!
    @IBOutlet weak var backgroundView: NSBox!

    // MARK: Public

    override func awakeFromNib() {
        super.awakeFromNib()
        initTracking()
    }

    override func prepareForReuse() {
        super.prepareForReuse()
        NSCursor.arrow.set()
        isSelected = false
        checkButton.state = .off
    }

    func render(_ tag: Tag, isSelected: Bool) {
        self.tagItem = tag
        self.isSelected = isSelected
        checkButton.title = tag.name
        checkButton.state = isSelected ? .on : .off
        backgroundView.fillColor = isSelected ? backgroundColor : .clear
    }

    @IBAction func checkButtonOnChanged(_ sender: Any) {
        guard let tagItem = tagItem else { return }
        let isSelected = checkButton.state == .on
        delegate?.tagSelectionStateOnChange(with: tagItem, isSelected: isSelected)
    }

    override func mouseExited(with event: NSEvent) {
        super.mouseExited(with: event)
        NSCursor.arrow.set()
        if !isSelected {
            backgroundView.animator().alphaValue = 0.0
        }
    }

    override func mouseEntered(with event: NSEvent) {
        super.mouseEntered(with: event)
        NSCursor.pointingHand.set()
        if !isSelected {
            backgroundView.fillColor = hoverBackgroundColor
            backgroundView.animator().alphaValue = 1.0
        }
    }

    fileprivate func initTracking() {
        let trackingArea = NSTrackingArea(rect: bounds,
                                          options: [.activeAlways, .inVisibleRect, .mouseEnteredAndExited],
                                          owner: self,
                                          userInfo: nil)
        addTrackingArea(trackingArea)
    }
}
