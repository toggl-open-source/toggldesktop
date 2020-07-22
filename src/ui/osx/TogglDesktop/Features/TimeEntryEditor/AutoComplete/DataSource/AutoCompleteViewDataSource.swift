//
//  AutoCompleteViewDataSource.swift
//  TogglDesktop
//
//  Created by Nghia Tran on 3/25/19.
//  Copyright © 2019 Alari. All rights reserved.
//

import Foundation

protocol AutoCompleteViewDataSourceDelegate: class {

    func autoCompleteSelectionDidChange(sender: AutoCompleteViewDataSource, item: Any)
}

class AutoCompleteViewDataSource: NSObject {

    // MARK: Variables
    private let maxHeight: CGFloat = 600.0
    private(set) var items: [Any] = []
    private(set) var autoCompleteView: AutoCompleteView!
    private(set) var textField: AutoCompleteTextField!
    weak var delegate: AutoCompleteViewDataSourceDelegate?
    var count: Int {
        return items.count
    }
    var tableView: NSTableView {
        return autoCompleteView.tableView
    }

    // MARK: Init

    init(items: [Any], updateNotificationName: Notification.Name) {
        super.init()
        self.items = items
        NotificationCenter.default.addObserver(self,
                                               selector: #selector(self.receiveDataSourceNotifiation(_:)),
                                               name: updateNotificationName,
                                               object: nil)
    }

    deinit {
        NotificationCenter.default.removeObserver(self)
    }

    // MARK: Public

    func setup(with textField: AutoCompleteTextField) {
        self.textField = textField
        self.autoCompleteView = textField.autoCompleteView
        self.autoCompleteView.prepare(with: self)
        registerCustomeCells()
        tableView.delegate = self
        tableView.dataSource = self
    }

    func registerCustomeCells() {
        // Should override
    }

    func render(with items: [Any]) {
        self.items = items
        tableView.reloadData()
        sizeToFit()
    }

    func filter(with text: String) {
        // Should override
    }

    @objc func receiveDataSourceNotifiation(_ noti: Notification) {
        guard let items = noti.object as? [Any] else {
            return
        }
        render(with: items)

        // If there is new data during searching on auto-complete
        // We should filter gain
        if textField.state == .expand && !textField.stringValue.isEmpty {
            filter(with: textField.stringValue)
        }
    }

    func selectSelectedRow() {
        selectRow(at: tableView.selectedRow)
    }

    func selectRow(at index: Int) {
        guard let item = items[safe: index] else { return }
        delegate?.autoCompleteSelectionDidChange(sender: self, item: item)
    }

    private func sizeToFit() {
        if items.isEmpty {
            autoCompleteView.update(height: 0.0)
            return
        }

        // Get total height of all cells
        let totalHeight = items.enumerated().reduce(into: 0.0) { (height, item) in
            return height += tableView(tableView, heightOfRow: item.offset)
        }

        // Get suitable height
        // 0 <= height <= maxHeight
        var suitableHeight = CGFloat.minimum(CGFloat.maximum(0, totalHeight), maxHeight)

        // Padding
        if let scrollView = tableView.enclosingScrollView {
            suitableHeight += scrollView.contentInsets.bottom + scrollView.contentInsets.top
        }

        autoCompleteView.update(height: suitableHeight)
    }

    func keyboardDidEnter() {
        // Override
    }
}

extension AutoCompleteViewDataSource: NSTableViewDataSource, NSTableViewDelegate {

    func numberOfRows(in tableView: NSTableView) -> Int {
        return items.count
    }

    func tableView(_ tableView: NSTableView, viewFor tableColumn: NSTableColumn?, row: Int) -> NSView? {
        return nil
    }

    func tableView(_ tableView: NSTableView, heightOfRow row: Int) -> CGFloat {
        return 44.0
    }

    func tableView(_ tableView: NSTableView, shouldSelectRow row: Int) -> Bool {
        return true
    }

    func tableView(_ tableView: NSTableView, rowViewForRow row: Int) -> NSTableRowView? {
        return AutoCompleteRowView()
    }
}
