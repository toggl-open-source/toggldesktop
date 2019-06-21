//
//  TimelineDashboardViewController.swift
//  TogglDesktop
//
//  Created by Nghia Tran on 6/17/19.
//  Copyright © 2019 Alari. All rights reserved.
//

import Cocoa

final class TimelineDashboardViewController: NSViewController {

    // MARK: OUTLET

    @IBOutlet weak var datePickerContainerView: NSView!
    @IBOutlet weak var recordSwitcher: OGSwitch!
    @IBOutlet weak var collectionView: NSCollectionView!

    // MARK: Variables

    lazy var datePickerView: DatePickerView = DatePickerView.xibView()
    private lazy var datasource = TimelineDatasource(collectionView)

    // MARK: View
    
    override func viewDidLoad() {
        super.viewDidLoad()

        initCommon()
        initNotifications()
        initCollectionView()

        DesktopLibraryBridge.shared().fetchTimelineData()
    }

    deinit {
        NotificationCenter.default.removeObserver(self)
    }

    func updateNextKeyView() {
        recordSwitcher.nextKeyView = datePickerView.previousDateBtn
        datePickerView.updateNextKeyView()
    }

    func render(with cmd: TimelineDisplayCommand) {
        let timeline = TimelineData(cmd: cmd)
        let date = Date(timeIntervalSince1970: cmd.start)
        datePickerView.currentDate = date
        datasource.render(timeline)
    }
    
    @IBAction func recordSwitchOnChanged(_ sender: Any) {
        DesktopLibraryBridge.shared().enableTimelineRecord(recordSwitcher.isOn)
    }

    @IBAction func zoomLevelOnChanged(_ sender: Any) {
        datasource.update(.x1)
    }
}

// MARK: Private

extension TimelineDashboardViewController {

    fileprivate func initCommon() {
        datePickerContainerView.addSubview(datePickerView)
        datePickerView.edgesToSuperView()
        datePickerView.delegate = self
    }

    fileprivate func initNotifications() {
        NotificationCenter.default.addObserver(self,
                                               selector: #selector(self.handleDisplaySettingNotification(_:)),
                                               name: NSNotification.Name(kDisplaySettings),
                                               object: nil)
        NotificationCenter.default.addObserver(self,
                                               selector: #selector(self.handleLoginNotification(_:)),
                                               name: NSNotification.Name(kDisplayLogin),
                                               object: nil)
    }

    fileprivate func initCollectionView() {

    }

    @objc private func handleLoginNotification(_ noti: Notification) {
        guard let cmd = noti.object as? DisplayCommand else { return }
        recordSwitcher.isEnabled = cmd.user_id != 0
    }

    @objc private func handleDisplaySettingNotification(_ noti: Notification) {
        guard let cmd = noti.object as? DisplayCommand,
            let setting = cmd.settings else { return }
        recordSwitcher.setOn(isOn: setting.timeline_recording_enabled, animated: false)
    }
}

// MARK: DatePickerViewDelegate

extension TimelineDashboardViewController: DatePickerViewDelegate {

    func datePickerOnChanged(_ sender: DatePickerView, date: Date) {

    }

    func datePickerShouldClose(_ sender: DatePickerView) {

    }

    func isTimeEntryRunning(_ sender: DatePickerView) -> Bool {
        return false
    }

    func shouldOpenCalendar(_ sender: DatePickerView) -> Bool {
        return false
    }

    func datePickerDidTapPreviousDate(_ sender: DatePickerView) {
        DesktopLibraryBridge.shared().timelineSetPreviousDate()
    }

    func datePickerDidTapNextDate(_ sender: DatePickerView) {
        DesktopLibraryBridge.shared().timelineSetNextDate()
    }
}
