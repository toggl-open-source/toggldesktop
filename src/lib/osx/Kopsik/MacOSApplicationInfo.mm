//
//  MacOSApplicationInfo.m
//  TogglDesktopLibrary
//
//  Created by Nghia Tran on 12/23/19.
//  Copyright © 2019 Toggl. All rights reserved.
//

#import <AppKit/AppKit.h>
#include <string.h>
#include <iostream>

@interface MacOSApplicationUtility : NSObject

+ (NSRunningApplication *) getFrontApplication;
@end

@implementation MacOSApplicationUtility

+ (NSRunningApplication *) getFrontApplication
{
    NSArray<NSRunningApplication *> *runningApps = [[NSWorkspace sharedWorkspace] runningApplications];
    for (NSRunningApplication *app in runningApps) {
        if (app.isActive) {
            return app;
        }
    }
    return nil;
}

@end

int macOSGetActiveWindowInfo(std::string *title, std::string *filename) {
    @autoreleasepool {
        *title = "";
        *filename = "";

        // Get front app
        NSRunningApplication *frontApp = [MacOSApplicationUtility getFrontApplication];
        if (frontApp == nil) {
            return -1;
        }

        // window title
        CFArrayRef _windows = CGWindowListCopyWindowInfo(kCGWindowListOptionOnScreenOnly, kCGNullWindowID);
        NSArray *windows = CFBridgingRelease(_windows);

        for (int i = 0; i < windows.count; i++) {
            NSDictionary *dict = windows[i];

            // Get window layer and skip the background
            NSNumber *window_layer = (NSNumber *) [dict objectForKey:(__bridge NSString *) kCGWindowLayer];
            if (window_layer.intValue != 0) {
                continue;
            }
            NSNumber *window_owner_pid = (NSNumber *) [dict objectForKey:(__bridge NSString *) kCGWindowOwnerPID];

            // Get app with pid
            NSRunningApplication *app = [NSRunningApplication runningApplicationWithProcessIdentifier:window_owner_pid.intValue];
            if (app == nil) {
                continue;
            }

            // Compare if it's the same process
            // We don't compare pid, we use -isEqual
            // Ref: https://developer.apple.com/documentation/appkit/nsrunningapplication/1526998-processidentifier
            if ([app isEqual:frontApp]) {
                NSString *window_name = (NSString *) [dict objectForKey:(__bridge NSString *) kCGWindowName];
                if (window_name) {
                    *title = std::string([window_name UTF8String]);
                    // If the title we got is emtpy, keep looking.
                    // This is needed to get the actual tab title.
                    if (!title->empty()) {
                        break;
                    }
                }
            }
        }

        // get application filename
        NSString *appName = frontApp.localizedName;
        if (appName == nil) {
            return -1;
        }
        *filename = std::string(appName.UTF8String);

        return 0;
    }
}
