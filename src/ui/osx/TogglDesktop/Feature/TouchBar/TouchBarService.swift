//
//  TouchBarService.swift
//  TogglDesktop
//
//  Created by Nghia Tran on 10/1/19.
//  Copyright © 2019 Alari. All rights reserved.
//

import Foundation

@objc protocol TouchBarServiceDelegate: class {

    func touchBarServiceStartTimeEntryOnTap()
}

@available(OSX 10.12.2, *)
@objcMembers
final class TouchBarService: NSObject {

    static let shared = TouchBarService()

    enum DisplayState {
        case tracking
        case normal
    }

    fileprivate struct Constants {
        static let TimeEntryIdentifer = NSUserInterfaceItemIdentifier("TimeEntryScrubberItem")
    }

    // MARK: OUTLET

    private lazy var startButton: NSButton = {
        let btn = NSButton(title: "Start", target: self, action: #selector(self.startBtnOnTap(_:)))
        btn.image = NSImage(named: NSImage.touchBarPlayTemplateName)
        btn.alternateImage = NSImage(named: NSImage.touchBarRecordStopTemplateName)
        btn.imagePosition = .imageOnly
        btn.setButtonType(.toggle)
        return btn
    }()

    private lazy var runningTimeEntryBtn = NSButton(title: "Running...", target: nil, action: nil)

    // MARK: Variables

    lazy var touchBar = NSTouchBar()
    weak var delegate: TouchBarServiceDelegate?
    private var timeEntries: [TimeEntryViewItem] = []
    private var displayState = DisplayState.normal { didSet { updateDisplayState() }}
    private lazy var scrubberView: NSScrubber = {
        let view = NSScrubber(frame: .zero)
        view.delegate = self
        view.dataSource = self
        view.mode = NSScrubber.Mode.free
//        view.itemAlignment = NSScrubber.Alignment.leading
        view.register(TimeEntryScrubberItem.self, forItemIdentifier: Constants.TimeEntryIdentifer)
        return view
    }()

    // MARK: Init

    override init() {
        super.init()
        initCommon()
        setup()
        initNotification()
    }

    // MARK: Public

    func updateRunningItem(_ timeEntry: TimeEntryViewItem) {
        runningTimeEntryBtn.title = timeEntry.touchBarTitle
    }

    func updateTimeEntryList(_ timeEntries: [TimeEntryViewItem]) {
        self.timeEntries = Array(timeEntries.prefix(5))
        scrubberView.reloadData()
    }
}

// MARK: Private

@available(OSX 10.12.2, *)
extension TouchBarService {

    fileprivate func initCommon() {
        NSApplication.shared.isAutomaticCustomizeTouchBarMenuItemEnabled = false
    }

    fileprivate func setup() {
        touchBar.delegate = self
        touchBar.customizationIdentifier = .mainTouchBar
        touchBar.defaultItemIdentifiers = [.timeEntryItem, .startStopItem]
    }

    fileprivate func initNotification() {
        NotificationCenter.default.addObserver(self,
                                               selector: #selector(self.stateButtonTimerBarChangeNotification(_:)),
                                               name: NSNotification.Name(kStartButtonStateChange),
                                               object: nil)
    }

    @objc private func stateButtonTimerBarChangeNotification(_ noti: Notification) {
        guard let value = noti.object as? NSNumber else {
            return
        }
        startButton.state = NSControl.StateValue(rawValue: value.intValue)
        displayState = startButton.state == .on ? .tracking : .normal
    }

    private func updateDisplayState() {
        switch displayState {
        case .normal:
            touchBar.defaultItemIdentifiers.removeAll(where: { $0 == .runningTimeEntry })
        case .tracking:
            let count = touchBar.defaultItemIdentifiers.count
            touchBar.defaultItemIdentifiers.insert(.runningTimeEntry, at: count - 1)
        }
    }
}

// MARK: NSTouchBarDelegate

@available(OSX 10.12.2, *)
extension TouchBarService: NSTouchBarDelegate {

    func touchBar(_ touchBar: NSTouchBar, makeItemForIdentifier identifier: NSTouchBarItem.Identifier) -> NSTouchBarItem? {
        switch identifier {
        case NSTouchBarItem.Identifier.timeEntryItem:
            let item = NSCustomTouchBarItem(identifier: identifier)
            item.visibilityPriority = .high
            item.view = scrubberView
            return item
        case NSTouchBarItem.Identifier.runningTimeEntry:
            let item = NSCustomTouchBarItem(identifier: identifier)
//            item.visibilityPriority = .high
            item.view = runningTimeEntryBtn
            return item
        case NSTouchBarItem.Identifier.startStopItem:
            let item = NSCustomTouchBarItem(identifier: identifier)
//            item.visibilityPriority = .high
            item.view = startButton
            return item
        default:
            return nil
        }
    }
}

// MARK: Private

@available(OSX 10.12.2, *)
extension TouchBarService {

    @objc fileprivate func startBtnOnTap(_ sender: NSButton) {
        delegate?.touchBarServiceStartTimeEntryOnTap()
    }

    @objc fileprivate func timeEntryBtnOnTap(_ sender: NSButton) {

    }
}

@available(OSX 10.12.2, *)
extension TouchBarService: NSScrubberDelegate, NSScrubberDataSource {

    func numberOfItems(for scrubber: NSScrubber) -> Int {
        return timeEntries.count
    }

    func scrubber(_ scrubber: NSScrubber, viewForItemAt index: Int) -> NSScrubberItemView {
        let timeEntry = timeEntries[index]
        guard let cell = scrubber.makeItem(withIdentifier: Constants.TimeEntryIdentifer, owner: self) as? TimeEntryScrubberItem
            else { return NSScrubberItemView() }
        cell.config(timeEntry)
        return cell
    }

    func scrubber(_ scrubber: NSScrubber, didSelectItemAt selectedIndex: Int) {
        print(selectedIndex)
    }
}
