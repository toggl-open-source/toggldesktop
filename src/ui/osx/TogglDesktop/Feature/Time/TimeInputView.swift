//
//  TimeInputView.swift
//  TogglDesktop
//
//  Created by Nghia Tran on 5/3/19.
//  Copyright © 2019 Alari. All rights reserved.
//

import Cocoa

protocol TimeInputViewDelegate: class {

    func timeInputDidSelect(_ time: TimeData, with selection: TimeInputView.Selection)
}

final class TimeInputView: NSView {

    enum Selection {
        case hour
        case minute
        case second
        case none
    }

    enum DisplayMode {
        case compact // only hour + minute
        case full // all

        var font: NSFont {
            switch self {
            case .compact:
                return NSFont.systemFont(ofSize: 14.0)
            case .full:
                return NSFont.systemFont(ofSize: 19.0)
            }
        }
    }

    fileprivate struct TimeComponent {

        let value: String
        let selection: Selection
    }

    // MARK: OUTLET

    @IBOutlet weak var titleLbl: NSTextField!

    // MARK: Variables

    weak var delegate: TimeInputViewDelegate?
    private var time: TimeData! {
        didSet {
            baseAtt = generateBaseAttribute()
            selectionAtt = generateSelectionAttribute()
            renderTimeTitle()
        }
    }
    private var mode: DisplayMode = .full {
        didSet {
            renderTimeTitle()
        }
    }
    fileprivate var currentSelection: Selection = .none
    private var greenColor: NSColor {
        if #available(OSX 10.13, *) {
            return NSColor(named: NSColor.Name("green-color"))!
        } else {
            return ConvertHexColor.hexCode(toNSColor: "#28cd41")
        }
    }
    fileprivate lazy var baseAtt: [NSAttributedString.Key: Any] = generateBaseAttribute()
    fileprivate lazy var selectionAtt: [NSAttributedString.Key: Any] = generateSelectionAttribute()

    // MARK: View cycle

    override func awakeFromNib() {
        super.awakeFromNib()
    }

    func updateLayout(with mode: DisplayMode) {
        self.mode = mode
    }

    func render(with date: Date, isSelected: Bool) {

        // If there is no selection -> select hours as defaul
        if isSelected && currentSelection == .none {
            currentSelection = .hour
        }

        self.time = TimeData(date: date)
    }
}

// MARK: Private

extension TimeInputView {


    fileprivate func renderTimeTitle() {
        guard let time = time else { return }

        // Get componetns
        var components: [TimeComponent] = []
        switch mode {
        case .full:
            components = [TimeComponent(value: "\(time.hour)", selection: .hour),
                          TimeComponent(value: "\(time.minute)", selection: .minute),
                          TimeComponent(value: "\(time.second)", selection: .second)]
        case .compact:
            components = [TimeComponent(value: "\(time.hour)", selection: .hour),
                          TimeComponent(value: "\(time.minute)", selection: .minute)]
        }


        // Combine all components
        // Render attribute if it's selected
        let emptyString = NSMutableAttributedString(string: "", attributes: baseAtt)
        let attributedTitle = components.reduce(into: emptyString) { (previousResult, component) in
            let attribute = component.selection == currentSelection ? selectionAtt : baseAtt
            let subTitle = NSAttributedString(string: component.value,
                                              attributes: attribute)
            if !previousResult.string.isEmpty {
                previousResult.append(NSAttributedString(string: ":", attributes: baseAtt))
            }
            previousResult.append(subTitle)
        }
        titleLbl.attributedStringValue = attributedTitle
    }

    fileprivate func generateBaseAttribute() -> [NSAttributedString.Key: Any] {
        return [NSAttributedString.Key.font: mode.font,
                NSAttributedString.Key.foregroundColor: NSColor.labelColor]
    }

    fileprivate func generateSelectionAttribute() -> [NSAttributedString.Key: Any] {
        return [NSAttributedString.Key.font: mode.font,
                NSAttributedString.Key.foregroundColor: greenColor]
    }


}
