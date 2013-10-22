//
//  AppDelegate.h
//  test2
//
//  Created by Alari on 9/15/13.
//  Copyright (c) 2013 kopsik developers. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "kopsik_api.h"

@interface AppDelegate : NSObject <NSApplicationDelegate>
@property NSStatusItem *statusItem;
- (IBAction)onPreferencesMenuItem:(id)sender;
@end
