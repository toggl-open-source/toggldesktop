//
//  ProjectCreationView.swift
//  TogglDesktop
//
//  Created by Nghia Tran on 3/28/19.
//  Copyright © 2019 Alari. All rights reserved.
//

import Cocoa

protocol ProjectCreationViewDelegate: class {

    func projectCreationDidCancel()
    func projectCreationDidAdd()
    func projectCreationDidUpdateSize()
}

final class ProjectCreationView: NSView {

    enum DisplayMode {
        case compact
        case full // with color picker

        var height: CGFloat {
            switch self {
            case .compact:
                return 200.0
            case .full:
                return 400.0
            }
        }
    }

    // MARK: OUTLET

    @IBOutlet weak var addBtn: NSButton!
    @IBOutlet weak var projectTextField: NSTextField!
    @IBOutlet weak var marketingAutoComplete: AutoCompleteTextField!
    @IBOutlet weak var clientAutoComplete: AutoCompleteTextField!
    @IBOutlet weak var colorBtn: CursorButton!
    @IBOutlet weak var colorPickerContainerView: NSView!

    // MARK: Variables
    weak var delegate: ProjectCreationViewDelegate?
    private var selectedColor = ProjectColor.default {
        didSet {
            updateSelectColorView()
        }
    }
    private lazy var colorPickerView: ColorPickerView = {
        let picker = ColorPickerView.xibView() as ColorPickerView
        picker.delegate = self
        colorPickerContainerView.addSubview(picker)
        picker.edgesToSuperView()
        return picker
    }()
    private var displayMode = DisplayMode.compact {
        didSet {
            updateLayout()
        }
    }
    var suitableHeight: CGFloat {
        return displayMode.height
    }

    // MARK: Public

    override func awakeFromNib() {
        super.awakeFromNib()

        initCommon()
    }

    func setTitleAndFocus(_ title: String) {
        projectTextField.stringValue = title
        window?.makeFirstResponder(projectTextField)
    }

    @IBAction func cancelBtnOnTap(_ sender: Any) {
        delegate?.projectCreationDidCancel()
    }

    @IBAction func addBtnOnTap(_ sender: Any) {
        delegate?.projectCreationDidAdd()
    }

    @IBAction func publicProjectOnChange(_ sender: Any) {

    }

    @IBAction func colorBtnOnTap(_ sender: Any) {
        let isON = colorBtn.state == .on
        displayMode = isON ? .full : .compact
        colorBtn.layer?.borderWidth = isON ? 4.0 : 0.0
    }
}

// MARK: Private

extension ProjectCreationView {

    fileprivate func initCommon() {
        colorPickerView.isHidden = false
        colorPickerContainerView.isHidden = true
        colorBtn.wantsLayer = true
        colorBtn.layer?.cornerRadius = 12.0
        colorBtn.layer?.borderColor = colorBtnBorderColor.cgColor
        colorBtn.cursor = .pointingHand

        // Default value
        selectedColor = ProjectColor.default
        displayMode = .compact
    }

    fileprivate func updateLayout() {
        let height = displayMode.height
        switch displayMode {
        case .compact:
            colorPickerContainerView.isHidden = true
            self.frame = CGRect(x: frame.origin.x, y: frame.origin.y, width: frame.size.width, height: height)
        case .full:
            colorPickerContainerView.isHidden = false
            colorPickerView.select(selectedColor)
            self.frame = CGRect(x: frame.origin.x, y: frame.origin.y, width: frame.size.width, height: height)
        }
        delegate?.projectCreationDidUpdateSize()
    }

    fileprivate func updateSelectColorView() {
        colorBtn.layer?.backgroundColor = ConvertHexColor.hexCode(toNSColor: selectedColor.colorHex)!.cgColor
    }

    fileprivate var colorBtnBorderColor: NSColor {
        if #available(OSX 10.13, *) {
            return NSColor(named: "color-project-btn-border-color")!
        } else {
            return NSColor(white: 0, alpha: 0.1)
        }
    }
}

// MARK: ColorPickerViewDelegate

extension ProjectCreationView: ColorPickerViewDelegate {

    func colorPickerDidSelectColor(_ color: ProjectColor) {
        selectedColor = color
    }

    func colorPickerDidSelectCustomeColor(_ color: ProjectColor) {
        
    }
}
