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

    private lazy var projectDatasource = ProjectDataSource()

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
        projectBox.wantsLayer = true
        projectBox.layer?.masksToBounds = false
        projectBox.contentView?.wantsLayer = true
        projectBox.contentView?.layer?.masksToBounds = false
    }

    fileprivate func initDatasource() {
        projectDatasource.registerCustomeCells()
        projectTextField.prepare(with: projectDatasource, parentView: projectBox)
    }
}
