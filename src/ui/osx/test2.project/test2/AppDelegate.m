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
#import "AboutWindowController.h"
#import "ErrorHandler.h"
#import "ModelChange.h"
#import "MenuItemTags.h"

@interface  AppDelegate()
@property (nonatomic,strong) IBOutlet MainWindowController *mainWindowController;
@property (nonatomic,strong) IBOutlet PreferencesWindowController *preferencesWindowController;
@property (nonatomic,strong) IBOutlet AboutWindowController *aboutWindowController;
@property TimeEntryViewItem *running_time_entry;
@property NSTimer *statusItemTimer;
@property NSString *lastKnownLoginState;
@property NSString *lastKnownTrackingState;
@end

@implementation AppDelegate

int blink = 0;

NSString *kTimeTotalUnknown = @"--:--";

- (void)applicationDidFinishLaunching:(NSNotification *)aNotification
{
  NSLog(@"applicationDidFinishLaunching");
  
  self.mainWindowController = [[MainWindowController alloc] initWithWindowNibName:@"MainWindowController"];
  [self.mainWindowController.window setReleasedWhenClosed:NO];
  [self.mainWindowController showWindow:self];
  [NSApp activateIgnoringOtherApps:YES];

  self.preferencesWindowController = [[PreferencesWindowController alloc] initWithWindowNibName:@"PreferencesWindowController"];

  self.aboutWindowController = [[AboutWindowController alloc] initWithWindowNibName:@"AboutWindowController"];
  
  [self createStatusItem];
  
  self.lastKnownLoginState = kUIStateUserLoggedIn;
  self.lastKnownTrackingState = kUIStateTimerStopped;

  [[NSNotificationCenter defaultCenter] addObserver:self
                                           selector:@selector(eventHandler:)
                                               name:kUIStateUserLoggedIn
                                             object:nil];
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
  [[NSNotificationCenter defaultCenter] addObserver:self
                                           selector:@selector(eventHandler:)
                                               name:kUICommandShowPreferences
                                             object:nil];
  [[NSNotificationCenter defaultCenter] addObserver:self
                                           selector:@selector(eventHandler:)
                                               name:kUIEventModelChange
                                             object:nil];
}

-(void)eventHandler: (NSNotification *) notification
{
  if ([notification.name isEqualToString:kUICommandShowPreferences]) {
    [self onPreferencesMenuItem:self];
    return;
  }
  
  if ([notification.name isEqualToString:kUIStateUserLoggedIn]) {
    self.lastKnownLoginState = kUIStateUserLoggedIn;

  } else if ([notification.name isEqualToString:kUIStateUserLoggedOut]) {
    self.lastKnownLoginState = kUIStateUserLoggedOut;
    self.lastKnownTrackingState = kUIStateTimerStopped;
    self.running_time_entry = nil;

  } else if ([notification.name isEqualToString:kUIStateTimerStopped]) {
    self.running_time_entry = nil;
    self.lastKnownTrackingState = kUIStateTimerStopped;

  } else if ([notification.name isEqualToString:kUIStateTimerRunning]) {
    self.running_time_entry = notification.object;
    self.lastKnownTrackingState = kUIStateTimerRunning;

  } else if ([notification.name isEqualToString:kUIEventModelChange]) {
    ModelChange *ch = notification.object;
    if (self.running_time_entry &&
        [self.running_time_entry.GUID isEqualToString:ch.GUID] &&
        [ch.ModelType isEqualToString:@"time_entry"] &&
        [ch.ChangeType isEqualToString:@"update"]) {
      // Time entry duration can be edited on server side and it's
      // pushed to us via websocket or pulled via regular sync.
      // When it happens, timer keeps on running, but the time should be
      // updated on status item:
      self.running_time_entry = [TimeEntryViewItem findByGUID:ch.GUID];
    }
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
  [menu addItemWithTitle:@"About" action:@selector(onAboutMenuItem:) keyEquivalent:@""];
  [menu addItem:[NSMenuItem separatorItem]];
  [menu addItemWithTitle:@"Show" action:@selector(onShowMenuItem) keyEquivalent:@""];
  [menu addItem:[NSMenuItem separatorItem]];
  [menu addItemWithTitle:@"New" action:@selector(onNewMenuItem) keyEquivalent:@""].tag = kMenuItemTagNew;
  [menu addItemWithTitle:@"Continue" action:@selector(onContinueMenuItem) keyEquivalent:@""].tag = kMenuItemTagContinue;
  [menu addItemWithTitle:@"Stop" action:@selector(onStopMenuItem) keyEquivalent:@""].tag = kMenuItemTagStop;
  [menu addItem:[NSMenuItem separatorItem]];
  [menu addItemWithTitle:@"Sync" action:@selector(onSyncMenuItem:) keyEquivalent:@""];
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
                                                        selector:@selector(statusItemTimerFired:)
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

- (IBAction)onSyncMenuItem:(id)sender {
  // FIXME: sync
}

- (IBAction)onHelpMenuItem:(id)sender {
  // FIXME: help
}

- (IBAction)onLogoutMenuItem:(id)sender {
  // FIXME: log out
}

- (IBAction)onClearCacheMenuItem:(id)sender {
  NSAlert *alert = [[NSAlert alloc] init];
  [alert addButtonWithTitle:@"OK"];
  [alert addButtonWithTitle:@"Cancel"];
  [alert setMessageText:@"Clear local data and log out?"];
  [alert setInformativeText:@"Deleted unsynced time entries cannot be restored."];
  [alert setAlertStyle:NSWarningAlertStyle];
  if ([alert runModal] != NSAlertFirstButtonReturn) {
    return;
  }
  
  char err[KOPSIK_ERR_LEN];
  kopsik_api_result res = kopsik_clear_cache(ctx, err, KOPSIK_ERR_LEN);
  if (KOPSIK_API_SUCCESS != res) {
    handle_error(res, err);
  }
}

- (IBAction)onAboutMenuItem:(id)sender {
  [self.aboutWindowController showWindow:self];
  [NSApp activateIgnoringOtherApps:YES];
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
  kopsik_timeline_stop(ctx);
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
  ctx = kopsik_context_init("osx_native_app", [version UTF8String]);

  [Bugsnag startBugsnagWithApiKey:@"2a46aa1157256f759053289f2d687c2f"];
  NSString* environment = [infoDict objectForKey:@"KopsikEnvironment"];
  NSAssert(environment != nil, @"Missing environment in plist");
  [Bugsnag configuration].releaseStage = environment;

  // Parse command line arguments
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

- (void)statusItemTimerFired:(NSTimer*)timer
{
  if (self.running_time_entry != nil) {
    const int duration_str_len = 10;
    char str[duration_str_len];
    if (blink) {
      blink = 0;
    } else {
      blink = 1;
    }
    kopsik_format_duration_in_seconds_hhmm(self.running_time_entry.duration_in_seconds,
                                           blink,
                                           str,
                                           duration_str_len);
    [self.statusItem setTitle:[NSString stringWithUTF8String:str]];
  }
}

- (BOOL)validateUserInterfaceItem:(id<NSValidatedUserInterfaceItem>)anItem {
    switch ([anItem tag]) {
      case kMenuItemTagNew:
        if (self.lastKnownLoginState != kUIStateUserLoggedIn) {
          return NO;
        }
        break;
      case kMenuItemTagContinue:
        if (self.lastKnownLoginState != kUIStateUserLoggedIn) {
          return NO;
        }
        if (self.lastKnownTrackingState != kUIStateTimerStopped) {
          return NO;
        }
        break;
      case kMenuItemTagStop:
        if (self.lastKnownLoginState != kUIStateUserLoggedIn) {
          return NO;
        }
        if (self.lastKnownTrackingState != kUIStateTimerRunning) {
          return NO;
        }
        break;
    }
    return YES;
}

@end
