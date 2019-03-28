//
//  AutoCompleteView.swift
//  TogglDesktop
//
//  Created by Nghia Tran on 3/25/19.
//  Copyright © 2019 Alari. All rights reserved.
//

import Cocoa

final class AutoCompleteViewWindow: NSWindow {

    // MARK: Private
    private let topPadding: CGFloat = 5.0
    let autoCompleteView: AutoCompleteView
    override var canBecomeMain: Bool {
        return true
    }
    
    // MARK: Init

    init() {
        self.autoCompleteView = AutoCompleteView.xibView()
        super.init(contentRect: autoCompleteView.bounds,
                   styleMask: .borderless,
                   backing: .buffered,
                   defer: true)
        contentView = autoCompleteView
    }

    func layout(with textField: NSTextField) {
        guard let window = textField.window else { return }
        let size = textField.frame.size

        // Convert
        var location = CGPoint.zero
        let point = textField.superview!.convert(textField.frame.origin, to: nil)
        if #available(OSX 10.12, *) {
            location = window.convertPoint(toScreen: point)
        } else {
            // Fallback on earlier versions
        }
        location.y -= topPadding
        setFrame(CGRect(x: 0, y: 0, width: size.width, height: size.height), display: false)
        setFrameTopLeftPoint(location)
    }

    func cancel() {
        parent?.removeChildWindow(self)
        orderOut(nil)
    }

    func prepare(with dataSource: AutoCompleteViewDataSource) {
        autoCompleteView.prepare(with: dataSource)
    }

    func filter(with text: String) {
        autoCompleteView.filter(with: text)
    }
}

final class AutoCompleteView: NSView {

    // MARK: OUTLET

    @IBOutlet weak var tableView: NSTableView!
    @IBOutlet weak var tableViewHeight: NSLayoutConstraint!
    @IBOutlet weak var createNewProjectBtn: CursorButton!

    // MARK: Variables
    private var dataSource: AutoCompleteViewDataSource!

    // MARK: Public

    override func awakeFromNib() {
        super.awakeFromNib()

        initCommon()
    }

    func prepare(with dataSource: AutoCompleteViewDataSource) {
        self.dataSource = dataSource
        self.dataSource.prepare(self)
    }

    func filter(with text: String) {
        dataSource.filter(with: text)
    }

    func update(height: CGFloat) {
        tableViewHeight.constant = height
    }

    @IBAction func newProjectBtnOnTap(_ sender: Any) {

    }
}

// MARK: Private

extension AutoCompleteView {

    fileprivate func initCommon() {
        createNewProjectBtn.cursor = .pointingHand
    }
}
