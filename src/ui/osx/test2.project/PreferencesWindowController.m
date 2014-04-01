//
//  PreferencesWindowController.m
//  Kopsik
//
//  Created by Tanel Lebedev on 22/10/2013.
//  Copyright (c) 2013 TogglDesktop developers. All rights reserved.
//

#import "PreferencesWindowController.h"
#import "kopsik_api.h"
#import "ErrorHandler.h"
#import "Context.h"
#import "UIEvents.h"
#import "MASShortcutView+UserDefaults.h"

NSString *const kPreferenceGlobalShortcutShowHide = @"TogglDesktopGlobalShortcutShowHide";
NSString *const kPreferenceGlobalShortcutStartStop = @"TogglDesktopGlobalShortcutStartStop";

@implementation PreferencesWindowController

- (void)windowDidLoad {
    [super windowDidLoad];

    self.showHideShortcutView.associatedUserDefaultsKey = kPreferenceGlobalShortcutShowHide;
    self.startStopShortcutView.associatedUserDefaultsKey = kPreferenceGlobalShortcutStartStop;

    NSNumberFormatter *formatter = [[NSNumberFormatter alloc] init];
    [formatter setUsesGroupingSeparator:NO];
    [formatter setNumberStyle:NSNumberFormatterDecimalStyle];
    [self.portTextField setFormatter:formatter];

    [self loadSettings];
    [self loadProxySettings];

    [self enableProxyFields];
  
    [self displayTimelineRecordingState];

    [[NSNotificationCenter defaultCenter] addObserver:self
                                             selector:@selector(eventHandler:)
                                                 name:kUIStateUserLoggedIn
                                               object:nil];
    [[NSNotificationCenter defaultCenter] addObserver:self
                                             selector:@selector(eventHandler:)
                                                 name:kUIStateUserLoggedOut
                                               object:nil];
}

- (NSInteger)boolToState:(BOOL)value {
  if (value) {
    return NSOnState;
  }
  return NSOffState;
}

- (void)loadSettings {
  _Bool idle_detection = false;
  _Bool menubar_timer = false;
  _Bool dock_icon = false;

  if (!kopsik_get_settings(ctx,
                           &idle_detection,
                           &menubar_timer,
                           &dock_icon)) {
    return;
  }
  
  [self.useIdleDetectionButton setState:[self boolToState:idle_detection]];
  [self.menubarTimerCheckbox setState:[self boolToState:menubar_timer]];
  [self.dockIconCheckbox setState:[self boolToState:dock_icon]];

}

- (void)loadProxySettings {
  _Bool use_proxy = false;
  char *proxy_host = 0;
  unsigned int proxy_port = 0;
  char *proxy_username = 0;
  char *proxy_password = 0;

  if (!kopsik_get_proxy_settings(ctx,
                                 &use_proxy,
                                 &proxy_host,
                                 &proxy_port,
                                 &proxy_username,
                                 &proxy_password)) {
    return;
  }
  
  [self.useProxyButton setState:[self boolToState:use_proxy]];

  if (proxy_host) {
    [self.hostTextField setStringValue:[NSString stringWithUTF8String:proxy_host]];
  } else {
    [self.hostTextField setStringValue:@""];
  }

  if (proxy_port) {
    [self.portTextField setIntegerValue:proxy_port];
  } else {
    [self.portTextField setStringValue:@""];
  }

  if (proxy_username) {
    [self.usernameTextField setStringValue:[NSString stringWithUTF8String:proxy_username]];
  } else {
    [self.usernameTextField setStringValue:@""];
  }

  if (proxy_password) {
    [self.passwordTextField setStringValue:[NSString stringWithUTF8String:proxy_password]];
  } else {
    [self.passwordTextField setStringValue:@""];
  }

  if (proxy_host) {
    free(proxy_host);
  }
  if (proxy_username) {
    free(proxy_username);
  }
  if (proxy_password) {
    free(proxy_password);
  }
}

- (IBAction)useProxyButtonChanged:(id)sender {
  [self enableProxyFields];

  [self saveProxySettings];
}

- (IBAction)useIdleDetectionButtonChanged:(id)sender {
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

  if (!kopsik_set_settings(ctx,
                           [self stateToBool:[self.useIdleDetectionButton state]],
                           [self stateToBool:[self.menubarTimerCheckbox state]],
                           [self stateToBool:[self.dockIconCheckbox state]])) {
    return;
  }
  [[NSNotificationCenter defaultCenter] postNotificationName:kUIEventSettingsChanged
                                                      object:nil];
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
  
  if (!kopsik_set_proxy_settings(ctx,
                                 use_proxy,
                                 [host UTF8String],
                                 (unsigned int)port,
                                 [username UTF8String],
                                 [password UTF8String])) {
    return;
  }
  [[NSNotificationCenter defaultCenter] postNotificationName:kUIEventSettingsChanged
                                                      object:nil];
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

- (void)eventHandler: (NSNotification *) notification {
  if ([notification.name isEqualToString:kUIStateUserLoggedIn]) {
    [self performSelectorOnMainThread:@selector(enableTimelineSettings)
                           withObject:nil
                        waitUntilDone:NO];
    return;
  }

  if ([notification.name isEqualToString:kUIStateUserLoggedOut]) {
    [self performSelectorOnMainThread:@selector(disableTimelineSettings)
                           withObject:nil
                        waitUntilDone:NO];
    return;
  }
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
