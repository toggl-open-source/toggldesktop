//
//  AboutWindowController.m
//  kopsik_ui_osx
//
//  Created by Tanel Lebedev on 29/10/2013.
//  Copyright (c) 2013 TogglDesktop developers. All rights reserved.
//

#import "AboutWindowController.h"
#import "Update.h"
#import "UIEvents.h"
#import "kopsik_api.h"

@interface AboutWindowController ()
@property Update *update;
@end

@implementation AboutWindowController

extern void *ctx;

- (void)windowDidLoad {
  [super windowDidLoad];
    
  NSDictionary* infoDict = [[NSBundle mainBundle] infoDictionary];
  NSString* version = [infoDict objectForKey:@"CFBundleShortVersionString"];
  [self.versionTextField setStringValue:[NSString stringWithFormat:@"Version %@", version]];
  NSString* appname = [infoDict objectForKey:@"CFBundleName"];
  [self.appnameTextField setStringValue:appname];

  NSString *path = [[NSBundle mainBundle] pathForResource:@"Credits" ofType:@"rtf"];
  [self.creditsTextView readRTFDFromFile:path];
  
  [[NSNotificationCenter defaultCenter] addObserver:self
                                           selector:@selector(startDisplayUpdate:)
                                               name:kDisplayUpdate
                                             object:nil];

  [self showUpdateChannel];
}

- (void)showUpdateChannel {
  unsigned int update_channel_len = 10;
  char update_channel[update_channel_len];
  if (!kopsik_get_update_channel(ctx,
                                 update_channel,
                                 update_channel_len)) {
    return;
  }
  self.updateChannelComboBox.stringValue = [NSString stringWithUTF8String:update_channel];
}

- (IBAction)updateChannelSelected:(id)sender {
  NSString *updateChannel = self.updateChannelComboBox.stringValue;
  if (!kopsik_set_update_channel(ctx,
                                 [updateChannel UTF8String])) {
    return;
  }
  
  [self check];
}

- (void)check {
  [self.checkForUpdateButton setEnabled:NO];
  [self.updateChannelComboBox setEnabled:NO];
  [self.checkForUpdateButton setTitle:@"Checking for update.."];
  kopsik_check_for_updates(ctx);
}

- (IBAction)showWindow:(id)sender {
  [self check];
  [super showWindow: sender];
}

- (IBAction)checkForUpdateClicked:(id)sender {
  [[NSWorkspace sharedWorkspace] openURL:[NSURL URLWithString:self.update.URL]];
  [[NSApplication sharedApplication] terminate:self];
}

-(void)startDisplayUpdate:(NSNotification *)notification {
  [self performSelectorOnMainThread:@selector(displayUpdate:)
                         withObject:notification.object
                      waitUntilDone:NO];
}

-(void)displayUpdate:(Update *)update {
  NSAssert([NSThread isMainThread], @"Rendering stuff should happen on main thread");
  
  if (!update) {
    self.update = nil;
    [self.checkForUpdateButton setEnabled:NO];
    [self.updateChannelComboBox setEnabled:YES];
    [self.checkForUpdateButton setTitle:@"TogglDesktop is up to date."];
    return;
  }
  
  self.update = update;
  [self.checkForUpdateButton setEnabled:YES];
  [self.updateChannelComboBox setEnabled:YES];

  NSString *title = [NSString stringWithFormat:@"Click here to download update! (%@)", self.update.version];
    [self.checkForUpdateButton setTitle:title];
}

@end
