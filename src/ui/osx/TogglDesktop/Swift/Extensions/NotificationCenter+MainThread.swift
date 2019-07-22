//
//  NotificationCenter+MainThread.swift
//  TogglDesktop
//
//  Created by Nghia Tran on 1/10/19.
//  Copyright © 2019 Alari. All rights reserved.
//

import Foundation

extension NotificationCenter {

    @objc func postNotificationOnMainThread(_ aName: NSNotification.Name,
                                            object anObject: Any?) {
        postNotificationOnMainThread(aName,
                                     object: anObject,
                                     userInfo: nil)
    }

    @objc func postNotificationOnMainThread(_ aName: NSNotification.Name,
                                            object anObject: Any?,
                                            userInfo: [AnyHashable: Any]?) {
        runOnMainThreadIfNeed {[unowned self] in
            self.post(name: aName, object: anObject, userInfo: userInfo)
        }
    }
}

func runOnMainThreadIfNeed(_ block: @escaping () -> Void) {

    // If it's main thread, just execute
    if Thread.isMainThread {
        block()
        return
    }

    // Or run async on Main Thread later
    DispatchQueue.main.async {
        block()
    }
}
