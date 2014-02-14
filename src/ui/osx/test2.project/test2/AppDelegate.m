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
#import "User.h"
#import "Update.h"
#import "idler.h"
#import "IdleEvent.h"
#import "IdleNotificationWindowController.h"
#import "CrashReporter.h"
#import "FeedbackWindowController.h"

@interface AppDelegate()
@property (nonatomic, strong) IBOutlet MainWindowController *
  mainWindowController;
@property (nonatomic, strong) IBOutlet PreferencesWindowController *
  preferencesWindowController;
@property (nonatomic, strong) IBOutlet AboutWindowController *
  aboutWindowController;
@property (nonatomic, strong) IBOutlet IdleNotificationWindowController *
  idleNotificationWindowController;
@property (nonatomic, strong) IBOutlet FeedbackWindowController *
  feedbackWindowController;
@property TimeEntryViewItem *lastKnownRunningTimeEntry;
@property NSTimer *statusItemTimer;
@property NSTimer *idleTimer;
@property NSString *lastKnownLoginState;
@property NSString *lastKnownTrackingState;
@property int lastIdleSecondsReading;
@property NSDate *lastIdleStarted;

// we'll be updating running TE as a menu item, too
@property (strong) IBOutlet NSMenuItem *runningTimeEntryMenuItem;

// Where logs are written and db is kept
@property NSString *app_path;
@property NSString *db_path;
@property NSString *log_path;
@property NSString *log_level;

// Environment (development, production, etc)
@property NSString *environment;

// Websocket and API hosts can be overriden
@property NSString *websocket_url_override;
@property NSString *api_url_override;

// For testing crash reporter
@property BOOL forceCrash;
@end

@implementation AppDelegate

int blink = 0;

NSString *kTimeTotalUnknown = @" --:--";

- (void)applicationDidFinishLaunching:(NSNotification *)aNotification {
  NSLog(@"applicationDidFinishLaunching");

  if (![self.environment isEqualToString:@"production"]) {
    // Turn on UI constraint debugging, if not in production
    [[NSUserDefaults standardUserDefaults] setBool:YES
                                            forKey:@"NSConstraintBasedLayoutVisualizeMutuallyExclusiveConstraints"];
  }
  
  self.mainWindowController =
    [[MainWindowController alloc]
      initWithWindowNibName:@"MainWindowController"];
  [self.mainWindowController.window setReleasedWhenClosed:NO];
  
  PLCrashReporter *crashReporter = [PLCrashReporter sharedReporter];
  NSError *error;
  
  // Check if we previously crashed
  if ([crashReporter hasPendingCrashReport]) {
    [self handleCrashReport];
  }
  
  // Enable the Crash Reporter
  if (![crashReporter enableCrashReporterAndReturnError: &error]) {
    NSLog(@"Warning: Could not enable crash reporter: %@", error);
  }
  
  if (self.forceCrash) {
    abort();
  }
  
  [self onShowMenuItem];

  self.preferencesWindowController =
    [[PreferencesWindowController alloc]
      initWithWindowNibName:@"PreferencesWindowController"];

  self.aboutWindowController =
    [[AboutWindowController alloc]
      initWithWindowNibName:@"AboutWindowController"];

  self.idleNotificationWindowController =
    [[IdleNotificationWindowController alloc]
      initWithWindowNibName:@"IdleNotificationWindowController"];
  
  self.feedbackWindowController =
    [[FeedbackWindowController alloc]
      initWithWindowNibName:@"FeedbackWindowController"];

  [self createStatusItem];
  
  self.lastKnownLoginState = kUIStateUserLoggedOut;
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
  [[NSNotificationCenter defaultCenter] addObserver:self
                                           selector:@selector(eventHandler:)
                                               name:kUICommandSplitAt
                                             object:nil];
  [[NSNotificationCenter defaultCenter] addObserver:self
                                           selector:@selector(eventHandler:)
                                               name:kUICommandStopAt
                                             object:nil];
  [[NSNotificationCenter defaultCenter] addObserver:self
                                           selector:@selector(eventHandler:)
                                               name:kUICommandStop
                                             object:nil];
  [[NSNotificationCenter defaultCenter] addObserver:self
                                           selector:@selector(eventHandler:)
                                               name:kUICommandNew
                                             object:nil];
  [[NSNotificationCenter defaultCenter] addObserver:self
                                           selector:@selector(eventHandler:)
                                               name:kUICommandContinue
                                             object:nil];
  [[NSNotificationCenter defaultCenter] addObserver:self
                                           selector:@selector(eventHandler:)
                                               name:kUIEventSettingsChanged
                                             object:nil];

  kopsik_set_change_callback(ctx, on_model_change);
  
  char err[KOPSIK_ERR_LEN];
  KopsikUser *user = kopsik_user_init();
  if (KOPSIK_API_SUCCESS != kopsik_current_user(ctx, err, KOPSIK_ERR_LEN, user)) {
    [[NSNotificationCenter defaultCenter] 
      postNotificationName:kUIStateError
      object:[NSString stringWithUTF8String:err]];
    kopsik_user_clear(user);
    return;
  }

  User *userinfo = nil;
  if (user->ID) {
    userinfo = [[User alloc] init];
    [userinfo load:user];
  }
  kopsik_user_clear(user);
  
  if (userinfo == nil) {
    [[NSNotificationCenter defaultCenter]
      postNotificationName:kUIStateUserLoggedOut object:nil];
  } else {
    [[NSNotificationCenter defaultCenter]
      postNotificationName:kUIStateUserLoggedIn object:userinfo];
  }
 
  NSDictionary* infoDict = [[NSBundle mainBundle] infoDictionary];
  NSNumber* checkEnabled = infoDict[@"KopsikCheckForUpdates"];
  if ([checkEnabled boolValue]) {
    [self checkForUpdates];
  }
}

- (void)startWebSocket {
  NSLog(@"startWebSocket");
  kopsik_websocket_switch(ctx, 1);
}

- (void)stopWebSocket {
  NSLog(@"stopWebSocket");
  kopsik_websocket_switch(ctx, 0);
}

- (void)startTimeline {
  NSLog(@"startTimeline");
  kopsik_timeline_switch(ctx, handle_error, 1);
}

- (void)stopTimeline {
  NSLog(@"stopTimeline");
  kopsik_timeline_switch(ctx, handle_error, 0);
}

- (void)startNewTimeEntry:(TimeEntryViewItem *)new_time_entry {
  char err[KOPSIK_ERR_LEN];
  KopsikTimeEntryViewItem *item = kopsik_time_entry_view_item_init();
  NSAssert(new_time_entry != nil, @"new time entry details cannot be nil");
  kopsik_api_result res = kopsik_start(ctx,
                                       err,
                                       KOPSIK_ERR_LEN,
                                       [new_time_entry.Description UTF8String],
                                       [new_time_entry.duration UTF8String],
                                       new_time_entry.TaskID,
                                       new_time_entry.ProjectID,
                                       item);
  if (KOPSIK_API_SUCCESS != res) {
    kopsik_time_entry_view_item_clear(item);
    handle_error(res, err);
    return;
  }

  TimeEntryViewItem *timeEntry = [[TimeEntryViewItem alloc] init];
  [timeEntry load:item];
  kopsik_time_entry_view_item_clear(item);
  if (timeEntry.duration_in_seconds < 0) {
    [[NSNotificationCenter defaultCenter]
     postNotificationName:kUIStateTimerRunning object:timeEntry];
  }

  kopsik_sync(ctx, 0, handle_error);
}

- (void)continueTimeEntry:(NSString *)guid {
  char err[KOPSIK_ERR_LEN];
  KopsikTimeEntryViewItem *item = kopsik_time_entry_view_item_init();
  kopsik_api_result res = 0;
  int was_found = 0;
  if (guid == nil) {
    res = kopsik_continue_latest(ctx, err, KOPSIK_ERR_LEN, item, &was_found);
  } else {
    was_found = 1;
    res = kopsik_continue(ctx, err, KOPSIK_ERR_LEN, [guid UTF8String], item);
  }

  if (res != KOPSIK_API_SUCCESS) {
    kopsik_time_entry_view_item_clear(item);
    [[NSNotificationCenter defaultCenter]
      postNotificationName:kUIStateError
      object:[NSString stringWithUTF8String:err]];
    return;
  }
  
  if (!was_found) {
    kopsik_time_entry_view_item_clear(item);
    return;
  }
  
  TimeEntryViewItem *timeEntry = [[TimeEntryViewItem alloc] init];
  [timeEntry load:item];
  kopsik_time_entry_view_item_clear(item);
  
  [[NSNotificationCenter defaultCenter]
    postNotificationName:kUIStateTimerRunning object:timeEntry];
  
  kopsik_sync(ctx, 0, handle_error);
}

- (void)stopTimeEntry {
  char err[KOPSIK_ERR_LEN];
  KopsikTimeEntryViewItem *item = kopsik_time_entry_view_item_init();
  int was_found = 0;
  if (KOPSIK_API_SUCCESS != kopsik_stop(
      ctx, err, KOPSIK_ERR_LEN, item, &was_found)) {
    kopsik_time_entry_view_item_clear(item);
    [[NSNotificationCenter defaultCenter]
      postNotificationName:kUIStateError
      object:[NSString stringWithUTF8String:err]];
    return;
  }
  
  if (!was_found) {
    kopsik_time_entry_view_item_clear(item);
    return;
  }

  TimeEntryViewItem *te = [[TimeEntryViewItem alloc] init];
  [te load:item];
  kopsik_time_entry_view_item_clear(item);
  [[NSNotificationCenter defaultCenter]
    postNotificationName:kUIStateTimerStopped object:te];
  
  kopsik_sync(ctx, 0, handle_error);
}

- (void)splitTimeEntryAfterIdle:(IdleEvent *)idleEvent {
  NSLog(@"Idle event: %@", idleEvent);
  NSAssert(idleEvent != nil, @"idle event cannot be nil");
  char err[KOPSIK_ERR_LEN];
  KopsikTimeEntryViewItem *item = kopsik_time_entry_view_item_init();
  int was_found = 0;
  NSTimeInterval startedAt = [idleEvent.started timeIntervalSince1970];
  NSLog(@"Time entry split at %f", startedAt);
  kopsik_api_result res = kopsik_split_running_time_entry_at(ctx,
                                                             err,
                                                             KOPSIK_ERR_LEN,
                                                             startedAt,
                                                             item,
                                                             &was_found);
  if (KOPSIK_API_SUCCESS != res) {
    kopsik_time_entry_view_item_clear(item);
    handle_error(res, err);
    return;
  }
  
  if (was_found) {
    TimeEntryViewItem *timeEntry = [[TimeEntryViewItem alloc] init];
    [timeEntry load:item];
    [[NSNotificationCenter defaultCenter]
      postNotificationName:kUIStateTimerRunning
      object:timeEntry];
  }

  kopsik_time_entry_view_item_clear(item);

  kopsik_sync(ctx, 0, handle_error);
}

- (void)stopTimeEntryAfterIdle:(IdleEvent *)idleEvent {
  NSAssert(idleEvent != nil, @"idle event cannot be nil");
  NSLog(@"Idle event: %@", idleEvent);
  char err[KOPSIK_ERR_LEN];
  KopsikTimeEntryViewItem *item = kopsik_time_entry_view_item_init();
  int was_found = 0;
  NSTimeInterval startedAt = [idleEvent.started timeIntervalSince1970];
  NSLog(@"Time entry stop at %f", startedAt);
  kopsik_api_result res = kopsik_stop_running_time_entry_at(ctx,
                                                            err,
                                                            KOPSIK_ERR_LEN,
                                                            startedAt,
                                                            item,
                                                            &was_found);
  if (KOPSIK_API_SUCCESS != res) {
    kopsik_time_entry_view_item_clear(item);
    handle_error(res, err);
    return;
  }

  if (was_found) {
    TimeEntryViewItem *timeEntry = [[TimeEntryViewItem alloc] init];
    [timeEntry load:item];
    [[NSNotificationCenter defaultCenter]
      postNotificationName:kUIStateTimerStopped
      object:timeEntry];
  }

  kopsik_sync(ctx, 0, handle_error);
}

- (void)userLoggedIn:(User *)user {
  self.lastKnownLoginState = kUIStateUserLoggedIn;

  // Start syncing after a while.
  [self performSelector:@selector(startSync)
             withObject:nil
             afterDelay:0.5];
  [self performSelector:@selector(startWebSocket)
             withObject:nil
             afterDelay:0.5];

  [self performSelector:@selector(startTimeline)
             withObject:nil
             afterDelay:0.5];
  
  renderRunningTimeEntry();
}

- (void)userLoggedOut {
  self.lastKnownLoginState = kUIStateUserLoggedOut;
  self.lastKnownTrackingState = kUIStateTimerStopped;
  self.lastKnownRunningTimeEntry = nil;
  [self stopWebSocket];
  [self stopTimeline];
}

- (void)timerStopped {
  self.lastKnownRunningTimeEntry = nil;
  self.lastKnownTrackingState = kUIStateTimerStopped;
}

- (void)timerStarted:(TimeEntryViewItem *)timeEntry {
  self.lastKnownRunningTimeEntry = timeEntry;
  self.lastKnownTrackingState = kUIStateTimerRunning;
}

- (void)modelChanged:(ModelChange *)modelChange {
   if (self.lastKnownRunningTimeEntry &&
      [self.lastKnownRunningTimeEntry.GUID isEqualToString:modelChange.GUID] &&
      [modelChange.ModelType isEqualToString:@"time_entry"] &&
      [modelChange.ChangeType isEqualToString:@"update"]) {
    // Time entry duration can be edited on server side and it's
    // pushed to us via websocket or pulled via regular sync.
    // When it happens, timer keeps on running, but the time should be
    // updated on status item:
    self.lastKnownRunningTimeEntry = [TimeEntryViewItem findByGUID:modelChange.GUID];
  }
}

- (void)settingsChanged {
  [self updateIdleDetectionTimer];
  if (![self.lastKnownLoginState isEqualToString:kUIStateUserLoggedOut]) {
    [self startSync];
    [self startWebSocket];
  }
}

- (void)eventHandler: (NSNotification *) notification {
  if ([notification.name isEqualToString:kUIEventSettingsChanged]) {
    [self settingsChanged];
  } else if ([notification.name isEqualToString:kUICommandShowPreferences]) {
    [self onPreferencesMenuItem:self];
  } else if ([notification.name isEqualToString:kUICommandNew]) {
    [self startNewTimeEntry:notification.object];
  } else if ([notification.name isEqualToString:kUICommandContinue]) {
    [self continueTimeEntry:notification.object];
  } else if ([notification.name isEqualToString:kUICommandStop]) {
    [self stopTimeEntry];
  } else if ([notification.name isEqualToString:kUIStateUserLoggedIn]) {
    [self userLoggedIn:notification.object];
  } else if ([notification.name isEqualToString:kUIStateUserLoggedOut]) {
    [self userLoggedOut];
  } else if ([notification.name isEqualToString:kUIStateTimerStopped]) {
    [self timerStopped];
  } else if ([notification.name isEqualToString:kUIStateTimerRunning]) {
    [self timerStarted:notification.object];
  } else if ([notification.name isEqualToString:kUIEventModelChange]) {
    [self modelChanged:notification.object];
  } else if ([notification.name isEqualToString:kUICommandSplitAt]) {
    [self splitTimeEntryAfterIdle:notification.object];
  } else if ([notification.name isEqualToString:kUICommandStopAt]) {
    [self stopTimeEntryAfterIdle:notification.object];
  }
  [self updateStatus];
}

- (void)updateStatus {
  if (self.lastKnownRunningTimeEntry == nil) {
    [self.statusItem setTitle: kTimeTotalUnknown];
    [self.statusItem setImage:self.offImage];
    [self.runningTimeEntryMenuItem setTitle:@"Timer is not running."];
    return;
  }
  
  [self.statusItem setImage:self.onImage];
  NSString *msg = [NSString stringWithFormat:@"Running: %@",
                    self.lastKnownRunningTimeEntry.Description];
  [self.runningTimeEntryMenuItem setTitle:msg];
}

- (void)createStatusItem {
  NSMenu *menu = [[NSMenu alloc] init];
  self.runningTimeEntryMenuItem = [menu addItemWithTitle:@"Timer status"
                                                  action:nil
                                           keyEquivalent:@""];
  [menu addItem:[NSMenuItem separatorItem]];
  [menu addItemWithTitle:@"New"
                  action:@selector(onNewMenuItem:)
           keyEquivalent:@""].tag = kMenuItemTagNew;
  [menu addItemWithTitle:@"Continue"
                  action:@selector(onContinueMenuItem)
           keyEquivalent:@""].tag = kMenuItemTagContinue;
  [menu addItemWithTitle:@"Stop"
                  action:@selector(onStopMenuItem)
           keyEquivalent:@""].tag = kMenuItemTagStop;
  [menu addItem:[NSMenuItem separatorItem]];
  [menu addItemWithTitle:@"Show"
                  action:@selector(onShowMenuItem)
           keyEquivalent:@""];
  [menu addItem:[NSMenuItem separatorItem]];
  [menu addItemWithTitle:@"Sync"
                  action:@selector(onSyncMenuItem:)
           keyEquivalent:@""].tag = kMenuItemTagSync;
  [menu addItemWithTitle:@"Preferences"
                  action:@selector(onPreferencesMenuItem:)
           keyEquivalent:@""];
  [menu addItem:[NSMenuItem separatorItem]];
  [menu addItemWithTitle:@"About"
                  action:@selector(onAboutMenuItem:)
           keyEquivalent:@""];
  NSMenuItem *sendFeedbackMenuItem = [menu addItemWithTitle:@"Send Feedback"
                  action:@selector(onSendFeedbackMenuItem)
           keyEquivalent:@""];
  sendFeedbackMenuItem.tag = kMenuItemTagSendFeedback;
  [menu addItemWithTitle:@"Logout"
                  action:@selector(onLogoutMenuItem:)
           keyEquivalent:@""].tag = kMenuItemTagLogout;;
  [menu addItemWithTitle:@"Quit"
                  action:@selector(onQuitMenuItem)
           keyEquivalent:@""];
  
  NSStatusBar *bar = [NSStatusBar systemStatusBar];
  
  self.onImage = [NSImage imageNamed:@"on"];
  self.offImage = [NSImage imageNamed:@"off"];
  
  self.statusItem = [bar statusItemWithLength:NSVariableStatusItemLength];
  [self.statusItem setTitle: kTimeTotalUnknown];
  [self.statusItem setHighlightMode:YES];
  [self.statusItem setEnabled:YES];
  [self.statusItem setMenu:menu];
  [self.statusItem setImage:self.offImage];

  self.statusItemTimer = [NSTimer
    scheduledTimerWithTimeInterval:1.0
    target:self
    selector:@selector(statusItemTimerFired:)
    userInfo:nil
    repeats:YES];
  [self updateIdleDetectionTimer];
}

- (void)updateIdleDetectionTimer {
  // Start idle detection, if its enabled
  KopsikSettings *settings = kopsik_settings_init();
  char err[KOPSIK_ERR_LEN];
  kopsik_api_result res = kopsik_get_settings(ctx,
                                              err,
                                              KOPSIK_ERR_LEN,
                                              settings);
  if (KOPSIK_API_SUCCESS != res) {
    kopsik_settings_clear(settings);
    handle_error(res, err);
    return;
  }
  if (settings->UseIdleDetection) {
    NSLog(@"Starting idle detection");
    self.idleTimer = [NSTimer
      scheduledTimerWithTimeInterval:1.0
      target:self
      selector:@selector(idleTimerFired:)
      userInfo:nil
      repeats:YES];
  } else {
    NSLog(@"Idle detection is disabled. Stopping idle detection.");
    if (self.idleTimer != nil) {
      [self.idleTimer invalidate];
      self.idleTimer = nil;
    }
  }

  kopsik_settings_clear(settings);
}

- (void)onNewMenuItem:(id)sender {
  [[NSNotificationCenter defaultCenter]
    postNotificationName:kUICommandNew
    object:[[TimeEntryViewItem alloc] init]];
}

- (void)onSendFeedbackMenuItem {
  [self.feedbackWindowController showWindow:self];
  [NSApp activateIgnoringOtherApps:YES];
}

- (void)onSendFeedbackMainMenuItem:(id)sender {
  [self onSendFeedbackMenuItem];
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
  [self startSync];
}

- (IBAction)onHelpMenuItem:(id)sender {
  [[NSWorkspace sharedWorkspace] openURL:
    [NSURL URLWithString:@"http://support.toggl.com/toggl-on-my-desktop/"]];
}

- (IBAction)onLogoutMenuItem:(id)sender {
  char err[KOPSIK_ERR_LEN];
  if (KOPSIK_API_SUCCESS != kopsik_logout(ctx, err, KOPSIK_ERR_LEN)) {
    [[NSNotificationCenter defaultCenter]
      postNotificationName:kUIStateError
      object:[NSString stringWithUTF8String:err]];
    return;
  }
  [[NSNotificationCenter defaultCenter]
    postNotificationName:kUIStateUserLoggedOut object:nil];
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
  
  [[NSNotificationCenter defaultCenter] postNotificationName:kUIStateUserLoggedOut object:nil];
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

- (IBAction)onHideMenuItem:(id)sender {
  [self.mainWindowController.window close];
}

void on_timeline_toggle_recording_result(kopsik_api_result result,
                                         const char *errmsg) {
  handle_error(result, errmsg);
}

- (void)onQuitMenuItem {
  [[NSApplication sharedApplication] terminate:self];
}

- (void)applicationWillTerminate:(NSNotification *)app {
  NSLog(@"applicationWillTerminate");
  kopsik_context_shutdown(ctx);
  kopsik_context_clear(ctx);
  ctx = 0;
}

- (BOOL)applicationShouldHandleReopen:(NSApplication *)sender
    hasVisibleWindows:(BOOL)flag{
  [self.mainWindowController.window setIsVisible:YES];
  return YES;
}

- (NSString *)applicationSupportDirectory {
  NSString *path;
  NSError *error;
  NSArray* paths = NSSearchPathForDirectoriesInDomains(
    NSApplicationSupportDirectory, NSUserDomainMask, YES);
  if ([paths count] == 0) {
    NSLog(@"Unable to access application support directory!");
  }
  path = [paths[0] stringByAppendingPathComponent:@"Kopsik"];

  // Append environment name to path. So we can have
  // production and development running side by side.
  path = [path stringByAppendingPathComponent:self.environment];
  
	if ([[NSFileManager defaultManager] fileExistsAtPath:path]){
    return path;
  }
	if (![[NSFileManager defaultManager] createDirectoryAtPath:path
                                 withIntermediateDirectories:YES
                                                  attributes:nil
                                                       error:&error]){
		NSLog(@"Create directory error: %@", error);
	}
  return path;
}

const NSString *appName = @"osx_native_app";

- (void)parseCommandLineArguments{
  NSArray *arguments = [[NSProcessInfo processInfo] arguments];
  NSLog(@"Command line arguments: %@", arguments);
  
  for (int i = 1; i < arguments.count; i++) {
    NSString *argument = arguments[i];

    if ([argument rangeOfString:@"email"].location != NSNotFound) {
      defaultEmail = arguments[i+1];
      continue;
    }
    if ([argument rangeOfString:@"password"].location != NSNotFound) {
      defaultPassword = arguments[i+1];
      continue;
    }
    if (([argument rangeOfString:@"force"].location != NSNotFound) &&
        ([argument rangeOfString:@"crash"].location != NSNotFound)) {
      NSLog(@"forcing crash");
      self.forceCrash = YES;
      continue;
    }
    if (([argument rangeOfString:@"log"].location != NSNotFound) &&
        ([argument rangeOfString:@"path"].location != NSNotFound)) {
      self.log_path = arguments[i+1];
      NSLog(@"log path overriden with '%@'", self.log_path);
      continue;
    }
    if (([argument rangeOfString:@"db"].location != NSNotFound) &&
        ([argument rangeOfString:@"path"].location != NSNotFound)) {
      self.db_path = arguments[i+1];
      NSLog(@"db path overriden with '%@'", self.db_path);
      continue;
    }
    if (([argument rangeOfString:@"log"].location != NSNotFound) &&
        ([argument rangeOfString:@"level"].location != NSNotFound)) {
      self.log_level = arguments[i+1];
      NSLog(@"log level overriden with '%@'", self.log_level);
      continue;
    }
    if (([argument rangeOfString:@"api"].location != NSNotFound) &&
        ([argument rangeOfString:@"url"].location != NSNotFound)) {
      self.api_url_override = arguments[i+1];
      NSLog(@"API URL overriden with '%@'", self.api_url_override);
      continue;
    }
    if (([argument rangeOfString:@"websocket"].location != NSNotFound) &&
        ([argument rangeOfString:@"url"].location != NSNotFound)) {
      self.websocket_url_override = arguments[i+1];
      NSLog(@"Websocket URL overriden with '%@'", self.websocket_url_override);
      continue;
    }
  }
}

- (void)disallowDuplicateInstances {
  // Disallow duplicate instances in production
  if (![self.environment isEqualToString:@"production"]) {
    return;
  }
  if ([[NSRunningApplication runningApplicationsWithBundleIdentifier:
        [[NSBundle mainBundle] bundleIdentifier]] count] > 1) {
    NSString *msg = [NSString
                     stringWithFormat:@"Another copy of %@ is already running.",
                     [[NSBundle mainBundle]
                      objectForInfoDictionaryKey:(NSString *)kCFBundleNameKey]];
    [[NSAlert alertWithMessageText:msg
                     defaultButton:nil
                   alternateButton:nil
                       otherButton:nil
         informativeTextWithFormat:@"This copy will now quit."] runModal];
  
    [NSApp terminate:nil];
  }
}

- (id) init {
  self = [super init];

  NSDictionary* infoDict = [[NSBundle mainBundle] infoDictionary];
  self.environment = infoDict[@"KopsikEnvironment"];

  [self disallowDuplicateInstances];

  [Bugsnag startBugsnagWithApiKey:@"2a46aa1157256f759053289f2d687c2f"];
  NSAssert(self.environment != nil, @"Missing environment in plist");
  [Bugsnag configuration].releaseStage = self.environment;

  self.app_path = [self applicationSupportDirectory];
  self.db_path = [self.app_path stringByAppendingPathComponent:@"kopsik.db"];
  self.log_path = [self.app_path stringByAppendingPathComponent:@"kopsik.log"];
  self.log_level = @"debug";

  [self parseCommandLineArguments];

  NSLog(@"Starting with db path %@, log path %@, log level %@",
    self.db_path, self.log_path, self.log_level);

  kopsik_set_log_path([self.log_path UTF8String]);
  kopsik_set_log_level([self.log_level UTF8String]);

  NSString* version = infoDict[@"CFBundleShortVersionString"];
  ctx = kopsik_context_init([appName UTF8String], [version UTF8String]);
  NSLog(@"Version %@", version);

  char err[KOPSIK_ERR_LEN];
  kopsik_api_result res =
    kopsik_set_db_path(ctx, err, KOPSIK_ERR_LEN, [self.db_path UTF8String]);
  NSAssert(KOPSIK_API_SUCCESS == res, @"Failed to initialize DB");

  id logToFile = infoDict[@"KopsikLogUserInterfaceToFile"];
  if ([logToFile boolValue]) {
    NSLog(@"Redirecting UI log to file");
    NSString *logPath =
      [self.app_path stringByAppendingPathComponent:@"ui.log"];
    freopen([logPath fileSystemRepresentation],"a+", stderr);
  }

  res = kopsik_configure_proxy(ctx, err, KOPSIK_ERR_LEN);
  NSAssert(KOPSIK_API_SUCCESS == res, @"Failed to initialize DB");

  if (self.api_url_override != nil) {
    kopsik_set_api_url(ctx, [self.api_url_override UTF8String]);
  }

  if (self.websocket_url_override != nil) {
    kopsik_set_websocket_url(ctx, [self.websocket_url_override UTF8String]);
  }

  NSLog(@"AppDelegate init done");
  
  return self;
}

- (void)statusItemTimerFired:(NSTimer*)timer
{
  if (self.lastKnownRunningTimeEntry != nil) {
    char str[duration_str_len];
    if (blink) {
      blink = 0;
    } else {
      blink = 1;
    }
    kopsik_format_duration_in_seconds_hhmm(
      self.lastKnownRunningTimeEntry.duration_in_seconds,
      blink,
      str,
      duration_str_len);

    NSString *statusStr;
    statusStr = @" ";
    statusStr = [statusStr stringByAppendingString:[NSString stringWithUTF8String:str]];
    [self.statusItem setTitle:statusStr];
  }
}

const int kIdleThresholdSeconds = 5 * 60;

- (void)idleTimerFired:(NSTimer*)timer {
  uint64_t idle_seconds = 0;
  if (0 != get_idle_time(&idle_seconds)) {
    NSLog(@"Achtung! Failed to get idle status.");
    return;
  }
  
//  NSLog(@"Idle seconds: %lld", idle_seconds);

  if (idle_seconds >= kIdleThresholdSeconds && self.lastIdleStarted == nil) {
    self.lastIdleStarted = [NSDate date];
    NSLog(@"User is idle since %@", self.lastIdleStarted);

  } else if (self.lastIdleStarted != nil &&
      self.lastIdleSecondsReading >= idle_seconds) {
    NSDate *now = [NSDate date];
    if (self.lastKnownRunningTimeEntry) {
      IdleEvent *idleEvent = [[IdleEvent alloc] init];
      idleEvent.started = self.lastIdleStarted;
      idleEvent.finished = now;
      idleEvent.seconds = self.lastIdleSecondsReading;
      [[NSNotificationCenter defaultCenter]
        postNotificationName:kUIEventIdleFinished
        object:idleEvent];
    } else {
      NSLog(@"Time entry is not running, ignoring idleness");
    }
    NSLog(@"User is not idle since %@", now);
    self.lastIdleStarted = nil;
  }
  
  self.lastIdleSecondsReading = idle_seconds;
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
      case kMenuItemTagSync:
        if (self.lastKnownLoginState != kUIStateUserLoggedIn) {
          return NO;
        }
        break;
      case kMenuItemTagLogout:
        if (self.lastKnownLoginState != kUIStateUserLoggedIn) {
          return NO;
        }
        break;
      case kMenuItemTagClearCache:
        if (self.lastKnownLoginState != kUIStateUserLoggedIn) {
          return NO;
        }
        break;
      case kMenuItemTagSendFeedback:
        if (self.lastKnownLoginState != kUIStateUserLoggedIn) {
          return NO;
        }
        break;
      default:
        // Dont care about this stuff
        break;
    }
    return YES;
}

void sync_finished(kopsik_api_result result, const char *err) {
  NSLog(@"sync_finished");
  if (KOPSIK_API_SUCCESS != result) {
    [[NSNotificationCenter defaultCenter]
      postNotificationName:kUIStateError
      object:[NSString stringWithUTF8String:err]];
    return;
  }
  renderRunningTimeEntry();
}

void renderRunningTimeEntry() {
  KopsikTimeEntryViewItem *item = kopsik_time_entry_view_item_init();
  int is_tracking = 0;
  char err[KOPSIK_ERR_LEN];
  if (KOPSIK_API_SUCCESS != kopsik_running_time_entry_view_item(ctx,
      err,
      KOPSIK_ERR_LEN,
      item,
      &is_tracking)) {
    [[NSNotificationCenter defaultCenter]
      postNotificationName:kUIStateError
      object:[NSString stringWithUTF8String:err]];
    kopsik_time_entry_view_item_clear(item);
    return;
  }

  if (is_tracking) {
    TimeEntryViewItem *timeEntry = [[TimeEntryViewItem alloc] init];
    [timeEntry load:item];
    [[NSNotificationCenter defaultCenter]
      postNotificationName:kUIStateTimerRunning object:timeEntry];
  } else {
    [[NSNotificationCenter defaultCenter]
      postNotificationName:kUIStateTimerStopped object:nil];
  }
  kopsik_time_entry_view_item_clear(item);
}

void on_model_change(kopsik_api_result result,
                     const char *errmsg,
                     KopsikModelChange *change) {
  NSLog(@"on_model_change %s %s ID=%d GUID=%s in thread %@",
        change->ChangeType,
        change->ModelType,
        change->ModelID,
        change->GUID,
        [NSThread currentThread]);
  
  if (KOPSIK_API_SUCCESS != result) {
    [[NSNotificationCenter defaultCenter]
      postNotificationName:kUIStateError
      object:[NSString stringWithUTF8String:errmsg]];
    return;
  }

  ModelChange *modelChange = [[ModelChange alloc] init];
  [modelChange load:change];

  [[NSNotificationCenter defaultCenter]
    postNotificationName:kUIEventModelChange object:modelChange];
}

- (void)startSync {
  NSLog(@"startSync");
  kopsik_sync(ctx, 1, sync_finished);
}

- (void)checkForUpdates {
  kopsik_check_for_updates(ctx, check_for_updates_callback);
}

void check_for_updates_callback(kopsik_api_result result,
                                const char *errmsg,
                                const int is_update_available,
                                const char *url,
                                const char *version) {
  if (result != KOPSIK_API_SUCCESS) {
    handle_error(result, errmsg);
    return;
  }
  if (!is_update_available) {
    [[NSNotificationCenter defaultCenter]
      postNotificationName:kUIStateUpToDate
      object:nil];
    NSLog(@"check_for_updates_callback: no updates available");
    return;
  }

  Update *update = [[Update alloc] init];
  update.URL = [NSString stringWithUTF8String:url];
  update.version = [NSString stringWithUTF8String:version];

  [[NSNotificationCenter defaultCenter]
    postNotificationName:kUIStateUpdateAvailable
    object:update];

  NSAlert *alert = [[NSAlert alloc] init];
  [alert addButtonWithTitle:@"Yes"];
  [alert addButtonWithTitle:@"No"];
  [alert setMessageText:@"Download new version?"];
  NSString *informative = [NSString stringWithFormat:
    @"There's a new version of this app available (%@).", update.version];
  [alert setInformativeText:informative];
  [alert setAlertStyle:NSWarningAlertStyle];
  if ([alert runModal] != NSAlertFirstButtonReturn) {
    return;
  }

  [[NSWorkspace sharedWorkspace] openURL:[NSURL URLWithString:update.URL]];
  [NSApp terminate:nil];
}

- (void) handleCrashReport {
  PLCrashReporter *crashReporter = [PLCrashReporter sharedReporter];

  NSError *error;
  NSData *crashData = [crashReporter loadPendingCrashReportDataAndReturnError: &error];
  if (crashData == nil) {
    NSLog(@"Could not load crash report: %@", error);
    [crashReporter purgePendingCrashReport];
    return;
  }

  PLCrashReport *report = [[PLCrashReport alloc] initWithData: crashData
                                                        error: &error];
  if (report == nil) {
    NSLog(@"Could not parse crash report");
    [crashReporter purgePendingCrashReport];
    return;
  }

  NSString *summary = [NSString stringWithFormat:@"Crashed with signal %@ (code %@, address=0x%" PRIx64 ")",
                       report.signalInfo.name,
                       report.signalInfo.code,
                       report.signalInfo.address];

  NSLog(@"Crashed on %@", report.systemInfo.timestamp);
  NSLog(@"%@", summary);

  NSException* exception;
  NSMutableDictionary *data = [[NSMutableDictionary alloc] init];;
  if (report.hasExceptionInfo) {
    exception = [NSException
      exceptionWithName:report.exceptionInfo.exceptionName
      reason:report.exceptionInfo.exceptionReason
      userInfo:nil];
  } else {
    exception = [NSException
      exceptionWithName:@"Crash"
      reason:summary
      userInfo:nil];
  }
  [Bugsnag notify:exception withData:data];

  [crashReporter purgePendingCrashReport];
}

@end
