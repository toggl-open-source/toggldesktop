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

    private var topPadding: CGFloat = 5.0
    var isSeparateWindow = true
    override var canBecomeMain: Bool {
        return true
    }
    override var canBecomeKey: Bool {
        return true
    }

    // MARK: Init

    init(view: NSView) {
        super.init(contentRect: view.bounds,
                   styleMask: .borderless,
                   backing: .buffered,
                   defer: true)
        contentView = view
        hasShadow = true
        backgroundColor = NSColor.clear
        isOpaque = false
        setContentBorderThickness(0, for: NSRectEdge(rawValue: 0)!)
    }

    func layoutFrame(with textField: NSTextField, origin: CGPoint, size: CGSize) {
        guard let window = textField.window else { return }
        var height = size.height

        // Convert
        var location = CGPoint.zero
        let point = textField.superview!.convert(origin, to: nil)
        if #available(OSX 10.12, *) {
            location = window.convertPoint(toScreen: point)
        } else {
            location = window.convertToScreen(NSRect(origin: point, size: size)).origin
        }

        if isSeparateWindow {
            location.y -= topPadding
        } else {
            location.y -= -30.0
            height += 30.0
        }

        setFrame(CGRect(x: 0, y: 0, width: size.width, height: height), display: false)
        setFrameTopLeftPoint(location)
    }

    func cancel() {
        parent?.removeChildWindow(self)
        orderOut(nil)
    }
}

protocol AutoCompleteViewDelegate: class {

    func didTapOnCreateButton()
}

final class AutoCompleteView: NSView {

    private struct Constants {
        static let CreateButtonHeight: CGFloat = 40
    }

    // MARK: OUTLET

    @IBOutlet weak var tableView: KeyboardTableView!
    @IBOutlet weak var tableViewHeight: NSLayoutConstraint!
    @IBOutlet weak var createNewItemBtn: CursorButton!
    @IBOutlet weak var createNewItemContainerView: NSBox!
    @IBOutlet weak var horizontalLine: NSBox!
    @IBOutlet weak var stackView: NSStackView!
    @IBOutlet weak var placeholderBox: NSView!
    @IBOutlet weak var placeholderBoxContainerView: NSView!

    // MARK: Variables

    weak var delegate: AutoCompleteViewDelegate?
    private weak var dataSource: AutoCompleteViewDataSource?

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
        var height = height
        let screenFrame = self.window!.convertToScreen(frame)
        let topLeftY = screenFrame.origin.y + screenFrame.size.height
        var offset: CGFloat = createNewItemContainerView.isHidden ? 0 : Constants.CreateButtonHeight
        offset -= 2 // No collision with screen

        // Reduce the size if the height is greater than screen
        if (height + offset) > topLeftY {
            height = topLeftY - offset
        }

        // Overriden
        tableViewHeight.constant = height
    }

    func setCreateButtonSectionHidden(_ isHidden: Bool) {
        horizontalLine.isHidden = isHidden
        createNewItemContainerView.isHidden = isHidden
    }

    func updateTitleForCreateButton(with text: String) {
        createNewItemBtn.title = text
    }

    @IBAction func createNewItemOnTap(_ sender: Any) {
        delegate?.didTapOnCreateButton()
    }
}

// MARK: Private

extension AutoCompleteView {

    fileprivate func initCommon() {
        stackView.wantsLayer = true
        stackView.layer?.masksToBounds = true
        stackView.layer?.cornerRadius = 8
        placeholderBox.isHidden = true
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
                    return false
                }

                // Only focus to create button if the view is expaned
                if let textField = strongSelf.dataSource?.textField, textField.state == .expand {
                    strongSelf.window?.makeKeyAndOrderFront(nil)
                    strongSelf.window?.makeFirstResponder(strongSelf.createNewItemBtn)
                    return true
                }
            default:
                return false
            }
            return false
        }
        tableView.clickedOnRow = {[weak self] clickedRow in
            self?.dataSource?.selectRow(at: clickedRow)
        }
    }
}
