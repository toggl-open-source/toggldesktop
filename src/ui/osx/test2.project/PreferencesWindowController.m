//
//  PreferencesWindowController.m
//  Kopsik
//
//  Created by Tanel Lebedev on 22/10/2013.
//  Copyright (c) 2013 kopsik developers. All rights reserved.
//

#import "PreferencesWindowController.h"
#import "kopsik_api.h"
#import "ErrorHandler.h"
#import "Context.h"
#import "UIEvents.h"

@implementation PreferencesWindowController

- (void)windowDidLoad {
    [super windowDidLoad];

    NSNumberFormatter *formatter = [[NSNumberFormatter alloc] init];
    [formatter setUsesGroupingSeparator:NO];
    [formatter setNumberStyle:NSNumberFormatterDecimalStyle];
    [self.portTextField setFormatter:formatter];

    [self loadPreferences];
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

- (void)loadPreferences {
    NSLog(@"loadPreferences");
  
    KopsikSettings *settings = kopsik_settings_init();
    char err[KOPSIK_ERR_LEN];
    kopsik_api_result res = kopsik_get_settings(ctx,
                                             err,
                                             KOPSIK_ERR_LEN,
                                             settings);
    if (KOPSIK_API_SUCCESS != res) {
      kopsik_settings_clear(settings);
      handle_error(err);
      return;
    }
  
    if (settings->UseProxy) {
      [self.useProxyButton setState:NSOnState];
    } else {
      [self.useProxyButton setState:NSOffState];
    }
    if (settings->ProxyHost) {
      [self.hostTextField setStringValue:[NSString stringWithUTF8String:settings->ProxyHost]];
    } else {
      [self.hostTextField setStringValue:@""];
    }
    if (settings->ProxyPort) {
      [self.portTextField setIntegerValue:settings->ProxyPort];
    } else {
      [self.portTextField setStringValue:@""];
    }
    if (settings->ProxyUsername) {
      [self.usernameTextField setStringValue:[NSString stringWithUTF8String:settings->ProxyUsername]];
    } else {
      [self.usernameTextField setStringValue:@""];
    }
    if (settings->ProxyPassword) {
      [self.passwordTextField setStringValue:[NSString stringWithUTF8String:settings->ProxyPassword]];
    } else {
      [self.passwordTextField setStringValue:@""];
    }
    if (settings->UseIdleDetection) {
      [self.useIdleDetectionButton setState:NSOnState];
    } else {
      [self.useIdleDetectionButton setState:NSOffState];
    }
    kopsik_settings_clear(settings);
}

- (IBAction)useProxyButtonChanged:(id)sender {
  [self enableProxyFields];
  [self savePreferences];
}

- (IBAction)useIdleDetectionButtonChanged:(id)sender {
  [self savePreferences];
}

- (void)enableProxyFields {
  bool use_proxy = [self.useProxyButton state] == NSOnState;
  [self.hostTextField setEnabled:use_proxy];
  [self.portTextField setEnabled:use_proxy];
  [self.usernameTextField setEnabled:use_proxy];
  [self.passwordTextField setEnabled:use_proxy];
}

- (IBAction)hostTextFieldChanged:(id)sender {
  [self savePreferences];
}

- (IBAction)portTextFieldChanged:(id)sender {
  [self savePreferences];
}

- (IBAction)usernameTextFieldChanged:(id)sender {
  [self savePreferences];
}

- (IBAction)passwordTextFieldChanged:(id)sender {
  [self savePreferences];
}

- (void)savePreferences {
  NSLog(@"savePreferences");

  int use_proxy = 0;
  if ([self.useProxyButton state] == NSOnState) {
    use_proxy = 1;
  }
  NSString *host = [self.hostTextField stringValue];
  NSInteger port = [self.portTextField integerValue];
  NSString *username = [self.usernameTextField stringValue];
  NSString *password = [self.passwordTextField stringValue];

  int use_idle_detection = 0;
  if ([self.useIdleDetectionButton state] == NSOnState) {
    use_idle_detection = 1;
  }

  char err[KOPSIK_ERR_LEN];
  kopsik_api_result res = kopsik_set_settings(ctx,
                                              err,
                                              KOPSIK_ERR_LEN,
                                              use_proxy,
                                              [host UTF8String],
                                              (unsigned int)port,
                                              [username UTF8String],
                                              [password UTF8String],
                                              use_idle_detection);
  if (KOPSIK_API_SUCCESS != res) {
    handle_error(err);
    return;
  }
  [[NSNotificationCenter defaultCenter] postNotificationName:kUIEventSettingsChanged
                                                      object:nil];
}

- (IBAction)recordTimelineCheckboxChanged:(id)sender {
  kopsik_timeline_toggle_recording(ctx);
  [self displayTimelineRecordingState];
}

- (void)displayTimelineRecordingState {
  NSCellStateValue state = NSOffState;
  if (kopsik_timeline_is_recording_enabled(ctx)) {
    state = NSOnState;
  }
  [self.recordTimelineCheckbox setState:state];
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
