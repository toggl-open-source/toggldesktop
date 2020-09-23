//
//  TimerDescriptionFieldHandler.swift
//  TogglDesktop
//
//  Created by Andrew Nester on 23.09.2020.
//  Copyright © 2020 Toggl. All rights reserved.
//

import Foundation

class TimerDescriptionFieldHandler {

    enum State {
        case descriptionUpdate(String)
        case projectDropdownShow
        case projectFilter(String)
        case autocompleteShow
    }

    var state: State = .descriptionUpdate("") {
        didSet {
            onStateChanged?(state)
        }
    }

    var onStateChanged: ((State) -> Void)?

    func textFieldTextDidChange(_ textField: NSTextField) {
        let editor = textField.currentEditor()!

        let text = editor.string
        let cursorLocation = editor.selectedRange.location
        let lastTypedIndex = text.index(text.startIndex, offsetBy: cursorLocation - 1)
        let typedSymbol = text[lastTypedIndex]

        print("<<< typedSymbol = \(typedSymbol)")

        if typedSymbol == "@" {
            var canPresentedDropdown = false
            if cursorLocation == 1 {
                canPresentedDropdown = true
            } else {
                let lastTwoSymbols = text[text.index(before: lastTypedIndex)...lastTypedIndex]
                if String(lastTwoSymbols) == " @" {
                    print(">>>>>> showing project dropdown >>>>>>")
                    canPresentedDropdown = true
                }
            }
            if canPresentedDropdown {
                state = .projectDropdownShow
            }
        } else if case .projectDropdownShow = state {
            let beforeCursor = text[text.startIndex...lastTypedIndex]
            if let shortcutIndex = beforeCursor.lastIndex(of: "@") {
                let searchText = text[text.index(after: shortcutIndex)...lastTypedIndex]
                print(">>>>>>>>> filtering with: \(searchText)")
                state = .projectFilter(String(searchText))
            }
        } else {
            state = .descriptionUpdate(text)
        }

//        if cursorLocation > 0 {
//            var canPresentedDropdown = false
//
//            if cursorLocation == 1 {
//                if typedSymbol == "@" {
//                    print(">>>>>> showing project dropdown >>>>>>")
//                    canPresentedDropdown = true
//                }
//            } else {
//                let lastTwoSymbols = text[text.index(before: lastTypedIndex)...lastTypedIndex]
//                if String(lastTwoSymbols) == " @" {
//                    print(">>>>>> showing project dropdown >>>>>>")
//                    canPresentedDropdown = true
//                }
//            }
//
//            if canPresentedDropdown {
//                state = .projectDropdownShow
//            } else if case .projectDropdownShow = state {
//                let beforeCursor = text[text.startIndex...lastTypedIndex]
//                if let shortcutIndex = beforeCursor.lastIndex(of: "@") {
//                    let searchText = text[text.index(after: shortcutIndex)...lastTypedIndex]
//                    print(">>>>>>>>> filtering with: \(searchText)")
//                    state = .projectFilter(String(searchText))
//                }
//            }
//        }
    }
}
