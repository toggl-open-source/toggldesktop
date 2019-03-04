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

    // MARK: Variables
    fileprivate var payload: SystemMessage.Payload?

    // MARK: Init

    required init?(coder decoder: NSCoder) {
        super.init(coder: decoder)
        initFloatingView()

        floatingView.onClose = {[weak self] in
            guard let strongSelf = self else { return }
            strongSelf.handleClosing()
        }
    }

    @objc class func initFromXib() -> SystemMessageView {
        return SystemMessageView.xibView()
    }

    // MARK: Public

    @objc func registerToSystemMessage() {
        SystemMessage.shared.register(for: self)
    }

    @IBAction func iconOnTap(_ sender: Any) {
        floatingView.isHidden = false
        iconContainerView.isHidden = true
    }
}

// MARK: SystemMessagePresentable

extension SystemMessageView: SystemMessagePresentable {

    func present(_ payload: SystemMessage.Payload) {
        self.payload = payload
        isHidden = false
        floatingView.isHidden = false
        iconContainerView.isHidden = true
        self.superview?.bringSubviewToFront(self)

        // handle icon
        switch payload.mode {
        case .syncing:
            iconBtn.image = NSImage(named: NSImage.Name("spinner-icon"))
        case .error,
             .information:
            iconBtn.image = nil
        case .offline:
            iconBtn.image = NSImage(named: NSImage.Name("offline-icon"))
        }

        // Floating view
        floatingView.present(payload)
    }

    func dismiss(_ payload: SystemMessage.Payload) {
        guard let currentPayload = self.payload else { return }

        // Only dismiss if the payload is matched with the current
        // It presents to dismiss by accidently
        guard currentPayload.mode == payload.mode else { return }

        isHidden = true
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
            , multiplier: 1.0, constant: 10)

        addConstraints([top, left, right, bottom])
    }

    fileprivate func handleClosing() {
        guard let payload = payload else { return }
        switch payload.mode {
        case .error,
             .information:
            // Hide all, because we don't have circle icon for information
            isHidden = true

        case .offline, .syncing:
            // Hide floating view
            // Then presenting the icon btn
            floatingView.isHidden = true
            iconContainerView.isHidden = false
        }
    }
}

extension NSView {

    func bringSubviewToFront(_ view: NSView) {
        var theView = view
        self.sortSubviews({(viewA,viewB,rawPointer) in
            let view = rawPointer?.load(as: NSView.self)

            switch view {
            case viewA:
                return ComparisonResult.orderedDescending
            case viewB:
                return ComparisonResult.orderedAscending
            default:
                return ComparisonResult.orderedSame
            }
        }, context: &theView)
    }

}
