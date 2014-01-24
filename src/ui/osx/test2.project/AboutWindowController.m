//
//  AboutWindowController.m
//  kopsik_ui_osx
//
//  Created by Tanel Lebedev on 29/10/2013.
//  Copyright (c) 2013 kopsik developers. All rights reserved.
//

#import "AboutWindowController.h"
#import "Context.h"
#import "ErrorHandler.h"
#import "Update.h"

@interface AboutWindowController ()
@property Update *update;
@end

@implementation AboutWindowController

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
    
  NSDictionary* infoDict = [[NSBundle mainBundle] infoDictionary];
  NSString* version = [infoDict objectForKey:@"CFBundleShortVersionString"];
  [self.versionTextField setStringValue:[NSString stringWithFormat:@"Version %@", version]];
  NSString* appname = [infoDict objectForKey:@"CFBundleName"];
  [self.appnameTextField setStringValue:appname];

  NSString *path = [[NSBundle mainBundle] pathForResource:@"Credits" ofType:@"rtf"];
  [self.creditsTextView readRTFDFromFile:path];
  
  [[NSNotificationCenter defaultCenter] addObserver:self
                                           selector:@selector(eventHandler:)
                                               name:kUIStateUpdateAvailable
                                             object:nil];
  [[NSNotificationCenter defaultCenter] addObserver:self
                                           selector:@selector(eventHandler:)
                                               name:kUIStateUpToDate
                                             object:nil];
}

- (IBAction)showWindow:(id)sender
{
  [self.checkForUpdateButton setTitle:@"Checking for update.."];
  kopsik_check_for_updates(ctx, about_updates_checked);
  [super showWindow: sender];
}

- (IBAction)checkForUpdateClicked:(id)sender {
  [[NSWorkspace sharedWorkspace] openURL:[NSURL URLWithString:self.update.URL]];
  [[NSApplication sharedApplication] terminate:self];
}

-(void)eventHandler: (NSNotification *) notification
{
  if ([notification.name isEqualToString:kUIStateUpToDate]) {
    self.update = nil;
    [self.checkForUpdateButton setEnabled:NO];
    [self.checkForUpdateButton setTitle:@"TogglDesktop is up to date."];
    return;
  }
  
  if ([notification.name isEqualToString:kUIStateUpdateAvailable]) {
    self.update = notification.object;
    [self.checkForUpdateButton setEnabled:YES];
    NSString *title = [NSString stringWithFormat:@"Click here to download update! (%@)", self.update.version];
    [self.checkForUpdateButton setTitle:title];
  }
}

void about_updates_checked(kopsik_api_result result,
                           const char *errmsg,
                           const int is_update_available,
                           const char *url,
                           const char *version) {
  if (result != KOPSIK_API_SUCCESS) {
    handle_error(result, errmsg);
    return;
  }
  
  if (!is_update_available) {
    [[NSNotificationCenter defaultCenter] postNotificationName:kUIStateUpToDate
                                                        object:nil];
    return;
  }
  Update *update = [[Update alloc] init];
  update.URL = [NSString stringWithUTF8String:url];
  update.version = [NSString stringWithUTF8String:version];
  [[NSNotificationCenter defaultCenter] postNotificationName:kUIStateUpdateAvailable
                                                      object:update];
}

@end
