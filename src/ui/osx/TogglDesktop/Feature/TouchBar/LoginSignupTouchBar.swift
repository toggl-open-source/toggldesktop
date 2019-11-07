//
//  LoginSignupTouchBar.swift
//  TogglDesktop
//
//  Created by Nghia Tran on 11/7/19.
//  Copyright © 2019 Alari. All rights reserved.
//

import Foundation

@available(OSX 10.12.2, *)
@objc protocol LoginSignupTouchBarDelegate: class {

    func loginSignupTouchBar(on action: LoginSignupTouchBar.LoginSignupAction)
}

@available(OSX 10.12.2, *)
final class LoginSignupTouchBar: NSObject {

    @objc enum LoginSignupAction: Int {
        case login
        case loginGoogle
        case signUp
        case signUpGoogle
    }

    @objc enum LoginSignupMode: Int {
        case login
        case signUp
    }

    // MARK: Variable

    @objc weak var delegate: LoginSignupTouchBarDelegate?

    private lazy var loginButton: NSButton = {
        let btn = NSButton(title: "Log in", target: self, action: #selector(self.btnOnTap(_:)))
        btn.setButtonType(.momentaryPushIn)
        btn.bezelColor = getButtonColor()
        return btn
    }()

    private lazy var loginWithGoogleButton: NSButton = {
        let btn = NSButton(title: "Log in with Google", target: self, action: #selector(self.btnOnTap(_:)))
        btn.setButtonType(.momentaryPushIn)
        btn.image = NSImage(named: "google-logo")
        btn.imageHugsTitle = true
        btn.imagePosition = .imageLeft
        return btn
    }()

    private lazy var signUpButton: NSButton = {
        let btn = NSButton(title: "Sign up", target: self, action: #selector(self.btnOnTap(_:)))
        btn.setButtonType(.momentaryPushIn)
        btn.bezelColor = getButtonColor()
        return btn
    }()

    private lazy var signUpWithGoogleButton: NSButton = {
        let btn = NSButton(title: "Sign up with Google", target: self, action: #selector(self.btnOnTap(_:)))
        btn.setButtonType(.momentaryPushIn)
        btn.image = NSImage(named: "google-logo")
        btn.imageHugsTitle = true
        btn.imagePosition = .imageLeft
        return btn
    }()

    // MARK: Public

    @objc func makeTouchBar(for mode: LoginSignupMode) -> NSTouchBar {
        let touchBar = NSTouchBar()
        touchBar.delegate = self
        touchBar.customizationIdentifier = .loginSignUpTouchBar

        switch mode {
        case .login:
            touchBar.defaultItemIdentifiers = [.loginItem,
                                               .loginGoogleItem]
        case .signUp:
            touchBar.defaultItemIdentifiers = [.signUpItem,
                                               .signUpGoogleItem]
        }

        return touchBar
    }

    @objc private func btnOnTap(_ sender: NSButton) {

    }
}


// MARK: Private

@available(OSX 10.12.2, *)
extension LoginSignupTouchBar: NSTouchBarDelegate {

    func touchBar(_ touchBar: NSTouchBar, makeItemForIdentifier identifier: NSTouchBarItem.Identifier) -> NSTouchBarItem? {
        switch identifier {
        case NSTouchBarItem.Identifier.loginItem:
            let item = NSCustomTouchBarItem(identifier: identifier)
            item.view = loginButton
            return item
        case NSTouchBarItem.Identifier.loginGoogleItem:
            let item = NSCustomTouchBarItem(identifier: identifier)
            item.view = loginWithGoogleButton
            return item
        case NSTouchBarItem.Identifier.signUpItem:
            let item = NSCustomTouchBarItem(identifier: identifier)
            item.view = signUpButton
            return item
        case NSTouchBarItem.Identifier.signUpGoogleItem:
            let item = NSCustomTouchBarItem(identifier: identifier)
            item.view = signUpWithGoogleButton
            return item
        default:
            return nil
        }
    }

    private func getButtonColor() -> NSColor? {
        if #available(OSX 10.13, *) {
            return NSColor(named: "login-button-background")
        } else {
            return ConvertHexColor.hexCode(toNSColor: "#FF2D55")!
        }
    }
}
