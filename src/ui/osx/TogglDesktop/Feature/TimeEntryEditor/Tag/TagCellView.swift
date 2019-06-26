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

    // MARK: Variables
    
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
    private var greenColor: NSColor {
        if #available(OSX 10.13, *) {
            return NSColor(named: NSColor.Name("green-color"))!
        } else {
            return ConvertHexColor.hexCode(toNSColor: "#28cd41")
        }
    }
    private lazy var attributeDict: [NSAttributedString.Key: Any] = {
        let font = checkButton.font ?? NSFont.systemFont(ofSize: 14.0)
        let color = greenColor
        return [NSAttributedString.Key.foregroundColor: color,
                NSAttributedString.Key.font: font]
    }()

    // MARK: OUTLET

    @IBOutlet weak var checkButton: NSButton!
    @IBOutlet weak var backgroundView: NSBox!
    @IBOutlet weak var hoverView: NSBox!
    
    // MARK: Public

    override func awakeFromNib() {
        super.awakeFromNib()
        initCommon()
        initTracking()
    }

    override func prepareForReuse() {
        super.prepareForReuse()
        NSCursor.arrow.set()
        isSelected = false
        checkButton.state = .off
        hoverView.alphaValue = 0.0
    }

    func render(_ tag: Tag, isSelected: Bool) {
        self.tagItem = tag
        self.isSelected = isSelected
        checkButton.state = isSelected ? .on : .off
        backgroundView.fillColor = isSelected ? backgroundColor : .clear
        if isSelected {
            checkButton.attributedTitle = NSAttributedString(string: tag.name, attributes: attributeDict)
        } else {
            checkButton.title = tag.name
        }
        checkButton.toolTip = tag.name
    }

    func selectCheckBox() {
        checkButton.performClick(checkButton)
    }

    @IBAction func checkButtonOnChanged(_ sender: Any) {
        guard let tagItem = tagItem else { return }
        let isSelected = checkButton.state == .on
        delegate?.tagSelectionStateOnChange(with: tagItem, isSelected: isSelected)
    }

    override func mouseExited(with event: NSEvent) {
        super.mouseExited(with: event)
        NSCursor.arrow.set()
        hoverView.animator().alphaValue = 0.0
    }

    override func mouseEntered(with event: NSEvent) {
        super.mouseEntered(with: event)
        NSCursor.pointingHand.set()
        hoverView.animator().alphaValue = 1.0
    }

    private func initCommon() {
        hoverView.alphaValue = 0.0
    }

    private func initTracking() {
        let trackingArea = NSTrackingArea(rect: bounds,
                                          options: [.activeInKeyWindow, .inVisibleRect, .mouseEnteredAndExited],
                                          owner: self,
                                          userInfo: nil)
        addTrackingArea(trackingArea)
    }
}
