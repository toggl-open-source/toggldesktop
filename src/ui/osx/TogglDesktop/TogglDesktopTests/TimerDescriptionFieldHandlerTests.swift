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

    var controlTextDidEndEditingNotification: Notification!

    override func setUpWithError() throws {
        textField = AutoCompleteInput()

        // text field needs to be attached to window and be a first responder
        // to have `currentEditor() != nil`, that is needed for handler
        window = NSWindow()
        window.contentView?.addSubview(textField)
        window.makeKey()
        window.makeFirstResponder(textField)

        handler = TimerDescriptionFieldHandler(textField: textField, enableShortcuts: true)
        textField.delegate = handler

        controlTextDidEndEditingNotification = Notification(name: NSControl.textDidEndEditingNotification, object: textField, userInfo: nil)
    }

    override func tearDownWithError() throws {
    }

    func testRecalculatesStateOnCursorPositionChange() {
        textField.stringValue = "#abcd"
        moveCursorToEnd()
        XCTAssertEqual(handler.state, .tagsFilter("abcd"))

        moveCursor(to: 3)
        XCTAssertEqual(handler.state, .tagsFilter("ab"))

        moveCursor(to: 1)
        XCTAssertEqual(handler.state, .tagsFilter(""))

        moveCursor(to: 0)
        XCTAssertEqual(handler.state, .autocompleteFilter("#abcd"))
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

    // MARK: - Text processing

    func testControlTextDidChangeDoNotCrashIfNoEditor() {
        window.makeFirstResponder(nil)
    }

    func testControlTextDidChangeProjectShortcut() {
        assertControlTextDidChangeDetectShortcutAtStart(shortcut: "@", state: State.projectFilter)
        assertControlTextDidChangeDetectShortcutAtEnd(shortcut: "@", state: State.projectFilter)
        assertControlTextDidChangeNeedSpaceBeforeShortcut(shortcut: "@")
        assertControlTextDidChangeDetectShortcutInMiddle(shortcut: "@", state: State.projectFilter)
        assertControlTextDidChangeHandleTwoSameShortcuts(shortcut: "@", state: State.projectFilter)
    }

    func testControlTextDidChangeDetectTagShortcut() {
        assertControlTextDidChangeDetectShortcutAtStart(shortcut: "#", state: State.tagsFilter)
        assertControlTextDidChangeDetectShortcutAtEnd(shortcut: "#", state: State.tagsFilter)
        assertControlTextDidChangeNeedSpaceBeforeShortcut(shortcut: "#")
        assertControlTextDidChangeDetectShortcutInMiddle(shortcut: "#", state: State.tagsFilter)
        assertControlTextDidChangeHandleTwoSameShortcuts(shortcut: "#", state: State.tagsFilter)
    }

    func assertControlTextDidChangeDetectShortcutAtStart(shortcut: String, state: (String) -> State) {
        textField.stringValue = "\(shortcut)"
        moveCursorToEnd()
        XCTAssertEqual(handler.state, state(""))

        textField.stringValue = "\(shortcut)proj"
        moveCursorToEnd()
        XCTAssertEqual(handler.state, state("proj"))

        textField.stringValue = "\(shortcut)project with space"
        moveCursorToEnd()
        XCTAssertEqual(handler.state, state("project with space"))
    }

    func assertControlTextDidChangeDetectShortcutAtEnd(shortcut: String, state: (String) -> State) {
        textField.stringValue = "some \(shortcut)"
        moveCursorToEnd()
        XCTAssertEqual(handler.state, state(""))

        textField.stringValue = "some \(shortcut)proj"
        moveCursorToEnd()
        XCTAssertEqual(handler.state, state("proj"))

        textField.stringValue = "some \(shortcut)pr with space and long"
        moveCursorToEnd()
        XCTAssertEqual(handler.state, state("pr with space and long"))
    }

    func assertControlTextDidChangeNeedSpaceBeforeShortcut(shortcut: String) {
        textField.stringValue = "some\(shortcut)withoutspace"
        moveCursorToEnd()
        XCTAssertEqual(handler.state, State.autocompleteFilter("some\(shortcut)withoutspace"))
    }

    func assertControlTextDidChangeDetectShortcutInMiddle(shortcut: String, state: (String) -> State) {
        textField.stringValue = "some \(shortcut)|text"
        moveCursor(to: 6)
        XCTAssertEqual(handler.state, state(""))

        textField.stringValue = "some \(shortcut)proj| text"
        moveCursor(to: 10)
        XCTAssertEqual(handler.state, state("proj"))

        textField.stringValue = "some \(shortcut)|text"
        moveCursor(to: 6)
        XCTAssertEqual(handler.state, state(""))

        textField.stringValue = "some \(shortcut)proj|text"
        moveCursor(to: 10)
        XCTAssertEqual(handler.state, state("proj"))
    }

    func assertControlTextDidChangeHandleTwoSameShortcuts(shortcut: String, state: (String) -> State) {
        textField.stringValue = "\(shortcut)\(shortcut)"
        moveCursorToEnd()
        XCTAssertEqual(handler.state, state("\(shortcut)"))

        textField.stringValue = "\(shortcut)\(shortcut)proj"
        moveCursorToEnd()
        XCTAssertEqual(handler.state, state("\(shortcut)proj"))

        textField.stringValue = "\(shortcut) \(shortcut)"
        moveCursorToEnd()
        XCTAssertEqual(handler.state, state(""))

        textField.stringValue = "\(shortcut) \(shortcut)second_proj"
        moveCursorToEnd()
        XCTAssertEqual(handler.state, state("second_proj"))
    }

    // MARK: - Text processing - Autocomplete

    func testControlTextDidChangeDetectAutocompleteFilter() {
        textField.stringValue = ""
        moveCursorToEnd()
        XCTAssertEqual(handler.state, State.autocompleteFilter(""))

        textField.stringValue = "some"
        moveCursorToEnd()
        XCTAssertEqual(handler.state, State.autocompleteFilter("some"))

        textField.stringValue = "some query with space"
        moveCursorToEnd()
        XCTAssertEqual(handler.state, State.autocompleteFilter("some query with space"))
    }

    func testControlTextDidChangeAutocompleteNotDependOnCursorLocation() {
        textField.stringValue = "some query"
        moveCursor(to: 4)
        XCTAssertEqual(handler.state, State.autocompleteFilter("some query"))
    }

    func testControlTextDidChangeWithDisabledProjectShortcutFlag() {
        handler.isShortcutEnabled = false

        textField.stringValue = "@"
        moveCursorToEnd()
        XCTAssertEqual(handler.state, State.autocompleteFilter("@"))

        textField.stringValue = "some @proj"
        moveCursorToEnd()
        XCTAssertEqual(handler.state, State.autocompleteFilter("some @proj"))
    }

    // MARK: - didCloseProjectDropdown

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

    // MARK: - didCloseTagsDropdown

    func testDidCloseTagsDropdownSetStateToDefaultFromTagsFilter() {
        setStateTagsFilter(filterText: "some")

        handler.didCloseTagsDropdown()

        XCTAssertEqual(handler.state, .descriptionUpdate)
    }

    func testDidCloseTagsDropdownDoesNothingWhenWrongState() {
        setStateAutocompleteFilter(filterText: "some")

        handler.didCloseTagsDropdown()

        XCTAssertEqual(handler.state, .autocompleteFilter("some"))
    }

    // MARK: - didSelectProject

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

    func testDidSelectProject() {
        assertDidSelectRemovesShortcutQueryFromEnd(shortcut: "@", selectFunction: handler.didSelectProject)
        assertDidSelectRemovesShortcutQueryFromStart(shortcut: "@", selectFunction: handler.didSelectProject)
        assertDidSelectRemovesShortcutQueryFromMiddle(shortcut: "@", selectFunction: handler.didSelectProject)
    }

    // MARK: - didSelectTag

    func testDidSelectTagSetStateToDefaultFromTagsFilter() {
        setStateTagsFilter(filterText: "some")

        handler.didSelectTag()

        XCTAssertEqual(handler.state, .descriptionUpdate)
    }

    func testDidSelectTagDoesNothingWhenWrongState() {
        setStateAutocompleteFilter(filterText: "some")

        handler.didSelectTag()

        XCTAssertEqual(handler.state, .autocompleteFilter("some"))
    }

    func testDidSelectTag() {
        assertDidSelectRemovesShortcutQueryFromEnd(shortcut: "#", selectFunction: handler.didSelectTag)
        assertDidSelectRemovesShortcutQueryFromStart(shortcut: "#", selectFunction: handler.didSelectTag)
        assertDidSelectRemovesShortcutQueryFromMiddle(shortcut: "#", selectFunction: handler.didSelectTag)
    }

    // MARK: - didSelect<Shortcut> Helpers

    func assertDidSelectRemovesShortcutQueryFromEnd(shortcut: String, selectFunction: () -> Void) {
        textField.stringValue = "some \(shortcut)project query"
        moveCursorToEnd()

        selectFunction()

        XCTAssertEqual(textField.stringValue, "some ")
        XCTAssertEqual(handler.state, State.descriptionUpdate)
    }

    func assertDidSelectRemovesShortcutQueryFromStart(shortcut: String, selectFunction: () -> Void) {
        textField.stringValue = "\(shortcut)project query"
        moveCursorToEnd()

        selectFunction()

        XCTAssertEqual(textField.stringValue, "")
        XCTAssertEqual(handler.state, State.descriptionUpdate)
    }

    func assertDidSelectRemovesShortcutQueryFromMiddle(shortcut: String, selectFunction: () -> Void) {
        textField.stringValue = "some \(shortcut)project| other text"
        moveCursor(to: 13)

        selectFunction()

        XCTAssertEqual(textField.stringValue, "some | other text")
        XCTAssertEqual(handler.state, State.descriptionUpdate)
    }

    // MARK: - Helpers

    private func setStateAutocompleteFilter(filterText: String) {
        textField.stringValue = filterText
        moveCursorToEnd()
    }

    private func setStateProjectFilter(filterText: String) {
        textField.stringValue = "@\(filterText)"
        moveCursorToEnd()
    }

    private func setStateTagsFilter(filterText: String) {
        textField.stringValue = "#\(filterText)"
        moveCursorToEnd()
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
