//
//  TimerContainerBox.swift
//  TogglDesktop
//
//  Created by Nghia Tran on 3/6/19.
//  Copyright © 2019 Alari. All rights reserved.
//

import Cocoa

final class TimerContainerBox: NSBox, TextFieldResponderDelegate {

    enum State {
        case inactive
        case hover
        case active

        var fillColor: NSColor {
            switch self {
            case .active, .inactive:
                return .clear
            case .hover:
                return .togglTextFieldHoverBackground
            }
        }

        var borderColor: NSColor {
            switch self {
            case .inactive, .hover:
                return .clear
            case .active:
                return .togglLighterGrey
            }
        }

        var borderWidth: CGFloat {
            return 1
        }
    }

    private var state = State.inactive

    // MARK: Init

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
        renderLayout(for: state)
    }

    // MARK: Mouse Events

    override func mouseEntered(with event: NSEvent) {
        super.mouseEntered(with: event)

        if state == .inactive {
            renderLayout(for: .hover)
        }
    }

    override func mouseExited(with event: NSEvent) {
        super.mouseExited(with: event)

        if state == .hover {
            renderLayout(for: .inactive)
        }
    }

    // MARK: TextFieldResponderDelegate

    func didBecomeFirstResponder(_ sender: NSTextField) {
        renderLayout(for: .active)
    }

    func didResignFirstResponder(_ sender: NSTextField) {
        renderLayout(for: .inactive)
    }

    private func renderLayout(for state: State) {
        self.state = state
        borderColor = state.borderColor
        borderWidth = state.borderWidth
        fillColor = state.fillColor
    }
}
