//
//  LayerBackedViewController.swift
//  TogglDesktop
//
//  Created by Nghia Tran on 1/13/20.
//  Copyright © 2020 Alari. All rights reserved.
//

import Cocoa

class LayerBackedViewController: NSViewController {

    override func viewDidLoad() {
        super.viewDidLoad()
        initCommon()
    }
}

extension LayerBackedViewController {

    private func initCommon() {
        view.wantsLayer = true
    }
}
