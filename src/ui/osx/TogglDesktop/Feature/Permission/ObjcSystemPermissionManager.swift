//
//  ObjcSystemPermissionManager.swift
//  TogglDesktop
//
//  Created by Nghia Tran on 11/15/19.
//  Copyright © 2019 Alari. All rights reserved.
//

import Foundation

@objc final class ObjcSystemPermissionManager: NSObject {

    @objc class func tryGrantScreenRecordingPermission() {
        SystemPermissionManager.shared.grant(.screenRecording)
    }
}
