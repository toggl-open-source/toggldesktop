//
//  TimerDescriptionFieldHandlerTests.swift
//  TogglDesktop
//
//  Created by Andrew Nester on 02.10.2020.
//  Copyright © 2020 Toggl. All rights reserved.
//

import XCTest
@testable import Toggl_Track

class TimerDescriptionFieldHandlerTests: XCTestCase {

    typealias Action = TimerDescriptionFieldHandler.Action
    typealias State = TimerDescriptionFieldHandler.State

    var textField: AutoCompleteInput!
    var handler: TimerDescriptionFieldHandler!
    var window: NSWindow!

    var controlTextDidChangeNotification: Notification!
    var controlTextDidEndEditingNotification: Notification!

    override func setUpWithError() throws {
        textField = AutoCompleteInput()

        // text field needs to be attached to window and be a first responder
        // to have `currentEditor() != nil`, that is needed for handler
        window = NSWindow()
        window.contentView?.addSubview(textField)
        window.makeKey()
        window.makeFirstResponder(textField)

        handler = TimerDescriptionFieldHandler(textField: textField, enableProjectShortcut: true)
        textField.delegate = handler

        controlTextDidChangeNotification = Notification(name: NSControl.textDidChangeNotification, object: textField, userInfo: nil)
        controlTextDidEndEditingNotification = Notification(name: NSControl.textDidEndEditingNotification, object: textField, userInfo: nil)
    }

    override func tearDownWithError() throws {
    }

    // MARK: - controlTextDidEndEditing

    func testControlTextDidEndEditingRequestsAction() throws {
        handler.onPerformAction = { action in
            XCTAssertEqual(action, Action.endEditing)
            return false
        }

        handler.controlTextDidEndEditing(controlTextDidEndEditingNotification)
    }

    func testControlTextDidEndEditingSetStateToDefault() {
        setStateAutocompleteFilter(filterText: "any")
        XCTAssertNotEqual(handler.state, .descriptionUpdate)

        handler.controlTextDidEndEditing(controlTextDidEndEditingNotification)

        XCTAssertEqual(handler.state, .descriptionUpdate)
    }

    // MARK: - controlTextDidChange

    func testControlTextDidChangeDoNotCrashIfNoEditor() {
        window.makeFirstResponder(nil)
        handler.controlTextDidChange(controlTextDidChangeNotification)
    }

    // MARK: - controlTextDidChange - Project shortcut

    func testControlTextDidChangeDetectProjectShortcutAtStart() {
        textField.stringValue = "@"
        moveCursorToEnd()
        handler.controlTextDidChange(controlTextDidChangeNotification)
        XCTAssertEqual(handler.state, State.projectFilter(""))

        textField.stringValue = "@proj"
        moveCursorToEnd()
        handler.controlTextDidChange(controlTextDidChangeNotification)
        XCTAssertEqual(handler.state, State.projectFilter("proj"))

        textField.stringValue = "@project with space"
        moveCursorToEnd()
        handler.controlTextDidChange(controlTextDidChangeNotification)
        XCTAssertEqual(handler.state, State.projectFilter("project with space"))
    }

    func testControlTextDidChangeDetectProjectShortcutAtEnd() {
        textField.stringValue = "some @"
        moveCursorToEnd()
        handler.controlTextDidChange(controlTextDidChangeNotification)
        XCTAssertEqual(handler.state, State.projectFilter(""))

        textField.stringValue = "some @proj"
        moveCursorToEnd()
        handler.controlTextDidChange(controlTextDidChangeNotification)
        XCTAssertEqual(handler.state, State.projectFilter("proj"))

        textField.stringValue = "some @pr with space and long"
        moveCursorToEnd()
        handler.controlTextDidChange(controlTextDidChangeNotification)
        XCTAssertEqual(handler.state, State.projectFilter("pr with space and long"))
    }

    func testControlTextDidChangeNeedSpaceBeforeShortcut() {
        textField.stringValue = "some@withoutspace"
        moveCursorToEnd()
        handler.controlTextDidChange(controlTextDidChangeNotification)
        XCTAssertEqual(handler.state, State.autocompleteFilter("some@withoutspace"))
    }

    func testControlTextDidChangeDetectProjectShortcutInMiddle() {
        textField.stringValue = "some @|text"
        moveCursor(to: 6)
        handler.controlTextDidChange(controlTextDidChangeNotification)
        XCTAssertEqual(handler.state, State.projectFilter(""))

        textField.stringValue = "some @proj| text"
        moveCursor(to: 10)
        handler.controlTextDidChange(controlTextDidChangeNotification)
        XCTAssertEqual(handler.state, State.projectFilter("proj"))

        textField.stringValue = "some @|text"
        moveCursor(to: 6)
        handler.controlTextDidChange(controlTextDidChangeNotification)
        XCTAssertEqual(handler.state, State.projectFilter(""))

        textField.stringValue = "some @proj|text"
        moveCursor(to: 10)
        handler.controlTextDidChange(controlTextDidChangeNotification)
        XCTAssertEqual(handler.state, State.projectFilter("proj"))
    }

    func testControlTextDidChangeHandleTwoSameShortcuts() {
        textField.stringValue = "@@"
        moveCursorToEnd()
        handler.controlTextDidChange(controlTextDidChangeNotification)
        XCTAssertEqual(handler.state, State.projectFilter("@"))

        textField.stringValue = "@@proj"
        moveCursorToEnd()
        handler.controlTextDidChange(controlTextDidChangeNotification)
        XCTAssertEqual(handler.state, State.projectFilter("@proj"))

        textField.stringValue = "@ @"
        moveCursorToEnd()
        handler.controlTextDidChange(controlTextDidChangeNotification)
        XCTAssertEqual(handler.state, State.projectFilter(""))

        textField.stringValue = "@ @second_proj"
        moveCursorToEnd()
        handler.controlTextDidChange(controlTextDidChangeNotification)
        XCTAssertEqual(handler.state, State.projectFilter("second_proj"))
    }

    // MARK: - controlTextDidChange - Autocomplete

    func testControlTextDidChangeDetectAutocompleteFilter() {
        textField.stringValue = ""
        handler.controlTextDidChange(controlTextDidChangeNotification)
        XCTAssertEqual(handler.state, State.autocompleteFilter(""))

        textField.stringValue = "some"
        moveCursorToEnd()
        handler.controlTextDidChange(controlTextDidChangeNotification)
        XCTAssertEqual(handler.state, State.autocompleteFilter("some"))

        textField.stringValue = "some query with space"
        moveCursorToEnd()
        handler.controlTextDidChange(controlTextDidChangeNotification)
        XCTAssertEqual(handler.state, State.autocompleteFilter("some query with space"))
    }

    func testControlTextDidChangeAutocompleteNotDependOnCursorLocation() {
        textField.stringValue = "some query"
        moveCursor(to: 4)
        handler.controlTextDidChange(controlTextDidChangeNotification)
        XCTAssertEqual(handler.state, State.autocompleteFilter("some query"))
    }

    func testControlTextDidChangeWithDisabledProjectShortcutFlag() {
        handler.isProjectShortcutEnabled = false

        textField.stringValue = "@"
        moveCursorToEnd()
        handler.controlTextDidChange(controlTextDidChangeNotification)
        XCTAssertEqual(handler.state, State.autocompleteFilter("@"))

        textField.stringValue = "some @proj"
        moveCursorToEnd()
        handler.controlTextDidChange(controlTextDidChangeNotification)
        XCTAssertEqual(handler.state, State.autocompleteFilter("some @proj"))
    }

    // MARK: - Other

    func testDidCloseProjectDropdownSetStateToDefaultFromProjectFilter() {
        setStateProjectFilter(filterText: "some")

        handler.didCloseProjectDropdown()

        XCTAssertEqual(handler.state, .descriptionUpdate)
    }

    func testDidCloseProjectDropdownDoesNothingWhenWrongState() {
        setStateAutocompleteFilter(filterText: "some")

        handler.didCloseProjectDropdown()

        XCTAssertEqual(handler.state, .autocompleteFilter("some"))
    }

    func testDidSelectProjectSetStateToDefaultFromProjectFilter() {
        setStateProjectFilter(filterText: "some")

        handler.didSelectProject()

        XCTAssertEqual(handler.state, .descriptionUpdate)
    }

    func testDidSelectProjectDoesNothingWhenWrongState() {
        setStateAutocompleteFilter(filterText: "some")

        handler.didSelectProject()

        XCTAssertEqual(handler.state, .autocompleteFilter("some"))
    }

    func testDidSelectProjectRemovesShortcutQueryFromEnd() {
        textField.stringValue = "some @project query"
        moveCursorToEnd()
        handler.controlTextDidChange(controlTextDidChangeNotification)

        handler.didSelectProject()

        XCTAssertEqual(textField.stringValue, "some ")
        XCTAssertEqual(handler.state, State.descriptionUpdate)
    }

    func testDidSelectProjectRemovesShortcutQueryFromStart() {
        textField.stringValue = "@project query"
        moveCursorToEnd()
        handler.controlTextDidChange(controlTextDidChangeNotification)

        handler.didSelectProject()

        XCTAssertEqual(textField.stringValue, "")
        XCTAssertEqual(handler.state, State.descriptionUpdate)
    }

    func testDidSelectProjectRemovesShortcutQueryFromMiddle() {
        textField.stringValue = "some @project| other text"
        moveCursor(to: 13)
        handler.controlTextDidChange(controlTextDidChangeNotification)

        handler.didSelectProject()

        XCTAssertEqual(textField.stringValue, "some | other text")
        XCTAssertEqual(handler.state, State.descriptionUpdate)
    }

    // MARK: - Helpers

    private func setStateAutocompleteFilter(filterText: String) {
        textField.stringValue = filterText
        handler.controlTextDidChange(controlTextDidChangeNotification)
    }

    private func setStateProjectFilter(filterText: String) {
        textField.stringValue = "@\(filterText)"
        moveCursorToEnd()
        handler.controlTextDidChange(controlTextDidChangeNotification)
    }

    private func moveCursorToEnd() {
        textField.currentEditor()?.selectedRange = NSRange(location: textField.stringValue.count,
                                                           length: 0)
    }

    private func moveCursor(to location: Int) {
        textField.currentEditor()?.selectedRange = NSRange(location: location,
                                                           length: 0)
    }
}
