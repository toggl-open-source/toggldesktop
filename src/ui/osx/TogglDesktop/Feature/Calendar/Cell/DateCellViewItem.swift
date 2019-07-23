//
//  DateCellViewItem.swift
//  TogglDesktop
//
//  Created by Nghia Tran on 4/19/19.
//  Copyright © 2019 Alari. All rights reserved.
//

import Cocoa

final class DateCellViewItem: NSCollectionViewItem {

    // MARK: OUTLET

    @IBOutlet weak var titleLbl: NSTextField!
    @IBOutlet weak var backgroundBox: NSBox!
    @IBOutlet weak var monthLbl: NSTextField!
    @IBOutlet weak var hoverView: NSBox!

    // MARK: Variables

    private var isHightlight = false
    private lazy var titleColor: NSColor = {
        if #available(OSX 10.13, *) {
            return NSColor(named: NSColor.Name("grey-text-color"))!
        } else {
            return ConvertHexColor.hexCode(toNSColor: "#555555")
        }
    }()

    override var isSelected: Bool {
        didSet {
            guard !isHightlight else { return }
            hoverView.alphaValue = isSelected ? 1.0 : 0.0
        }
    }

    // MARK: Public

    override func awakeFromNib() {
        super.awakeFromNib()
        initCommon()
        initTracking()
    }

    override func prepareForReuse() {
        super.prepareForReuse()
        initCommon()
    }

    func render(with info: DateInfo, highlight: Bool, isCurrentMonth: Bool) {
        isHightlight = highlight
        titleLbl.stringValue = "\(info.day)"
        backgroundBox.isHidden = !highlight

        // Month
        if info.isFirstDayOfMonth {
            monthLbl.isHidden = false
            monthLbl.stringValue = info.monthTitle.uppercased()
        } else {
            monthLbl.isHidden = true
        }

        // Color for title
        if highlight {
            titleLbl.textColor = .white
        } else {
            titleLbl.textColor = isCurrentMonth ? NSColor.labelColor : titleColor
        }
    }

    private func initCommon() {
        monthLbl.isHidden = true
        backgroundBox.isHidden = true
        hoverView.alphaValue = 0.0
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

    private func initTracking() {
        let trackingArea = NSTrackingArea(rect: view.bounds,
                                          options: [.activeInKeyWindow, .mouseEnteredAndExited],
                                          owner: self,
                                          userInfo: nil)
        view.addTrackingArea(trackingArea)
    }
}
