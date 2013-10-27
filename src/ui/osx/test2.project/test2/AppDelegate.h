//
//  AppDelegate.h
//  test2
//
//  Created by Alari on 9/15/13.
//  Copyright (c) 2013 kopsik developers. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@interface AppDelegate : NSObject <NSApplicationDelegate, NSUserNotificationCenterDelegate>
@property NSStatusItem *statusItem;
@property NSImage* onImage;
@property NSImage* offImage;
- (IBAction)onPreferencesMenuItem:(id)sender;
@end
