//
//  AppDelegate.h
//  test2
//
//  Created by Alari on 9/15/13.
//  Copyright (c) 2013 TogglDesktop developers. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@interface AppDelegate : NSObject <NSApplicationDelegate>
@property (strong) NSStatusItem *statusItem;
@property (strong) NSImage* onImage;
@property (strong) NSImage* offImage;
@property (strong) NSImage* offlineOnImage;
@property (strong) NSImage* offlineOffImage;
@property (strong) NSImage* currentOnImage;
@property (strong) NSImage* currentOffImage;
- (IBAction)onPreferencesMenuItem:(id)sender;
- (IBAction)onAboutMenuItem:(id)sender;
- (IBAction)onSyncMenuItem:(id)sender;
- (IBAction)onShowMenuItem:(id)sender;
- (IBAction)onClearCacheMenuItem:(id)sender;
- (IBAction)onLogoutMenuItem:(id)sender;
- (IBAction)onHelpMenuItem:(id)sender;
- (IBAction)onNewMenuItem:(id)sender;
- (IBAction)onContinueMenuItem:(id)sender;
- (IBAction)onStopMenuItem:(id)sender;
- (IBAction)onHideMenuItem:(id)sender;
- (IBAction)onSendFeedbackMainMenuItem:(id)sender;
@end
