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

    // MARK: Public

    override func awakeFromNib() {
        super.awakeFromNib()
        initCommon()
    }

    override func prepareForReuse() {
        super.prepareForReuse()
        initCommon()
    }

    func render(with info: DateInfo) {
        titleLbl.stringValue = "\(info.day)"
        if info.isFirstDayOfMonth {
            monthLbl.isHidden = false
            monthLbl.stringValue = info.monthTitle
        } else {
            monthLbl.isHidden = true
        }
    }

    private func initCommon() {
        monthLbl.isHidden = true
    }
}
