//
//  AppDelegate.m
//  test2
//
//  Created by Alari on 9/15/13.
//  Copyright (c) 2013 kopsik developers. All rights reserved.
//

#import "AppDelegate.h"
#import "kopsik_api.h"
#import "Context.h"
#import "MainWindowController.h"
#import "PreferencesWindowController.h"
#import "Bugsnag.h"
#import "UIEvents.h"
#import "TimeEntryViewItem.h"

@interface  AppDelegate()
@property (nonatomic,strong) IBOutlet MainWindowController *mainWindowController;
@property (nonatomic,strong) IBOutlet PreferencesWindowController *preferencesWindowController;
@property TimeEntryViewItem *running_time_entry;
@property NSTimer *statusItemTimer;
@end

@implementation AppDelegate

NSString *kTimeTotalUnknown = @"--:--";

- (void)applicationDidFinishLaunching:(NSNotification *)aNotification
{
  NSLog(@"applicationDidFinishLaunching");

  [Bugsnag startBugsnagWithApiKey:@"2a46aa1157256f759053289f2d687c2f"];
  [Bugsnag configuration].releaseStage = @"development";
  
  self.mainWindowController = [[MainWindowController alloc] initWithWindowNibName:@"MainWindowController"];
  [self.mainWindowController.window setReleasedWhenClosed:NO];
  [self.mainWindowController showWindow:self];
  [NSApp activateIgnoringOtherApps:YES];

  self.preferencesWindowController = [[PreferencesWindowController alloc] initWithWindowNibName:@"PreferencesWindowController"];
  
  [self createStatusItem];
  
  [[NSNotificationCenter defaultCenter] addObserver:self
                                           selector:@selector(eventHandler:)
                                               name:kUIStateUserLoggedOut
                                             object:nil];
  [[NSNotificationCenter defaultCenter] addObserver:self
                                           selector:@selector(eventHandler:)
                                               name:kUIStateTimerRunning
                                             object:nil];
  [[NSNotificationCenter defaultCenter] addObserver:self
                                           selector:@selector(eventHandler:)
                                               name:kUIStateTimerStopped
                                             object:nil];
}

-(void)eventHandler: (NSNotification *) notification
{
  if ([notification.name isEqualToString:kUIStateUserLoggedOut]) {
    self.running_time_entry = nil;
  } else if ([notification.name isEqualToString:kUIStateTimerStopped]) {
    self.running_time_entry = nil;
  } else if ([notification.name isEqualToString:kUIStateTimerRunning]) {
    self.running_time_entry = notification.object;
  }
  if (self.running_time_entry == nil) {
    [self.statusItem setTitle: kTimeTotalUnknown];
    [self.statusItem setImage:self.offImage];
  } else {
    [self.statusItem setImage:self.onImage];
  }
}

- (void)createStatusItem {
  NSMenu *menu = [[NSMenu alloc] init];
  [menu addItemWithTitle:@"About" action:@selector(onAboutMenuItem) keyEquivalent:@""];
  [menu addItem:[NSMenuItem separatorItem]];
  [menu addItemWithTitle:@"Show" action:@selector(onShowMenuItem) keyEquivalent:@""];
  [menu addItem:[NSMenuItem separatorItem]];
  [menu addItemWithTitle:@"New" action:@selector(onNewMenuItem) keyEquivalent:@""];
  [menu addItemWithTitle:@"Continue" action:@selector(onContinueMenuItem) keyEquivalent:@""];
  [menu addItemWithTitle:@"Stop" action:@selector(onStopMenuItem) keyEquivalent:@""];
  [menu addItem:[NSMenuItem separatorItem]];
  [menu addItemWithTitle:@"Preferences" action:@selector(onPreferencesMenuItem:) keyEquivalent:@""];
  [menu addItem:[NSMenuItem separatorItem]];
  [menu addItemWithTitle:@"Quit" action:@selector(onQuitMenuItem) keyEquivalent:@""];
  
  NSStatusBar *bar = [NSStatusBar systemStatusBar];
  
  NSBundle *bundle = [NSBundle mainBundle];
  self.onImage = [[NSImage alloc] initWithContentsOfFile:[bundle pathForResource:@"on" ofType:@"png"]];
  self.offImage = [[NSImage alloc] initWithContentsOfFile:[bundle pathForResource:@"off" ofType:@"png"]];
  
  self.statusItem = [bar statusItemWithLength:NSVariableStatusItemLength];
  [self.statusItem setTitle: kTimeTotalUnknown];
  [self.statusItem setHighlightMode:YES];
  [self.statusItem setEnabled:YES];
  [self.statusItem setMenu:menu];
  [self.statusItem setImage:self.offImage];

  self.statusItemTimer = [NSTimer scheduledTimerWithTimeInterval:1.0
                                                          target:self
                                                        selector:@selector(timerFired:)
                                                        userInfo:nil
                                                         repeats:YES];
}

- (void)onNewMenuItem {
  [[NSNotificationCenter defaultCenter] postNotificationName:kUICommandNew
                                                      object:@"(no description)"];
}

- (void)onContinueMenuItem {
  [[NSNotificationCenter defaultCenter] postNotificationName:kUICommandContinue
                                                      object:nil];
}

- (void)onStopMenuItem {
  [[NSNotificationCenter defaultCenter] postNotificationName:kUICommandStop
                                                      object:nil];
}

- (void)onAboutMenuItem {
  [[NSApplication sharedApplication] orderFrontStandardAboutPanel:self];
}

- (void)onShowMenuItem {
  [self.mainWindowController showWindow:self];
  [NSApp activateIgnoringOtherApps:YES];
}

- (IBAction)onPreferencesMenuItem:(id)sender {
  [self.preferencesWindowController showWindow:self];
  [NSApp activateIgnoringOtherApps:YES];
}

- (void)onQuitMenuItem {
  [[NSApplication sharedApplication] terminate:self];
}

- (void)applicationWillTerminate:(NSNotification *)app
{
  NSLog(@"applicationWillTerminate");
  char err[KOPSIK_ERR_LEN];
  if (KOPSIK_API_SUCCESS != kopsik_websocket_stop(ctx, err, KOPSIK_ERR_LEN)) {
    NSLog(@"Error while shutting down websocket: %s", err);
  }
  NSLog(@"applicationWillTerminate done");
}

- (BOOL)applicationShouldHandleReopen:(NSApplication *)sender hasVisibleWindows:(BOOL)flag{
  [self.mainWindowController.window setIsVisible:YES];
  return YES;
}

- (NSString *)applicationSupportDirectory
{
  NSString *path;
  NSError *error;
  NSArray* paths = NSSearchPathForDirectoriesInDomains(NSApplicationSupportDirectory, NSUserDomainMask, YES);
  if ([paths count] == 0) {
    NSLog(@"Unable to access application support directory!");
  }
  path = [[paths objectAtIndex:0] stringByAppendingPathComponent:@"Kopsik"];
  
	if (![[NSFileManager defaultManager] fileExistsAtPath:path]){
		if (![[NSFileManager defaultManager] createDirectoryAtPath:path
                                   withIntermediateDirectories:NO
                                                    attributes:nil
                                                         error:&error]){
			NSLog(@"Create directory error: %@", error);
		}
	}
  return path;
}

- (id) init
{
  self = [super init];
  
  // Disallow duplicate instances
  if ([[NSRunningApplication runningApplicationsWithBundleIdentifier:[[NSBundle mainBundle] bundleIdentifier]] count] > 1) {
    [[NSAlert alertWithMessageText:[NSString stringWithFormat:@"Another copy of %@ is already running.", [[NSBundle mainBundle] objectForInfoDictionaryKey:(NSString *)kCFBundleNameKey]]
                     defaultButton:nil alternateButton:nil otherButton:nil informativeTextWithFormat:@"This copy will now quit."] runModal];
    
    [NSApp terminate:nil];
  }
  
  NSDictionary* infoDict = [[NSBundle mainBundle] infoDictionary];
  NSString* version = [infoDict objectForKey:@"CFBundleShortVersionString"];
  ctx = kopsik_context_init("osx", [version UTF8String]);

  NSArray *arguments = [[NSProcessInfo processInfo] arguments];
  NSLog(@"Command line arguments: %@", arguments);
  
  NSString *app_path = self.applicationSupportDirectory;
  NSString *db_path = [app_path stringByAppendingPathComponent:@"kopsik.db"];
  NSString *log_path = [app_path stringByAppendingPathComponent:@"kopsik.log"];
  NSString *log_level = @"debug";
  
  for (int i = 1; i < arguments.count; i++) {
    NSString *argument = [arguments objectAtIndex:i];
    if (([argument rangeOfString:@"log"].location != NSNotFound) &&
        ([argument rangeOfString:@"path"].location != NSNotFound)) {
      log_path = [arguments objectAtIndex:i+1];
      NSLog(@"log path overriden with '%@'", log_path);
      continue;
    }
    if (([argument rangeOfString:@"db"].location != NSNotFound) &&
        ([argument rangeOfString:@"path"].location != NSNotFound)) {
      db_path = [arguments objectAtIndex:i+1];
      NSLog(@"db path overriden with '%@'", db_path);
      continue;
    }
    if (([argument rangeOfString:@"log"].location != NSNotFound) &&
        ([argument rangeOfString:@"level"].location != NSNotFound)) {
      log_level = [arguments objectAtIndex:i+1];
      NSLog(@"log level overriden with '%@'", log_level);
      continue;
    }
    if (([argument rangeOfString:@"api"].location != NSNotFound) &&
        ([argument rangeOfString:@"url"].location != NSNotFound)) {
      NSString *url = [arguments objectAtIndex:i+1];
      kopsik_set_api_url(ctx, [url UTF8String]);
      NSLog(@"API URL overriden with '%@'", url);
      continue;
    }
    if (([argument rangeOfString:@"websocket"].location != NSNotFound) &&
        ([argument rangeOfString:@"url"].location != NSNotFound)) {
      NSString *url = [arguments objectAtIndex:i+1];
      kopsik_set_websocket_url(ctx, [url UTF8String]);
      NSLog(@"Websocket URL overriden with '%@'", url);
      continue;
    }
  }

  kopsik_set_db_path(ctx, [db_path UTF8String]);
  kopsik_set_log_path(ctx, [log_path UTF8String]);
  kopsik_set_log_level(ctx, [log_level UTF8String]);

  return self;
}

- (void) dealloc
{
  kopsik_context_clear(ctx);
  ctx = 0;
}

- (void)timerFired:(NSTimer*)timer
{
  if (self.running_time_entry != nil) {
    const int duration_str_len = 10;
    char str[duration_str_len];
    kopsik_format_duration_in_seconds_hhmm(self.running_time_entry.duration_in_seconds, str, duration_str_len);
    [self.statusItem setTitle:[NSString stringWithUTF8String:str]];
  }
}

@end
