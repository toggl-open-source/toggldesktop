//
//  AppDelegate.h
//  test2
//
//  Created by Alari on 9/15/13.
//  Copyright (c) 2013 TogglTrack developers. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@class AutocompleteItem;
@class Reachability;
@class SystemMessagePayload;

@protocol OIDExternalUserAgentSession;

@interface AppDelegate : NSObject <NSApplicationDelegate, NSUserNotificationCenterDelegate>
@property (strong, nonatomic) id<OIDExternalUserAgentSession> currentAuthorizationFlow;
@property (strong, nonatomic) NSStatusItem *statusItem;
@property (strong, nonatomic) Reachability *reach;

/// Message to show to a user after successful login. Message is shown as a small information toast.
@property (strong, nonatomic) SystemMessagePayload *afterLoginMessage;

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
- (NSString *)currentChannel;
@end
