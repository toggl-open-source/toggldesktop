//
//  SelectableButton.swift
//  TogglDesktop
//
//  Created by Andrew Nester on 28.07.2020.
//  Copyright © 2020 Toggl. All rights reserved.
//

import Cocoa

class SelectableButton: NSButton {

    @IBInspectable var tintColor: NSColor = Color.lighterGrey.color {
        didSet {
           updateAppearance()
        }
    }

    @IBInspectable var hoverTintColor: NSColor = Color.greyText.color {
        didSet {
           updateAppearance()
        }
    }

    @IBInspectable var activeTintColor: NSColor = Color.green.color {
        didSet {
           updateAppearance()
        }
    }

    @IBInspectable var selectedBackgroundColor: NSColor = Color.green.color.withAlphaComponent(0.3) {
        didSet {
           updateAppearance()
        }
    }

    @IBInspectable var cornerRadius: CGFloat = 0 {
        didSet {
            wantsLayer = true
            layer?.cornerRadius = cornerRadius
        }
    }

    enum State {
        case normal
        case hover
        case active
    }

    var controlState: State = .normal {
        didSet {
            updateAppearance()
        }
    }

    var isSelected: Bool = false {
        didSet {
            updateAppearance()
        }
    }

    /// Set to `true` if button should switch `controlState` to `.active` automatically after click action.
    var isActiveOnClick = true

    override var intrinsicContentSize: NSSize {
        var defaultSize = super.intrinsicContentSize
        // add padding around button for better appearance
        if defaultSize.width > cornerRadius * 2 {
            defaultSize.width += cornerRadius
        }
        return defaultSize
    }

    override var canBecomeKeyView: Bool { true }

    // MARK: - Init

    override init(frame frameRect: NSRect) {
        super.init(frame: frameRect)
        initCommon()
    }

    required init?(coder: NSCoder) {
        super.init(coder: coder)
        initCommon()
    }

    private func initCommon() {
        let trackingArea = NSTrackingArea(rect: NSRect.zero,
                                               options: [.activeInActiveApp, .mouseEnteredAndExited, .assumeInside, .inVisibleRect],
                                               owner: self)
        addTrackingArea(trackingArea)
    }

    override func layout() {
        super.layout()
        updateAppearance()
    }

    private func updateAppearance() {
        image?.isTemplate = true

        if isSelected {
            switch controlState {
            case .normal:
                layer?.backgroundColor = selectedBackgroundColor.withAlphaComponent(0.15).cgColor
            case .hover:
                layer?.backgroundColor = selectedBackgroundColor.withAlphaComponent(0.2).cgColor
            case .active:
                layer?.backgroundColor = selectedBackgroundColor.withAlphaComponent(0.3).cgColor
            }
            image = image?.image(withTintColor: activeTintColor)
        } else {
            switch controlState {
            case .normal:
                image = image?.image(withTintColor: tintColor)
            case .hover:
                image = image?.image(withTintColor: hoverTintColor)
            case .active:
                image = image?.image(withTintColor: activeTintColor)
            }
            layer?.backgroundColor = NSColor.clear.cgColor
        }
    }

    // MARK: - Events

    override func mouseEntered(with event: NSEvent) {
        super.mouseEntered(with: event)
        if controlState == .normal && isEnabled {
            controlState = .hover
        }
    }

    override func mouseExited(with event: NSEvent) {
        super.mouseExited(with: event)
        if controlState == .hover && isEnabled {
            controlState = .normal
        }
    }

    override func mouseDown(with event: NSEvent) {
        guard isEnabled else { return }
        controlState = .active

        // not calling `super` so we can receive `mouseUp` event
    }

    override func mouseUp(with event: NSEvent) {
        super.mouseUp(with: event)

        guard isEnabled else { return }

        let isUpInside = bounds.contains(convert(event.locationInWindow, from: nil))
        if isUpInside {
            controlState = isActiveOnClick ? .active : .normal
            sendAction(action, to: target)
        } else {
            controlState = .normal
        }
    }
}
