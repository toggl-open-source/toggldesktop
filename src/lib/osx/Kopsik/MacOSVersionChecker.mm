//
//  MacOSVersionChecker.m
//  TogglDesktopLibrary
//
//  Created by Nghia Tran on 10/21/19.
//  Copyright © 2019 Toggl. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <Foundation/NSProcessInfo.h>
#include <string.h>
#include <iostream>

struct OSVersion {
    long major;
    long minor;
    long path;

    OSVersion(long major, long minor, long path) {
        this->major = major;
        this->minor = minor;
        this->path = path;
    }
};

OSVersion* getProcessInfoVersion (void) {
    NSProcessInfo *processInfo = [[NSProcessInfo alloc] init];

    // check avaiability of the property operatingSystemVersion (10.10+) at runtime
    if ([processInfo respondsToSelector:@selector(operatingSystemVersion)])
    {
        NSOperatingSystemVersion versionObj = [processInfo operatingSystemVersion];
        OSVersion *version = new OSVersion(versionObj.majorVersion, versionObj.minorVersion, versionObj.patchVersion);
        return version;
    }
    else
    {
        return nil;
    }
}

bool isCatalinaOSX(void) {
    OSVersion *version = getProcessInfoVersion();
    if (version == nullptr) {
        return false;
    }
    if (version->major >= 10 && version->minor >= 15) {
        return true;
    }
    return false;
}
