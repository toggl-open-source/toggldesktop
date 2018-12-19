//
//  AppDelegate.h
//  test2
//
//  Created by Alari on 9/15/13.
//  Copyright (c) 2013 TogglDesktop developers. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "Reachability.h"

@interface AppDelegate : NSObject <NSApplicationDelegate, NSUserNotificationCenterDelegate>
@property (strong) NSStatusItem *statusItem;
@property (strong) NSImage *activeAppIcon;
@property (retain, nonatomic) Reachability *reach;
- (IBAction)onPreferencesMenuItem:(id)sender;
- (IBAction)onAboutMenuItem:(id)sender;
- (IBAction)onSyncMenuItem:(id)sender;
- (IBAction)onShowMenuItem:(id)sender;
- (IBAction)onEditMenuItem:(id)sender;
- (IBAction)onClearCacheMenuItem:(id)sender;
- (IBAction)onLogoutMenuItem:(id)sender;
- (IBAction)onHelpMenuItem:(id)sender;
- (IBAction)onNewMenuItem:(id)sender;
- (IBAction)onContinueMenuItem:(id)sender;
- (IBAction)onStopMenuItem:(id)sender;
- (IBAction)onHideMenuItem:(id)sender;
- (IBAction)onSendFeedbackMainMenuItem:(id)sender;
- (IBAction)onConsoleMenuItem:(id)sender;
@end
