//
//  AppDelegate.m
//  test2
//
//  Created by Alari on 9/15/13.
//  Copyright (c) 2013 TogglDesktop developers. All rights reserved.
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
#import "const.h"
#import "EditNotification.h"
#import "MASShortcut+UserDefaults.h"

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
@property NSTimer *menubarTimer;
@property NSTimer *idleTimer;
@property NSString *lastKnownLoginState;
@property NSString *lastKnownTrackingState;
@property long lastIdleSecondsReading;
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

// Avoid showing multiple upgrade dialogs
@property BOOL upgradeDialogVisible;

@end

@implementation AppDelegate

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
  
  PLCrashReporter *crashReporter = [self configuredCrashReporter];
  
  // Check if we previously crashed
  if ([crashReporter hasPendingCrashReport]) {
    [self handleCrashReport];
  }
  
  // Enable the Crash Reporter
  NSError *error;
  if (![crashReporter enableCrashReporterAndReturnError: &error]) {
    NSLog(@"Warning: Could not enable crash reporter: %@", error);
  }
  
  if (self.forceCrash) {
    abort();
  }
  
  [self onShowMenuItem:self];

  self.inactiveAppIcon = [NSImage imageNamed:@"app_inactive"];

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
  
  [self applySettings];

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
  [[NSNotificationCenter defaultCenter] addObserver:self
                                           selector:@selector(eventHandler:)
                                               name:kUIStateOffline
                                             object:nil];
  [[NSNotificationCenter defaultCenter] addObserver:self
                                           selector:@selector(eventHandler:)
                                               name:kUIStateOnline
                                             object:nil];
  [[NSNotificationCenter defaultCenter] addObserver:self
                                           selector:@selector(eventHandler:)
                                               name:kUIStateUpdateAvailable
                                             object:nil];

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

  [MASShortcut registerGlobalShortcutWithUserDefaultsKey:kPreferenceGlobalShortcutShowHide handler:^{
    if ([self.mainWindowController.window isVisible]) {
      [self.mainWindowController.window close];
    } else {
      [self onShowMenuItem:self];
    }
  }];

  [MASShortcut registerGlobalShortcutWithUserDefaultsKey:kPreferenceGlobalShortcutStartStop handler:^{
    if ([self.lastKnownTrackingState isEqualTo:kUIStateTimerStopped]) {
      [self onNewMenuItem:self];
    } else {
      [self onStopMenuItem:self];
    }
  }];
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
  kopsik_timeline_switch(ctx, 1);
}

- (void)stopTimeline {
  NSLog(@"stopTimeline");
  kopsik_timeline_switch(ctx, 0);
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
    handle_error(err);
    return;
  }

  TimeEntryViewItem *timeEntry = [[TimeEntryViewItem alloc] init];
  [timeEntry load:item];
  kopsik_time_entry_view_item_clear(item);
  if (timeEntry.duration_in_seconds < 0) {
    [[NSNotificationCenter defaultCenter]
     postNotificationName:kUIStateTimerRunning object:timeEntry];

    EditNotification *edit = [[EditNotification alloc] init];
    edit.GUID = timeEntry.GUID;
    edit.FieldName = kUIDescriptionClicked;
    [[NSNotificationCenter defaultCenter] postNotificationName:kUIStateTimeEntrySelected
                                                      object:edit];
  }

  [self onShowMenuItem:self];
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

  [self onShowMenuItem:self];
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

  [self onShowMenuItem:self];
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
    handle_error(err);
    return;
  }

  if (was_found) {
    TimeEntryViewItem *timeEntry = [[TimeEntryViewItem alloc] init];
    [timeEntry load:item];
    [[NSNotificationCenter defaultCenter]
      postNotificationName:kUIStateTimerStopped
      object:timeEntry];
  }

  [self onShowMenuItem:self];
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

  [NSApp setApplicationIconImage: self.inactiveAppIcon];
}

- (void)timerStopped {
  self.lastKnownRunningTimeEntry = nil;
  self.lastKnownTrackingState = kUIStateTimerStopped;

  [NSApp setApplicationIconImage: self.inactiveAppIcon];
}

- (void)timerStarted:(TimeEntryViewItem *)timeEntry {
  self.lastKnownRunningTimeEntry = timeEntry;
  self.lastKnownTrackingState = kUIStateTimerRunning;

  // Change app dock icon to default, which is red / tracking
  // See https://developer.apple.com/library/mac/documentation/Carbon/Conceptual/customizing_docktile/dockconcepts.pdf
  [NSApp setApplicationIconImage: nil];
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

- (void)eventHandler: (NSNotification *) notification {
  if ([notification.name isEqualToString:kUIEventSettingsChanged]) {
    [self applySettings];
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
  } else if ([notification.name isEqualToString:kUICommandStopAt]) {
    [self stopTimeEntryAfterIdle:notification.object];
  } else if ([notification.name isEqualToString:kUIStateOffline]) {
    [self offlineMode:true];
  } else if ([notification.name isEqualToString:kUIStateOnline]) {
    [self offlineMode:false];
  } else if ([notification.name isEqualToString:kUIStateUpdateAvailable]) {
    [self performSelectorOnMainThread:@selector(presentUpgradeDialog:)
      withObject:notification.object waitUntilDone:NO];
  }
  [self updateStatus];
}

- (void)offlineMode:(bool)offline {
  if (offline){
    self.currentOnImage = self.offlineOnImage;
    self.currentOffImage = self.offlineOffImage;
  } else {
    self.currentOnImage = self.onImage;
    self.currentOffImage = self.offImage;
  }
}

- (void)updateStatus {
  if (self.lastKnownRunningTimeEntry == nil) {
    [self.statusItem setTitle:@""];
    [self.statusItem setImage:self.currentOffImage];
    [self.runningTimeEntryMenuItem setTitle:@"Timer is not running."];
    return;
  }
  
  [self.statusItem setImage:self.currentOnImage];
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
           keyEquivalent:@"n"].tag = kMenuItemTagNew;
  [menu addItemWithTitle:@"Continue"
                  action:@selector(onContinueMenuItem:)
           keyEquivalent:@"o"].tag = kMenuItemTagContinue;
  [menu addItemWithTitle:@"Stop"
                  action:@selector(onStopMenuItem:)
           keyEquivalent:@"s"].tag = kMenuItemTagStop;
  [menu addItem:[NSMenuItem separatorItem]];
  [menu addItemWithTitle:@"Show"
                  action:@selector(onShowMenuItem:)
           keyEquivalent:@"t"];
  [menu addItem:[NSMenuItem separatorItem]];
  [menu addItemWithTitle:@"Sync"
                  action:@selector(onSyncMenuItem:)
           keyEquivalent:@""].tag = kMenuItemTagSync;
  [menu addItemWithTitle:@"Open in browser"
                  action:@selector(onOpenBrowserMenuItem:)
           keyEquivalent:@""].tag = kMenuItemTagOpenBrowser;
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

  self.offlineOnImage = [NSImage imageNamed:@"offline_on"];
  self.offlineOffImage = [NSImage imageNamed:@"offline_off"];

  self.currentOnImage = self.onImage;
  self.currentOffImage = self.offImage;
  
  self.statusItem = [bar statusItemWithLength:NSVariableStatusItemLength];
  [self.statusItem setTitle:@""];
  [self.statusItem setHighlightMode:YES];
  [self.statusItem setEnabled:YES];
  [self.statusItem setMenu:menu];
  [self.statusItem setImage:self.currentOffImage];
}

- (void)applySettings {
  unsigned int use_idle_detection = 0;
  unsigned int menubar_timer = 0;
  unsigned int dock_icon = 0;
  char err[KOPSIK_ERR_LEN];
  if (KOPSIK_API_SUCCESS != kopsik_get_settings(ctx,
                                                err,
                                                KOPSIK_ERR_LEN,
                                                &use_idle_detection,
                                                &menubar_timer,
                                                &dock_icon)) {
    handle_error(err);
    return;
  }

  // Start idle detection, if its enabled
  if (use_idle_detection) {
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
    [self.statusItem setTitle:@""];
  }

  // Start menubar timer if its enabled
  if (menubar_timer) {
    NSLog(@"Starting menubar timer");
    self.menubarTimer = [NSTimer
      scheduledTimerWithTimeInterval:1.0
      target:self
      selector:@selector(menubarTimerFired:)
      userInfo:nil
      repeats:YES];
  } else {
    NSLog(@"Menubar timer is disabled. Stopping menubar timer.");
    if (self.menubarTimer != nil) {
      [self.menubarTimer invalidate];
      self.menubarTimer = nil;
    }
    [self.statusItem setTitle:@""];
  }

  // Show/Hide dock icon
  ProcessSerialNumber psn = { 0, kCurrentProcess };
  if (dock_icon) {
    NSLog(@"Showing dock icon");
    TransformProcessType(&psn, kProcessTransformToForegroundApplication);
  } else {
    NSLog(@"Hiding dock icon.");
    TransformProcessType(&psn, kProcessTransformToUIElementApplication);
  }
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

- (IBAction)onContinueMenuItem:(id)sender {
  [[NSNotificationCenter defaultCenter] postNotificationName:kUICommandContinue
                                                      object:nil];
}

- (IBAction)onStopMenuItem:(id)sender {
  [[NSNotificationCenter defaultCenter] postNotificationName:kUICommandStop
                                                      object:nil];
}

- (IBAction)onSyncMenuItem:(id)sender {
  [self startSync];
}

- (IBAction)onOpenBrowserMenuItem:(id)sender {
  NSString *togglWebsiteURL = [NSString stringWithUTF8String:kTogglWebsiteURL];
  [[NSWorkspace sharedWorkspace] openURL:
    [NSURL URLWithString:togglWebsiteURL]];
}

- (IBAction)onHelpMenuItem:(id)sender {
  NSString *supportURL = [NSString stringWithUTF8String:kSupportURL];
  [[NSWorkspace sharedWorkspace] openURL:
    [NSURL URLWithString:supportURL]];
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

  [self onShowMenuItem:self];
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
    handle_error(err);
  }
  
  [[NSNotificationCenter defaultCenter] postNotificationName:kUIStateUserLoggedOut object:nil];
}

- (IBAction)onAboutMenuItem:(id)sender {
  [self.aboutWindowController showWindow:self];
  [NSApp activateIgnoringOtherApps:YES];
}

- (IBAction)onShowMenuItem:(id)sender {
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
  ctx = kopsik_context_init([appName UTF8String],
                            [version UTF8String],
                            on_model_change,
                            handle_error,
                            about_updates_checked,
                            application_online);
                            
  NSLog(@"Version %@", version);

  char err[KOPSIK_ERR_LEN];
  kopsik_api_result res =
    kopsik_set_db_path(ctx, err, KOPSIK_ERR_LEN, [self.db_path UTF8String]);
  NSAssert(KOPSIK_API_SUCCESS == res,
           ([NSString stringWithFormat:@"Failed to initialize DB with path: %@", self.db_path]));

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

- (void)menubarTimerFired:(NSTimer*)timer {
  if (self.lastKnownRunningTimeEntry != nil) {
    char str[duration_str_len];
    kopsik_format_duration_in_seconds_hhmm(
      self.lastKnownRunningTimeEntry.duration_in_seconds,
      str,
      duration_str_len);
    NSString *statusStr = @" ";
    statusStr = [statusStr stringByAppendingString:[NSString stringWithUTF8String:str]];
    [self.statusItem setTitle:statusStr];
  }
}

- (void)idleTimerFired:(NSTimer*)timer {
  uint64_t idle_seconds = 0;
  if (0 != get_idle_time(&idle_seconds)) {
    NSLog(@"Achtung! Failed to get idle status.");
    return;
  }
  
//  NSLog(@"Idle seconds: %lld", idle_seconds);

  if (idle_seconds >= kIdleThresholdSeconds && self.lastIdleStarted == nil) {
    NSTimeInterval since = [[NSDate date] timeIntervalSince1970] - idle_seconds;
    self.lastIdleStarted = [NSDate dateWithTimeIntervalSince1970:since];
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
      case kMenuItemTagOpenBrowser:
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
  kopsik_sync(ctx);
}

- (void)checkForUpdates {
  kopsik_check_for_updates(ctx);
}

void check_for_updates_callback(kopsik_api_result result,
                                const char *errmsg,
                                const int is_update_available,
                                const char *url,
                                const char *version) {
  if (KOPSIK_API_SUCCESS != result) {
    handle_error(errmsg);
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
}

- (void)presentUpgradeDialog:(Update *)update {
  if (self.upgradeDialogVisible) {
    NSLog(@"Upgrade dialog already visible");
    return;
  }
  self.upgradeDialogVisible = YES;

  NSAlert *alert = [[NSAlert alloc] init];
  [alert addButtonWithTitle:@"Yes"];
  [alert addButtonWithTitle:@"No"];
  [alert setMessageText:@"Download new version?"];
  NSString *informative = [NSString stringWithFormat:
    @"There's a new version of this app available (%@).", update.version];
  [alert setInformativeText:informative];
  [alert setAlertStyle:NSWarningAlertStyle];
  if ([alert runModal] != NSAlertFirstButtonReturn) {
    self.upgradeDialogVisible = NO;
    return;
  }

  [[NSWorkspace sharedWorkspace] openURL:[NSURL URLWithString:update.URL]];
  [NSApp terminate:nil];
}

- (PLCrashReporter *)configuredCrashReporter {
  PLCrashReporterConfig *config = [[PLCrashReporterConfig alloc]
                                   initWithSignalHandlerType:PLCrashReporterSignalHandlerTypeBSD
                                   symbolicationStrategy:PLCrashReporterSymbolicationStrategyAll];
  return [[PLCrashReporter alloc] initWithConfiguration: config];
}

- (void) handleCrashReport {
  PLCrashReporter *crashReporter = [self configuredCrashReporter];

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

  NSString *summary = [NSString stringWithFormat:@"Crashed with signal %@ (code %@)",
                       report.signalInfo.name,
                       report.signalInfo.code];

  NSString *humanReadable = [PLCrashReportTextFormatter stringValueForCrashReport:report
                                                                   withTextFormat:PLCrashReportTextFormatiOS];
  NSLog(@"Crashed on %@", report.systemInfo.timestamp);
  NSLog(@"Report: %@", humanReadable);

  NSException* exception;
  NSMutableDictionary *data = [[NSMutableDictionary alloc] init];;
  if (report.hasExceptionInfo) {
    exception = [NSException
      exceptionWithName:report.exceptionInfo.exceptionName
      reason:report.exceptionInfo.exceptionReason
      userInfo:nil];
  } else {
    exception = [NSException
      exceptionWithName:summary
      reason:humanReadable
      userInfo:nil];
  }
  [Bugsnag notify:exception withData:data];

  [crashReporter purgePendingCrashReport];
}

void about_updates_checked(
    const int is_update_available,
    const char *url,
    const char *version) {
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

void application_online() {
  [[NSNotificationCenter defaultCenter] postNotificationName:kUIStateOnline object:nil];
}

@end
