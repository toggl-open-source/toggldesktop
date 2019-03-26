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
    @IBOutlet weak var projectTextField: AutoCompleteTextField!

    // MARK: Variables

    private lazy var projectDatasource = ProjectDataSource(items: ProjectStorage.shared.items,
                                                           updateNotificationName: .ProjectStorageChangedNotification)

    // MARK: View Cycle
    override func viewDidLoad() {
        super.viewDidLoad()

        initCommon()
        initDatasource()
    }
    
    @IBAction func closeBtnOnTap(_ sender: Any) {
        
    }
}

// MARK: Private

extension EditorViewController {

    fileprivate func initCommon() {
        view.wantsLayer = true
        view.layer?.masksToBounds = false
        projectTextField.wantsLayer = true
        projectTextField.layer?.masksToBounds = true
        projectTextField.layer?.cornerRadius = 8
    }

    fileprivate func initDatasource() {
        projectTextField.prepare(with: projectDatasource, parentView: view)
    }
}
