//
//  DSFSecureTextField.swift
//
//  Created by Darren Ford on 2/1/20.
//  Copyright © 2020 Darren Ford. All rights reserved.
//
//  MIT License
//
//  Permission is hereby granted, free of charge, to any person obtaining a copy
//  of this software and associated documentation files (the "Software"), to deal
//  in the Software without restriction, including without limitation the rights
//  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//  copies of the Software, and to permit persons to whom the Software is
//  furnished to do so, subject to the following conditions:
//
//  The above copyright notice and this permission notice shall be included in all
//  copies or substantial portions of the Software.
//
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
//  SOFTWARE.

import AppKit

/// Customize TextField with Eye Feature
@IBDesignable
public class DSFSecureTextField: NSSecureTextField {

    static let PasswordEyeSize = CGSize(width: 20, height: 20)

	/// Whether to display a toggle button into the control to control the visibility
	@IBInspectable
    public dynamic var displayToggleButton: Bool = true {
		didSet {
			self.updateForPasswordVisibility()
		}
	}

	/// Allow or disallow showing plain text password
	@IBInspectable
	public dynamic var allowShowPassword: Bool = true {
		didSet {
			self.passwordIsVisible = false
			self.configureButtonForState()
		}
	}

	/// Show or obscure the password
	@objc public dynamic var passwordIsVisible: Bool = false {
		didSet {
			self.updateForPasswordVisibility()
		}
	}

	// Embedded button if the style requires it
	private var visibilityButton: NSButton?

	public override init(frame frameRect: NSRect) {
		super.init(frame: frameRect)
		self.setup()
	}

	required init?(coder: NSCoder) {
		super.init(coder: coder)
	}

	override public func viewDidMoveToWindow() {
		super.viewDidMoveToWindow()
		self.setup()
	}
}

// MARK: - Private (DSFSecureTextField)

private extension DSFSecureTextField {

	func configureButtonForState() {

		if self.allowShowPassword && self.displayToggleButton {

            let button = CursorButton(frame: NSRect(x: 0,
                                                    y: 0,
                                                    width: DSFSecureTextField.PasswordEyeSize.width,
                                                    height: DSFSecureTextField.PasswordEyeSize.height))

			self.visibilityButton = button
			button.action = #selector(visibilityChanged(_:))
			button.target = self
            button.title = ""
            button.image = NSImage(named: "password-eye-close")
            button.alternateImage = NSImage(named: "password-eye-open")
            button.imagePosition = .imageAbove
            button.isBordered = false
            button.bezelStyle = .rounded
            button.cursor = .pointingHand
            button.setButtonType(.toggle)

            button.translatesAutoresizingMaskIntoConstraints = false
            self.addSubview(button)
            button.centerYAnchor.constraint(equalTo: self.centerYAnchor).isActive = true
            button.widthAnchor.constraint(equalToConstant: 20).isActive = true
            button.heightAnchor.constraint(equalToConstant: 20).isActive = true
            button.trailingAnchor.constraint(equalTo: self.trailingAnchor, constant: -5).isActive = true

			button.needsLayout = true
			self.needsUpdateConstraints = true
		} else {
			self.visibilityButton?.removeFromSuperview()
			self.visibilityButton = nil
		}
		self.window?.recalculateKeyViewLoop()
	}

	func setup() {

		// By default, the password should ALWAYS be hidden
		self.passwordIsVisible = false
		self.configureButtonForState()
		self.updateForPasswordVisibility()
	}

	// MARK: Updates

	// Triggered when the user clicks the embedded button
	@objc func visibilityChanged(_ sender: NSButton) {
		self.passwordIsVisible = (sender.state == .on)
	}

	func updateForPasswordVisibility() {
		let str = self.cell?.stringValue ?? ""

		if self.window?.firstResponder == self.currentEditor() {
			// Text field has focus.
			self.abortEditing()
		}

		let newCell: NSTextFieldCell!
		let oldCell: NSTextFieldCell = self.cell as! NSTextFieldCell

		if !self.displayToggleButton {
			// Button should NOT be included
			if self.passwordIsVisible {
				newCell = NSTextFieldCell()
				self.cell = newCell
			} else {
				newCell = NSSecureTextFieldCell()
				self.cell = newCell
			}
		} else {
			if self.allowShowPassword {
				newCell = self.passwordIsVisible ? DSFPlainTextFieldCell() : DSFPasswordTextFieldCell()
                newCell.focusRingType = .none
				self.cell = newCell
			} else {
				newCell = NSSecureTextFieldCell()
				self.cell = newCell
			}
		}

		newCell.isEditable = true
		newCell.placeholderString = oldCell.placeholderString
		newCell.isScrollable = true
		newCell.font = oldCell.font
		newCell.isBordered = oldCell.isBordered
		newCell.isBezeled = oldCell.isBezeled
		newCell.backgroundStyle = oldCell.backgroundStyle
		newCell.bezelStyle = oldCell.bezelStyle
		newCell.drawsBackground = oldCell.drawsBackground

		self.cell?.stringValue = str

		self.visibilityButton?.needsLayout = true
		self.needsUpdateConstraints = true
	}
}

// MARK: - Private implementation (Text Field Cells)

private class DSFPasswordTextFieldCell: NSSecureTextFieldCell {
	override func select(withFrame rect: NSRect,
                         in controlView: NSView,
                         editor textObj: NSText,
                         delegate: Any?,
                         start selStart: Int,
                         length selLength: Int) {
		var newRect = rect
		newRect.size.width -= DSFSecureTextField.PasswordEyeSize.width
		super.select(withFrame: newRect, in: controlView, editor: textObj, delegate: delegate, start: selStart, length: selLength)
	}

	override func edit(withFrame rect: NSRect, in controlView: NSView, editor textObj: NSText, delegate: Any?, event: NSEvent?) {
		var newRect = rect
		newRect.size.width -= DSFSecureTextField.PasswordEyeSize.width
		super.edit(withFrame: newRect, in: controlView, editor: textObj, delegate: delegate, event: event)
	}
}

private class DSFPlainTextFieldCell: NSTextFieldCell {
	override func select(withFrame rect: NSRect,
                         in controlView: NSView,
                         editor textObj: NSText,
                         delegate: Any?,
                         start selStart: Int,
                         length selLength: Int) {
		var newRect = rect
		newRect.size.width -= DSFSecureTextField.PasswordEyeSize.width
		super.select(withFrame: newRect, in: controlView, editor: textObj, delegate: delegate, start: selStart, length: selLength)
	}

	override func edit(withFrame rect: NSRect, in controlView: NSView, editor textObj: NSText, delegate: Any?, event: NSEvent?) {
		var newRect = rect
		newRect.size.width -= DSFSecureTextField.PasswordEyeSize.width
		super.edit(withFrame: newRect, in: controlView, editor: textObj, delegate: delegate, event: event)
	}
}
