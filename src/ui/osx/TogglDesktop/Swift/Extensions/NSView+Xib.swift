//
//  NSView+Xib.swift
//  TogglDesktop
//
//  Created by Nghia Tran on 1/28/19.
//  Copyright © 2019 Alari. All rights reserved.
//

import Cocoa

extension NSObject {

    class func xibView<T: NSObject>() -> T {
        let name = String(describing: self)
        let nib = NSNib(nibNamed: name, bundle: nil)!
        var objectArray: NSArray?
        nib.instantiate(withOwner: self, topLevelObjects: &objectArray)
        for view in objectArray! {
            if let xibView = view as? T {
                return xibView
            }
        }
        fatalError("XIB error")
    }
}
