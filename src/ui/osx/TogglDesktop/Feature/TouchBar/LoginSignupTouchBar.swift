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
@objcMembers
final class LoginSignupTouchBar: NSObject {

    @objc enum LoginSignupAction: Int {
        case login
        case loginGoogle
        case signUp
        case signUpGoogle
        case loginApple
        case signUpApple
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

    private lazy var signUpWithAppleButton: NSButton = {
        let btn = NSButton(title: "Sign up with Apple", target: self, action: #selector(self.btnOnTap(_:)))
        btn.setButtonType(.momentaryPushIn)
        btn.image = NSImage(named: "apple-sign-in-logo-touch-bar")
        btn.imageHugsTitle = true
        btn.imagePosition = .imageLeft
        return btn
    }()

    private lazy var loginWithAppleButton: NSButton = {
        let btn = NSButton(title: "Log in with Apple", target: self, action: #selector(self.btnOnTap(_:)))
        btn.setButtonType(.momentaryPushIn)
        btn.image = NSImage(named: "apple-sign-in-logo-touch-bar")
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
                                               .loginAppleItem,
                                               .loginGoogleItem]
        case .signUp:
            touchBar.defaultItemIdentifiers = [.signUpItem,
                                               .signUpAppleItem,
                                               .signUpGoogleItem]
        }

        return touchBar
    }

    @objc private func btnOnTap(_ sender: NSButton) {
        switch sender {
        case loginButton:
            delegate?.loginSignupTouchBar(on: .login)
        case loginWithGoogleButton:
            delegate?.loginSignupTouchBar(on: .loginGoogle)
        case signUpButton:
            delegate?.loginSignupTouchBar(on: .signUp)
        case signUpWithGoogleButton:
            delegate?.loginSignupTouchBar(on: .signUpGoogle)
        case loginWithAppleButton:
            delegate?.loginSignupTouchBar(on: LoginSignupTouchBar.LoginSignupAction.loginApple)
        case signUpWithAppleButton:
            delegate?.loginSignupTouchBar(on: LoginSignupTouchBar.LoginSignupAction.signUpApple)
        default:
            break
        }
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
        case NSTouchBarItem.Identifier.loginAppleItem:
            let item = NSCustomTouchBarItem(identifier: identifier)
            item.view = loginWithAppleButton
            return item
        case NSTouchBarItem.Identifier.signUpAppleItem:
            let item = NSCustomTouchBarItem(identifier: identifier)
            item.view = signUpWithAppleButton
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
