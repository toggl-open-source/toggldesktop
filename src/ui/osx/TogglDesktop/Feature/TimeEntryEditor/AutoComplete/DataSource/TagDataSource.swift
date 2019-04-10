//
//  TagDataSource.swift
//  TogglDesktop
//
//  Created by Nghia Tran on 4/5/19.
//  Copyright © 2019 Alari. All rights reserved.
//

import Foundation

protocol TagDataSourceDelegate: class {

    func tagSelectionChanged(with selectedTags: [Tag])
}

final class TagDataSource: AutoCompleteViewDataSource {

    private struct Constants {

        static let CellID = NSUserInterfaceItemIdentifier("TagCellView")
        static let CellNibName = NSNib.Name("TagCellView")
        static let ClientEmptyCellID = NSUserInterfaceItemIdentifier("NoClientCellView")
        static let ClientEmptyIDNibName = NSNib.Name("NoClientCellView")
    }

    // MARK: Variables

    weak var tagDelegte: TagDataSourceDelegate?
    private(set) var selectedTags: [Tag] = []

    // MARK: Override

    override func setup(with textField: AutoCompleteTextField) {
        super.setup(with: textField)
        tableView.allowsEmptySelection = true
        autoCompleteView.setCreateButtonSectionHidden(true)
    }

    override func registerCustomeCells() {
        tableView.register(NSNib(nibNamed: Constants.CellNibName, bundle: nil),
                           forIdentifier: Constants.CellID)
        tableView.register(NSNib(nibNamed: Constants.ClientEmptyIDNibName, bundle: nil),
                           forIdentifier: Constants.ClientEmptyCellID)
    }

    override func filter(with text: String) {

        // show all
        if text.isEmpty {
            render(with: TagStorage.shared.tags)
            return
        }

        // Filter
        let filterItems = TagStorage.shared.filter(with: text)
        render(with: filterItems)
    }

    override func render(with items: [Any]) {
        super.render(with: items)

        // Hide create if it's has content
        if let first = items.first as? Tag, first.isEmptyTag {
            autoCompleteView.setCreateButtonSectionHidden(false)
            autoCompleteView.updateTitleForCreateButton(with: "Create new tag \"\(textField.stringValue)\"")
        } else {
            autoCompleteView.setCreateButtonSectionHidden(true)
        }
    }

    func updateSelectedTags(_ tags: [Tag]) {
        self.selectedTags = tags
    }

    // MARK: Public

    override func tableView(_ tableView: NSTableView, viewFor tableColumn: NSTableColumn?, row: Int) -> NSView? {
        let item = items[row] as! Tag
        if item.isEmptyTag {
            return tableView.makeView(withIdentifier: Constants.ClientEmptyCellID, owner: self) as! NoClientCellView
        }
        let view = tableView.makeView(withIdentifier: Constants.CellID, owner: self) as! TagCellView
        let isSelected = selectedTags.contains(where: { $0.name == item.name })
        view.delegate = self
        view.render(item, isSelected: isSelected)
        return view
    }

    override func tableView(_ tableView: NSTableView, heightOfRow row: Int) -> CGFloat {
        let item = items[row] as! Tag
        if item.isEmptyTag {
            return NoClientCellView.cellHeight
        }
        return TagCellView.cellHeight
    }

    override func tableView(_ tableView: NSTableView, shouldSelectRow row: Int) -> Bool {
        return false
    }
}

// MARK: TagCellViewDelegate

extension TagDataSource: TagCellViewDelegate {

    func tagSelectionStateOnChange(with tag: Tag, isSelected: Bool) {
        if isSelected {
            guard !selectedTags.contains(where: { $0.name == tag.name }) else { return }
            selectedTags.append(tag)
            tagDelegte?.tagSelectionChanged(with: selectedTags)
        } else {
            guard let index = selectedTags.firstIndex(where: { $0.name == tag.name }) else { return }
            selectedTags.remove(at: index)
            tagDelegte?.tagSelectionChanged(with: selectedTags)
        }
    }
}
