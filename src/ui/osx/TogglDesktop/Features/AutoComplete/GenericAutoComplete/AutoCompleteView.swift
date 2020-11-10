//
//  AutoCompleteView.swift
//  TogglDesktop
//
//  Created by Nghia Tran on 3/25/19.
//  Copyright © 2019 Alari. All rights reserved.
//

import Cocoa

protocol AutoCompleteViewDelegate: class {

    func didTapOnCreateButton()
    func shouldClose()
}

final class AutoCompleteView: NSView {

    private struct Constants {
        static let CreateButtonHeight: CGFloat = 40
    }

    // MARK: OUTLET

    @IBOutlet weak var tableView: KeyboardTableView! {
        didSet {
            tableView.refusesFirstResponder = true
        }
    }

    @IBOutlet weak var tableViewHeight: NSLayoutConstraint!
    @IBOutlet weak var tableScrollView: NSScrollView!
    @IBOutlet weak var createNewItemBtn: CursorButton!
    @IBOutlet weak var createNewItemContainerView: NSBox!
    @IBOutlet weak var horizontalLine: NSBox!
    @IBOutlet weak var stackView: NSStackView!
    @IBOutlet private weak var placeholderBox: NSView!
    @IBOutlet weak var placeholderBoxContainerView: NSView!
    @IBOutlet weak var defaultTextField: ResponderTextField!

    // MARK: Variables

    weak var delegate: AutoCompleteViewDelegate?
    private weak var dataSource: AutoCompleteViewDataSource?

    var isSearchFieldHidden: Bool = true {
        didSet {
            placeholderBox.isHidden = isSearchFieldHidden
        }
    }

    // MARK: Public

    override func awakeFromNib() {
        super.awakeFromNib()

        initCommon()
    }

    func prepare(with dataSource: AutoCompleteViewDataSource) {
        self.dataSource = dataSource
    }

    func filter(with text: String) {
        dataSource?.filter(with: text)
    }

    func update(height: CGFloat) {
        guard let window = self.window,
            let screen = NSScreen.main else { return }

        var height = height
        let screenFrame = window.convertToScreen(frame)

        // Get the top left
        // As screenFrame.origin.y is a bottom-left
        var topLeftY = screenFrame.origin.y + screenFrame.size.height

        // Add the offset in external monitor
        // Because there is only one origin coordinate system, so in some external screen, the bottom-left origin could be negative value
        // https://github.com/toggl-open-source/toggldesktop/issues/3524
        topLeftY -= screen.frame.origin.y

        // Exclude the system bar height
        let dockBarHeight = abs(screen.frame.height - screen.visibleFrame.height)
        var offset: CGFloat = createNewItemContainerView.isHidden ? 0 : Constants.CreateButtonHeight
        offset += dockBarHeight

        // Reduce the size if the height is greater than screen
        if (height + offset) > topLeftY {
            height = topLeftY - offset
        }

        // Make sure >= 0
        height = CGFloat.maximum(height, 0)

        // Hide or show the ScrollView to prevent UI Constraints ambiguous
        tableScrollView.isHidden = height == 0
        if height > 0 {
            tableViewHeight.constant = height
        }

        // Hack to workaround the blur text
        fixBlurTextOnNonRetinaScreen()
    }

    func setCreateButtonSectionHidden(_ isHidden: Bool) {
        horizontalLine.isHidden = isHidden
        createNewItemContainerView.isHidden = isHidden
    }

    func updateTitleForCreateButton(with text: String) {
        createNewItemBtn.title = text
    }

    func clean() {
        defaultTextField.stringValue = ""
        filter(with: "")
    }

    @IBAction func createNewItemOnTap(_ sender: Any) {
        delegate?.didTapOnCreateButton()
    }
}

// MARK: - NSTextFieldDelegate for defaultTextField

extension AutoCompleteView: NSTextFieldDelegate {
    func controlTextDidChange(_ obj: Notification) {
        if let textField = obj.object as? NSTextField, textField == defaultTextField {
            filter(with: textField.stringValue)
        }
    }

    func control(_ control: NSControl, textView: NSTextView, doCommandBy commandSelector: Selector) -> Bool {
        guard let currentEvent = NSApp.currentEvent, textView == defaultTextField.currentEditor() else {
            return false
        }

        if commandSelector == #selector(NSResponder.cancelOperation(_:)) {
            delegate?.shouldClose()
            return true
        }

        if commandSelector == #selector(NSResponder.moveDown(_:))
            || commandSelector == #selector(NSResponder.moveUp(_:))
            || commandSelector == #selector(NSResponder.insertTab(_:))
            || commandSelector == #selector(NSResponder.insertNewline(_:)) {
            return tableView.handleKeyboardEvent(currentEvent)
        }

        return false
    }
}

// MARK: Private

extension AutoCompleteView {

    fileprivate func initCommon() {
        stackView.wantsLayer = true
        stackView.layer?.masksToBounds = true
        stackView.layer?.cornerRadius = 8
        placeholderBox.isHidden = isSearchFieldHidden
        createNewItemBtn.cursor = .pointingHand
        tableView.keyDidDownOnPress = {[weak self] key -> Bool in
            guard let strongSelf = self else { return false }
            switch key {
            case .enter,
                 .returnKey:
                strongSelf.dataSource?.selectSelectedRow()
                strongSelf.dataSource?.keyboardDidEnter()
                return true

            case .tab:
                // Don't focus to create button if it's hidden
                if strongSelf.createNewItemContainerView.isHidden {
                    strongSelf.delegate?.shouldClose()
                    return false
                }

                // Only focus to create button if the view is expaned
                if strongSelf.window?.isVisible == true {
                    strongSelf.window?.makeKeyAndOrderFront(nil)
                    strongSelf.window?.makeFirstResponder(strongSelf.createNewItemBtn)
                    return true
                }

            case .downArrow, .upArrow:
                // handled by table view automatically
                return true
            }

            return false
        }
        tableView.clickedOnRow = {[weak self] clickedRow in
            self?.dataSource?.selectRow(at: clickedRow)
        }

        createNewItemBtn.didPressKey = { key, _ in
            if key == .tab {
                self.delegate?.shouldClose()
            }
        }
    }

    private func fixBlurTextOnNonRetinaScreen() {
        // Ref: https://github.com/toggl-open-source/toggldesktop/issues/3313
        if #available(OSX 10.12, *) {
            if let screenScale = NSScreen.main?.backingScaleFactor, screenScale == 1.0 {
                createNewItemBtn.image = nil
                createNewItemBtn.imageHugsTitle = false
            } else {
                createNewItemBtn.image = NSImage(named: "add-icon")
                createNewItemBtn.imageHugsTitle = true
            }
        }
    }
}
