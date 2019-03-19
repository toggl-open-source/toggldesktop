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

    // MARK: Public

    override func awakeFromNib() {
        super.awakeFromNib()
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

    @IBAction func openSectionOnTap(_ sender: Any) {
        delegate?.togglSection(at: section)
    }
}

// MARK: Private

extension TimeHeaderView {

    fileprivate func initDefaultValue() {
        arrowBtn.isHidden = true
    }
}
