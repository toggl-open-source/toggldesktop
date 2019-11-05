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
            renderTagsAndCreateBtn(TagStorage.shared.tags, with: text)
            return
        }

        // Filter
        let filterItems = TagStorage.shared.filter(with: text)
        renderTagsAndCreateBtn(filterItems, with: text)
    }

    func updateSelectedTags(_ tags: [Tag]) {
        self.selectedTags = tags
    }

    private func renderTagsAndCreateBtn(_ tags: [Tag], with text: String) {
        render(with: tags)

        // Determine if it should present the Create btn
        var shouldShowCreateBtn = false
        if let first = tags.first, first.isEmptyTag {
            shouldShowCreateBtn = true
        } else if !text.isEmpty && !tags.contains(where: { $0.name == text }) {
            shouldShowCreateBtn = true
        }

        if shouldShowCreateBtn {
            autoCompleteView.setCreateButtonSectionHidden(false)
            autoCompleteView.updateTitleForCreateButton(with: "Create new tag \"\(text)\"")
        } else {
            autoCompleteView.setCreateButtonSectionHidden(true)
        }
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
        let item = items[row] as! Tag
        if item.isEmptyTag {
            return false
        }
        return true
    }

    override func keyboardDidEnter() {
        guard tableView.selectedRow >= 0 else { return }
        guard let tagView = tableView.view(atColumn: 0, row: tableView.selectedRow, makeIfNecessary: false) as? TagCellView else { return }
        tagView.selectCheckBox()
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
