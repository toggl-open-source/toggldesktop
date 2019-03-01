//
//  SystemMessageView.swift
//  TogglDesktop
//
//  Created by Nghia Tran on 3/1/19.
//  Copyright © 2019 Alari. All rights reserved.
//

import Cocoa

final class SystemMessageView: NSView {

    // MARK: OUTLET
    @IBOutlet weak var iconContainerView: NSBox!
    @IBOutlet weak var iconBtn: NSButton!
    private lazy var floatingView: FloatingErrorView = {
        return FloatingErrorView.xibView()
    }()

    // MARK: Init

    required init?(coder decoder: NSCoder) {
        super.init(coder: decoder)
        initFloatingView()
    }

    // MARK: Public

    @objc func registerToSystemMessage() {
        SystemMessage.shared.register(for: self)
    }

    @IBAction func iconOnTap(_ sender: Any) {
        
    }
}

extension SystemMessageView: SystemMessagePresentable {

    func present(_ payload: SystemMessage.Payload) {
        
    }
}

// MARK: Private

extension SystemMessageView {

    fileprivate func initFloatingView() {
        self.addSubview(floatingView)
        floatingView.translatesAutoresizingMaskIntoConstraints = false
        let top = NSLayoutConstraint(item: self,
                                     attribute: .top,
                                     relatedBy: .equal,
                                     toItem: floatingView,
                                     attribute: .top
            , multiplier: 1.0, constant: 0)
        let left = NSLayoutConstraint(item: self,
                                      attribute: .left,
                                      relatedBy: .equal,
                                      toItem: floatingView,
                                      attribute: .left
            , multiplier: 1.0, constant: 0)
        let right = NSLayoutConstraint(item: self,
                                       attribute: .right,
                                       relatedBy: .equal,
                                       toItem: floatingView,
                                       attribute: .right
            , multiplier: 1.0, constant: 30)
        let bottom = NSLayoutConstraint(item: self,
                                        attribute: .bottom,
                                        relatedBy: .equal,
                                        toItem: floatingView,
                                        attribute: .bottom
            , multiplier: 1.0, constant: 0)

        addConstraints([top, left, right, bottom])
    }
}
