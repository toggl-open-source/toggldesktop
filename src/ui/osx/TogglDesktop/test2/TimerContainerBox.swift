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
        case active
    }

    // MARK: Variables
    private var state = State.inactive

    private var activeBorderColor: NSColor {
        return NSColor.clear
    }

    private var inactiveBorderColor: NSColor {
        return NSColor.clear
    }

    private var activeFillColor: NSColor {
        if #available(OSX 10.13, *) {
            return NSColor(named: NSColor.Name("upload-background-color"))!
        } else {
            return NSColor.white
        }
    }

    private var inactiveFillColor: NSColor {
        if #available(OSX 10.13, *) {
            return NSColor(named: NSColor.Name("preference-box-background-color"))!
        } else {
            return NSColor(deviceRed: 177.0/255.0, green: 177.0/255.0, blue: 177.0/255.0, alpha: 0.07)
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

        switch state {
        case .active:
            borderColor = activeBorderColor
            fillColor = activeFillColor
        case .inactive:
            borderColor = inactiveBorderColor
            fillColor = inactiveFillColor
        }
    }
}


