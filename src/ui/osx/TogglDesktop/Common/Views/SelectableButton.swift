//
//  SelectableButton.swift
//  TogglDesktop
//
//  Created by Andrew Nester on 28.07.2020.
//  Copyright © 2020 Toggl. All rights reserved.
//

import Cocoa

class SelectableButton: NSButton {

    @IBInspectable var tintColor: NSColor = .togglLighterGrey
    @IBInspectable var hoverTintColor: NSColor = .togglGreyText
    @IBInspectable var activeTintColor: NSColor = .togglGreen
    @IBInspectable var selectedBackgroundColor: NSColor = NSColor.togglGreen.withAlphaComponent(0.3)

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

    override var intrinsicContentSize: NSSize {
        var defaultSize = super.intrinsicContentSize
        // add padding around button for better appearance
        if defaultSize.width > cornerRadius * 2 {
            defaultSize.width += cornerRadius
        }
        return defaultSize
    }

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

        updateAppearance()
    }

    private func updateAppearance() {
        image?.isTemplate = true

        if isSelected {
            switch controlState {
            case .normal:
                layer?.backgroundColor = selectedBackgroundColor.withAlphaComponent(0.3).cgColor
            case .hover:
                layer?.backgroundColor = selectedBackgroundColor.withAlphaComponent(0.6).cgColor
            case .active:
                layer?.backgroundColor = selectedBackgroundColor.cgColor
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
        // leaving empty so we can receive `mouseUp` event
    }

    override func mouseUp(with event: NSEvent) {
        super.mouseUp(with: event)
        let isUpInside = bounds.contains(convert(event.locationInWindow, from: nil))
        if isUpInside {
            controlState = .active
            sendAction(action, to: target)
        }
    }
}
