//
//  MacOSApplicationInfo.m
//  TogglDesktopLibrary
//
//  Created by Nghia Tran on 12/23/19.
//  Copyright © 2019 Toggl. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <Foundation/NSProcessInfo.h>
#import <AppKit/AppKit.h>
#include <string.h>
#include <iostream>

static const int kTitleBufferSize = 255;
static const int kFilenameBufferSize = 255;

int macOSGetWindowInfo(
    std::string *title,
    std::string *filename,
    bool *idle) {
    *title = "";
    *filename = "";
    *idle = false;

    // get pid
    NSRunningApplication *frontApp;
    NSArray<NSRunningApplication *> *runningApps = [[NSWorkspace sharedWorkspace] runningApplications];
    for (NSRunningApplication *app in runningApps) {
        if (app.isActive) {
            frontApp = app;
            break;
        }
    }

    if (frontApp == nil) {
        return -1;
    }

    // window title
    CFArrayRef windows = CGWindowListCopyWindowInfo(
        kCGWindowListOptionOnScreenOnly, kCGNullWindowID);
    for (int i = 0; i < CFArrayGetCount(windows); i++) {
        CFDictionaryRef dict = (CFDictionaryRef)CFArrayGetValueAtIndex(
            windows, i);

        CFNumberRef window_layer = (CFNumberRef)CFDictionaryGetValue(
            dict, kCGWindowLayer);
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

        ProcessSerialNumber owner_serial_number;
        GetProcessForPID(window_owner_pid_int, &owner_serial_number);

        Boolean are_same;
        OSStatus err = SameProcess(
            &front_process_serial_number, &owner_serial_number, &are_same);
        if (err) {
            continue;
        }

        NSRunningApplication *app = [NSRunningApplication runningApplicationWithProcessIdentifier:window_owner_pid_int];
        if (app == nil) {
            continue;
        }
        if ([app isEqual:frontApp]) {

        }

        if (TRUE == are_same) {
            CFStringRef window_name = (CFStringRef)CFDictionaryGetValue(
                dict, kCGWindowName);
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
    CFStringRef processName = NULL;
    err = CopyProcessName(&front_process_serial_number, &processName);
    if (err) {
        return err;
    }
    char filename_buffer[kFilenameBufferSize];
    CFStringGetCString(processName, filename_buffer,
                       kFilenameBufferSize, kCFStringEncodingUTF8);
    CFRelease(processName);
    *filename = std::string(filename_buffer);

    return 0;
}
