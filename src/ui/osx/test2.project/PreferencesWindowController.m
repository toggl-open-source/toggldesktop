//
//  PreferencesWindowController.m
//  Kopsik
//
//  Created by Tanel Lebedev on 22/10/2013.
//  Copyright (c) 2013 TogglDesktop developers. All rights reserved.
//

#import "PreferencesWindowController.h"
#import "kopsik_api.h"
#import "UIEvents.h"
#import "Settings.h"
#import "ProxySettings.h"
#import "MASShortcutView+UserDefaults.h"
#import "DisplayCommand.h"

NSString *const kPreferenceGlobalShortcutShowHide = @"TogglDesktopGlobalShortcutShowHide";
NSString *const kPreferenceGlobalShortcutStartStop = @"TogglDesktopGlobalShortcutStartStop";

@implementation PreferencesWindowController

extern void *ctx;

- (void)windowDidLoad {
    [super windowDidLoad];

    self.showHideShortcutView.associatedUserDefaultsKey = kPreferenceGlobalShortcutShowHide;
    self.startStopShortcutView.associatedUserDefaultsKey = kPreferenceGlobalShortcutStartStop;

    NSNumberFormatter *formatter = [[NSNumberFormatter alloc] init];
    [formatter setUsesGroupingSeparator:NO];
    [formatter setNumberStyle:NSNumberFormatterDecimalStyle];
    [self.portTextField setFormatter:formatter];

    [self enableProxyFields];
  
    [self displayTimelineRecordingState];

    [[NSNotificationCenter defaultCenter] addObserver:self
                                             selector:@selector(startDisplaySettings:)
                                                 name:kDisplaySettings
                                               object:nil];
}

- (NSInteger)boolToState:(BOOL)value {
  if (value) {
    return NSOnState;
  }
  return NSOffState;
}

- (void)displayProxySettings:(ProxySettings *)proxySettings {
  NSAssert([NSThread isMainThread], @"Rendering stuff should happen on main thread");

  [self.useProxyButton setState:[self boolToState:proxySettings.use_proxy]];

  if (proxySettings.proxy_host) {
    [self.hostTextField setStringValue:proxySettings.proxy_host];
  } else {
    [self.hostTextField setStringValue:@""];
  }

  if (proxySettings.proxy_port) {
    [self.portTextField setIntegerValue:proxySettings.proxy_port];
  } else {
    [self.portTextField setStringValue:@""];
  }

  if (proxySettings.proxy_username) {
    [self.usernameTextField setStringValue:proxySettings.proxy_username];
  } else {
    [self.usernameTextField setStringValue:@""];
  }

  if (proxySettings.proxy_password) {
    [self.passwordTextField setStringValue:proxySettings.proxy_password];
  } else {
    [self.passwordTextField setStringValue:@""];
  }
}

- (IBAction)useProxyButtonChanged:(id)sender {
  [self enableProxyFields];

  [self saveProxySettings];
}

- (IBAction)useIdleDetectionButtonChanged:(id)sender {
  [self saveSettings];
}

- (IBAction)ontopCheckboxChanged:(id)sender {
  [self saveSettings];  
}

- (IBAction)reminderCheckboxChanged:(id)sender {
  [self saveSettings];
}

- (void)enableProxyFields {
  bool use_proxy = [self.useProxyButton state] == NSOnState;
  [self.hostTextField setEnabled:use_proxy];
  [self.portTextField setEnabled:use_proxy];
  [self.usernameTextField setEnabled:use_proxy];
  [self.passwordTextField setEnabled:use_proxy];
}

- (IBAction)hostTextFieldChanged:(id)sender {
  [self saveProxySettings];
}

- (IBAction)portTextFieldChanged:(id)sender {
  [self saveProxySettings];
}

- (IBAction)usernameTextFieldChanged:(id)sender {
  [self saveProxySettings];
}

- (IBAction)passwordTextFieldChanged:(id)sender {
  [self saveProxySettings];
}

- (unsigned int)stateToBool:(NSInteger)state {
  if (NSOnState == state) {
    return 1;
  }
  return 0;
}

- (void)saveSettings {
  NSLog(@"saveSettings");

  kopsik_set_settings(ctx,
                      [self stateToBool:[self.useIdleDetectionButton state]],
                      [self stateToBool:[self.menubarTimerCheckbox state]],
                      [self stateToBool:[self.dockIconCheckbox state]],
                      [self stateToBool:[self.ontopCheckbox state]],
                      [self stateToBool:[self.reminderCheckbox state]]);
}

- (void)saveProxySettings {
  NSLog(@"saveProxySettings");
  
  _Bool use_proxy = false;
  if ([self.useProxyButton state] == NSOnState) {
    use_proxy = true;
  }
  NSString *host = [self.hostTextField stringValue];
  NSInteger port = [self.portTextField integerValue];
  NSString *username = [self.usernameTextField stringValue];
  NSString *password = [self.passwordTextField stringValue];
  
  kopsik_set_proxy_settings(ctx,
                            use_proxy,
                            [host UTF8String],
                            (unsigned int)port,
                            [username UTF8String],
                            [password UTF8String]);
}

- (IBAction)recordTimelineCheckboxChanged:(id)sender {
  kopsik_timeline_toggle_recording(ctx);
  [self displayTimelineRecordingState];
}

- (IBAction)menubarTimerCheckboxChanged:(id)sender {
  [self saveSettings];
}

- (IBAction)dockIconCheckboxChanged:(id)sender {
  [self saveSettings];
}

- (void)displayTimelineRecordingState {
  NSCellStateValue state = NSOffState;
  if (kopsik_timeline_is_recording_enabled(ctx)) {
    state = NSOnState;
  }
  [self.recordTimelineCheckbox setState:state];

  _Bool logged_in = false;
  if (!kopsik_user_is_logged_in(ctx, &logged_in)) {
    return;
  }
  if (logged_in) {
    [self enableTimelineSettings];
    return;
  }
  [self disableTimelineSettings];
}

- (void)startDisplaySettings:(NSNotification *)notification {
  [self performSelectorOnMainThread:@selector(displaySettings:)
                           withObject:notification.object
                        waitUntilDone:NO];
}

- (void)displaySettings:(DisplayCommand *)cmd {
  NSAssert([NSThread isMainThread], @"Rendering stuff should happen on main thread");

  Settings *settings = cmd.settings;
  
  [self.useIdleDetectionButton setState:[self boolToState:settings.idle_detection]];
  [self.menubarTimerCheckbox setState:[self boolToState:settings.menubar_timer]];
  [self.dockIconCheckbox setState:[self boolToState:settings.dock_icon]];
  [self.ontopCheckbox setState:[self boolToState:settings.on_top]];
  [self.reminderCheckbox setState:[self boolToState:settings.reminder]];
}

- (void)enableTimelineSettings {
  NSAssert([NSThread isMainThread], @"Rendering stuff should happen on main thread");
  [self.recordTimelineCheckbox setEnabled:YES];
}

- (void)disableTimelineSettings {
  NSAssert([NSThread isMainThread], @"Rendering stuff should happen on main thread");
  [self.recordTimelineCheckbox setEnabled:NO];
}

@end
