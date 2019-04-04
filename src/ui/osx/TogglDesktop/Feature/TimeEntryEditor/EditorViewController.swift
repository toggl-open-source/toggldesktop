//
//  EditorViewController.swift
//  TogglDesktop
//
//  Created by Nghia Tran on 3/21/19.
//  Copyright © 2019 Alari. All rights reserved.
//

import Cocoa

final class EditorViewController: NSViewController {

    // MARK: OUTLET

    @IBOutlet weak var projectBox: NSBox!
    @IBOutlet weak var projectTextField: ProjectAutoCompleteTextField!
    @IBOutlet weak var descriptionTextField: NSTextField!
    @IBOutlet weak var tagTextField: NSTextField!
    @IBOutlet weak var billableCheckBox: NSButton!
    @IBOutlet weak var projectDotImageView: DotImageView!
    @IBOutlet weak var closeBtn: CursorButton!

    // MARK: Variables

    var timeEntry: TimeEntryViewItem! {
        didSet {
            fillData()
        }
    }
    private var selectedProjectItem: ProjectContentItem?
    private lazy var projectDatasource = ProjectDataSource(items: ProjectStorage.shared.items,
                                                           updateNotificationName: .ProjectStorageChangedNotification)

    // MARK: View Cycle

    override func viewDidLoad() {
        super.viewDidLoad()

        initCommon()
        initDatasource()
    }
    
    @IBAction func closeBtnOnTap(_ sender: Any) {
        DesktopLibraryBridge.shared().togglEditor()
    }
}

// MARK: Private

extension EditorViewController {

    fileprivate func initCommon() {
        view.wantsLayer = true
        view.layer?.masksToBounds = false
        closeBtn.cursor = .pointingHand
        
        projectTextField.dotImageView = projectDotImageView
        projectTextField.layoutArrowBtn(with: view)

        descriptionTextField.delegate = self
    }

    fileprivate func initDatasource() {
        projectDatasource.delegate = self
        projectDatasource.setup(with: projectTextField)
    }

    fileprivate func fillData() {
        descriptionTextField.stringValue = timeEntry.descriptionName
        billableCheckBox.state = timeEntry.billable ? .on : .off
        projectTextField.setTimeEntry(timeEntry)
    }
}

// MARK: AutoCompleteViewDataSourceDelegate

extension EditorViewController: AutoCompleteViewDataSourceDelegate {

    func autoCompleteSelectionDidChange(sender: AutoCompleteViewDataSource, item: Any) {
        if sender == projectDatasource {
            if let projectItem = item as? ProjectContentItem {
                selectedProjectItem = projectItem
                projectTextField.projectItem = projectItem
                projectTextField.closeSuggestion()


                // Update
                let item = projectItem.item
                DesktopLibraryBridge.shared().setProjectForTimeEntryWithGUID(timeEntry.guid,
                                                                             taskID: item.taskID,
                                                                             projectID: item.projectID)
            }
        }
    }
}

extension EditorViewController: NSTextFieldDelegate {

    func controlTextDidEndEditing(_ obj: Notification) {
        guard timeEntry.descriptionName != descriptionTextField.stringValue else { return }
        let name = descriptionTextField.stringValue
        let guid = timeEntry.guid!
        DesktopLibraryBridge.shared().updateTimeEntry(withDescription: name, guid: guid)
    }
}
