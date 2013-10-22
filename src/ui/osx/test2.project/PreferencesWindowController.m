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

@interface PreferencesWindowController ()

@end

@implementation PreferencesWindowController

- (id)initWithWindow:(NSWindow *)window
{
    self = [super initWithWindow:window];
    if (self) {
        // Initialization code here.
    }
    return self;
}

- (void)windowDidLoad
{
    [super windowDidLoad];
    [self loadProxy];
    [self enableProxyFields];
}

- (void)windowWillClose:(NSNotification *)notification {
    [self saveProxy];
}

- (void)loadProxy {
    NSLog(@"loadProxy");
  
    KopsikProxySettings *settings = kopsik_proxy_settings_init();
    char err[KOPSIK_ERR_LEN];
    kopsik_api_result res = kopsik_get_proxy(ctx,
                                             err,
                                             KOPSIK_ERR_LEN,
                                             settings);
    if (KOPSIK_API_SUCCESS != res) {
      kopsik_proxy_settings_clear(settings);
      handle_error(res, err);
      return;
    }
  
    if (settings->UseProxy) {
      [self.useProxyButton setState:NSOnState];
    } else {
      [self.useProxyButton setState:NSOffState];
    }
    if (settings->Host) {
      [self.hostTextField setStringValue:[NSString stringWithUTF8String:settings->Host]];
    } else {
      [self.hostTextField setStringValue:@""];
    }
    if (settings->Port) {
      [self.portTextField setIntegerValue:settings->Port];
    } else {
      [self.portTextField setStringValue:@""];
    }
    if (settings->Username) {
      [self.usernameTextField setStringValue:[NSString stringWithUTF8String:settings->Username]];
    } else {
      [self.usernameTextField setStringValue:@""];
    }
    if (settings->Password) {
      [self.passwordTextField setStringValue:[NSString stringWithUTF8String:settings->Password]];
    } else {
      [self.passwordTextField setStringValue:@""];
    }
    kopsik_proxy_settings_clear(settings);
}

- (IBAction)useProxyButtonChanged:(id)sender {
  [self enableProxyFields];
  [self saveProxy];
}

- (void)enableProxyFields {
  bool use_proxy = [self.useProxyButton state] == NSOnState;
  [self.hostTextField setEnabled:use_proxy];
  [self.portTextField setEnabled:use_proxy];
  [self.usernameTextField setEnabled:use_proxy];
  [self.passwordTextField setEnabled:use_proxy];
}

- (IBAction)hostTextFieldChanged:(id)sender {
  [self saveProxy];
}

- (IBAction)portTextFieldChanged:(id)sender {
  [self saveProxy];
}

- (IBAction)usernameTextFieldChanged:(id)sender {
  [self saveProxy];
}

- (IBAction)passwordTextFieldChanged:(id)sender {
  [self saveProxy];
}

- (void)saveProxy {
  NSLog(@"saveProxy");
  int use_proxy = 0;
  if ([self.useProxyButton state] == NSOnState) {
    use_proxy = 1;
  }
  NSString *host = [self.hostTextField stringValue];
  NSInteger port = [self.portTextField integerValue];
  NSString *username = [self.usernameTextField stringValue];
  NSString *password = [self.passwordTextField stringValue];
  
  char err[KOPSIK_ERR_LEN];
  kopsik_api_result res = kopsik_set_proxy(ctx,
                                           err,
                                           KOPSIK_ERR_LEN,
                                           use_proxy,
                                           [host UTF8String],
                                           (unsigned int)port,
                                           [username UTF8String],
                                           [password UTF8String]);
  handle_error(res, err);
}
@end
