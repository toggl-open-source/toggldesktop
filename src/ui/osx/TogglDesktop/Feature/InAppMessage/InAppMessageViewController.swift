//
//  InAppMessageViewController.swift
//  TogglDesktop
//
//  Created by Nghia Tran on 12/3/19.
//  Copyright © 2019 Alari. All rights reserved.
//

import Cocoa

@objc protocol InAppMessageViewControllerDelegate: class {

    func InAppMessageViewControllerShouldDismiss()
}

final class InAppMessageViewController: NSViewController {

    // MARK: Variables

    @IBOutlet weak var containerView: NSView!
    @IBOutlet weak var titleLbl: NSTextField!
    @IBOutlet weak var descriptionLbl: NSTextField!
    @IBOutlet weak var actionBtn: FlatButton!
    @IBOutlet weak var bottomContraint: NSLayoutConstraint!
    @IBOutlet weak var containerWidthConstraint: NSLayoutConstraint!
    
    // MARK: Variables

    @objc weak var delegate: InAppMessageViewControllerDelegate?
    private var message: InAppMessage?

    // MARK: View

    override func viewDidLoad() {
        super.viewDidLoad()
        initCommon()
    }

    @IBAction func closeBtnOnTap(_ sender: Any) {
        animate(with: {[weak self] in
            guard let strongSelf = self else { return }
            strongSelf.bottomContraint.animator().constant = -strongSelf.containerView.frame.height
        }, complete: {[weak self] in
                self?.delegate?.InAppMessageViewControllerShouldDismiss()
        })
    }

    @IBAction func actionBtnOnTap(_ sender: Any) {
        guard let message = message,
            let url = URL(string: message.urlAction) else { return }
        NSWorkspace.shared.open(url)
        closeBtnOnTap(self)
    }

    @objc func update(_ message: InAppMessage) {
        self.message = message
        titleLbl.stringValue = message.title
        titleLbl.toolTip = message.title
        descriptionLbl.stringValue = message.subTitle
        descriptionLbl.toolTip = message.subTitle
        actionBtn.title = message.buttonTitle
    }

    @objc func prepareForAnimation() {
        bottomContraint.constant = -containerView.frame.height
        if let superview = view.superview {
            containerWidthConstraint.constant = superview.frame.width
        }
    }

    @objc func present() {
        animate(with: {[weak self] in
            guard let strongSelf = self else { return }
            strongSelf.bottomContraint.animator().constant = 0
        })
    }
}

// MARK: Private

extension InAppMessageViewController {

    private func initCommon() {
        actionBtn.wantsLayer = true
        actionBtn.layer?.borderWidth = 1
        actionBtn.layer?.borderColor = NSColor.white.cgColor
        containerView.applyShadow(offset: CGSize(width: 0, height: 3))
    }

    private func animate(with block: () -> Void, complete: (() -> Void)? = nil) {
        NSAnimationContext.runAnimationGroup({(context) in
            context.duration = 0.3
            context.timingFunction = CAMediaTimingFunction(name: .easeOut)
            block()
        }, completionHandler: complete)
    }
}
