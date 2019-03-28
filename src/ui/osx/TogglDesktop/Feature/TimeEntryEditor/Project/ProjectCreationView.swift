//
//  ProjectCreationView.swift
//  TogglDesktop
//
//  Created by Nghia Tran on 3/28/19.
//  Copyright © 2019 Alari. All rights reserved.
//

import Cocoa

final class ProjectCreationView: NSView {

    // MARK: OUTLET

    @IBOutlet weak var addBtn: NSButton!
    @IBOutlet weak var projectAutoComplete: AutoCompleteTextField!
    @IBOutlet weak var marketingAutoComplete: AutoCompleteTextField!
    @IBOutlet weak var clientAutoComplete: AutoCompleteTextField!

    // MARK: Public

    override func awakeFromNib() {
        super.awakeFromNib()
    }

    @IBAction func cancelBtnOnTap(_ sender: Any) {
    }

    @IBAction func addBtnOnTap(_ sender: Any) {
    }

    @IBAction func publicProjectOnChange(_ sender: Any) {
    }

}

// MARK: Private

extension ProjectCreationView {

    fileprivate func initCommon() {
        
    }
}
