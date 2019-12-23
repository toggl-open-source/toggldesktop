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

static const int kTitleBufferSize = 255;

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
    *title = "";
    *filename = "";

    // Get front app
    NSRunningApplication *frontApp = [MacOSApplicationUtility getFrontApplication];
    if (frontApp == nil) {
        return -1;
    }

    // window title
    CFArrayRef windows = CGWindowListCopyWindowInfo(kCGWindowListOptionOnScreenOnly, kCGNullWindowID);
    for (int i = 0; i < CFArrayGetCount(windows); i++) {
        CFDictionaryRef dict = (CFDictionaryRef)CFArrayGetValueAtIndex(
            windows, i);

        // Get window layer and skip the background
        CFNumberRef window_layer = (CFNumberRef)CFDictionaryGetValue(dict, kCGWindowLayer);
        int window_layer_int;
        CFNumberGetValue(window_layer, kCFNumberIntType, &window_layer_int);
        if (window_layer_int != 0) {
            continue;
        }

        CFNumberRef window_owner_pid = (CFNumberRef)CFDictionaryGetValue(
            dict, kCGWindowOwnerPID);
        int window_owner_pid_int;
        CFNumberGetValue(window_owner_pid, kCFNumberIntType,
                         &window_owner_pid_int);

        // Get app with pid
        NSRunningApplication *app = [NSRunningApplication runningApplicationWithProcessIdentifier:window_owner_pid_int];
        if (app == nil) {
            continue;
        }

        // Compare if it's the same process
        // We don't compare pid, we use -isEqual
        // Ref: https://developer.apple.com/documentation/appkit/nsrunningapplication/1526998-processidentifier
        if ([app isEqual:frontApp]) {
            CFStringRef window_name = (CFStringRef)CFDictionaryGetValue(dict, kCGWindowName);
            if (window_name) {
                char title_buffer[kTitleBufferSize];
                CFStringGetCString(window_name, title_buffer,
                                   kTitleBufferSize, kCFStringEncodingUTF8);
                *title = std::string(title_buffer);
                // If the title we got is emtpy, keep looking.
                // This is needed to get the actual tab title.
                if (!title->empty()) {
                    break;
                }
            }
        }
    }
    CFRelease(windows);

    // get application filename
    NSString *appName = frontApp.localizedName;
    if (appName == nil) {
        return -1;
    }
    *filename = std::string(appName.UTF8String);

    return 0;
}
