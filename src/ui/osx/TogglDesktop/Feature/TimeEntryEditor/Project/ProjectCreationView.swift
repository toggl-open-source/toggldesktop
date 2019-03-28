//
//  ProjectCreationView.swift
//  TogglDesktop
//
//  Created by Nghia Tran on 3/28/19.
//  Copyright © 2019 Alari. All rights reserved.
//

import Cocoa

final class ProjectCreationView: NSView {

    enum DisplayMode {
        case compact
        case full // with color picker
    }

    // MARK: OUTLET

    @IBOutlet weak var addBtn: NSButton!
    @IBOutlet weak var projectAutoComplete: AutoCompleteTextField!
    @IBOutlet weak var marketingAutoComplete: AutoCompleteTextField!
    @IBOutlet weak var clientAutoComplete: AutoCompleteTextField!
    @IBOutlet weak var colorBtn: NSButton!
    @IBOutlet weak var colorPickerContainerView: NSView!

    // MARK: Variables
    private var selectedColor = ProjectColor.default
    private lazy var colorPickerView: ColorPickerView = {
        let picker = ColorPickerView.xibView() as ColorPickerView
        colorPickerContainerView.addSubview(picker)
        picker.edgesToSuperView()
        return picker
    }()
    private var displayMode = DisplayMode.compact {
        didSet {
            updateLayout()
        }
    }

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

    @IBAction func colorBtnOnTap(_ sender: Any) {
    }
}

// MARK: Private

extension ProjectCreationView {

    fileprivate func initCommon() {
        colorPickerContainerView.isHidden = true
    }

    fileprivate func updateLayout() {
        switch displayMode {
        case .compact:
            colorPickerContainerView.isHidden = true
            self.frame = CGRect(x: frame.origin.x, y: frame.origin.y, width: frame.size.width, height: 200.0)
        case .full:
            colorPickerContainerView.isHidden = false
            colorPickerView.select(selectedColor)
            self.frame = CGRect(x: frame.origin.x, y: frame.origin.y, width: frame.size.width, height: 398.0)
        }
    }
}
