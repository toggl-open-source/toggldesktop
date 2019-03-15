//
//  TimeEntryEmptyView.swift
//  TogglDesktop
//
//  Created by Nghia Tran on 3/14/19.
//  Copyright © 2019 Alari. All rights reserved.
//

import Cocoa

@objc protocol TimeEntryEmptyViewDelegate: class {

    func emptyViewDidTapOnLoadMore()
}

final class TimeEntryEmptyView: NSView {

    fileprivate struct Constants {

        static let stopwatchImage = NSImage.Name("stopwatch")
        static let spiderImage = NSImage.Name("spider")
    }

    @objc enum EmptyLayoutType: Int {
        case welcome
        case noEntry
    }

    // MARK: Variables

    @objc weak var delegate: TimeEntryEmptyViewDelegate?
    private var layoutType = EmptyLayoutType.welcome {
        didSet {
            layoutView()
        }
    }

    // MARK: OUTLET

    @IBOutlet weak var iconImageView: NSImageView!
    @IBOutlet weak var iconImageViewTop: NSLayoutConstraint!
    @IBOutlet weak var iconImageViewHeight: NSLayoutConstraint!
    @IBOutlet weak var iconImageViewWidth: NSLayoutConstraint!
    @IBOutlet weak var titleLabel: NSTextField!
    @IBOutlet weak var subTitleLabel: NSTextField!
    @IBOutlet weak var loadMoreBtn: NSButton!
    @IBOutlet weak var spinerView: NSProgressIndicator!
    
    // MARK: Public

    override func awakeFromNib() {
        super.awakeFromNib()

        layoutType = .welcome
    }

    @objc class func viewFromXIB() -> TimeEntryEmptyView {
        return TimeEntryEmptyView.xibView()
    }
    
    @objc func setLayoutType(_ layoutType: EmptyLayoutType) {
        self.layoutType = layoutType
    }

    @IBAction func loadMoreOnTap(_ sender: Any) {
        loadMoreBtn.title = ""
        spinerView.startAnimation(nil)
        spinerView.isHidden = false
        delegate?.emptyViewDidTapOnLoadMore()
    }
}

// MARK: Private

extension TimeEntryEmptyView {

    fileprivate func layoutView() {
        switch layoutType {
        case .welcome:
            iconImageView.image = NSImage(named: Constants.stopwatchImage)
            titleLabel.stringValue = "Welcome to Toggl"
            subTitleLabel.stringValue = "Time each activity you do and see where your hours go"
            loadMoreBtn.isHidden = true
            spinerView.stopAnimation(nil)
            spinerView.isHidden = true
            
            iconImageViewWidth.constant = 200
            iconImageViewHeight.constant = 150
            iconImageViewTop.constant = 20

        case .noEntry:
            iconImageView.image = NSImage(named: Constants.spiderImage)
            titleLabel.stringValue = "No recent entries"
            subTitleLabel.stringValue = "It’s been a long time since you’ve tracked your tasks!"
            loadMoreBtn.isHidden = false
            loadMoreBtn.title = "Load More"
            spinerView.stopAnimation(nil)
            spinerView.isHidden = true

            iconImageViewWidth.constant = 132
            iconImageViewHeight.constant = 170
            iconImageViewTop.constant = 0
        }
    }
}
