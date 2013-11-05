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

@interface  AppDelegate()
@property (nonatomic,strong) IBOutlet MainWindowController *mainWindowController;
@property (nonatomic,strong) IBOutlet PreferencesWindowController *preferencesWindowController;
@property (nonatomic,strong) IBOutlet AboutWindowController *aboutWindowController;
@property TimeEntryViewItem *running_time_entry;
@property NSTimer *statusItemTimer;
@property NSTimer *idleTimer;
@property NSString *lastKnownLoginState;
@property NSString *lastKnownTrackingState;
@property int lastIdleSecondsReading;
@property NSDate *lastIdleStarted;
@end

@implementation AppDelegate

int blink = 0;

NSString *kTimeTotalUnknown = @"--:--";

- (void)applicationDidFinishLaunching:(NSNotification *)aNotification
{
  NSLog(@"applicationDidFinishLaunching");
  
  self.mainWindowController = [[MainWindowController alloc] initWithWindowNibName:@"MainWindowController"];
  [self.mainWindowController.window setReleasedWhenClosed:NO];
  [self onShowMenuItem];

  self.preferencesWindowController = [[PreferencesWindowController alloc] initWithWindowNibName:@"PreferencesWindowController"];

  self.aboutWindowController = [[AboutWindowController alloc] initWithWindowNibName:@"AboutWindowController"];
  
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

  kopsik_set_change_callback(ctx, on_model_change);
  
  char err[KOPSIK_ERR_LEN];
  KopsikUser *user = kopsik_user_init();
  if (KOPSIK_API_SUCCESS != kopsik_current_user(ctx, err, KOPSIK_ERR_LEN, user)) {
    [[NSNotificationCenter defaultCenter] postNotificationName:kUIStateError
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
    [[NSNotificationCenter defaultCenter] postNotificationName:kUIStateUserLoggedOut object:nil];
  } else {
    [[NSNotificationCenter defaultCenter] postNotificationName:kUIStateUserLoggedIn object:userinfo];
  }
 
  NSDictionary* infoDict = [[NSBundle mainBundle] infoDictionary];
  NSNumber* checkEnabled = [infoDict objectForKey:@"KopsikCheckForUpdates"];
  if ([checkEnabled boolValue]) {
    [self checkForUpdates];
  }
  
}

- (void)startWebSocket {
  NSLog(@"startWebSocket");
  kopsik_websocket_start_async(ctx, handle_error);
  NSLog(@"startWebSocket done");
}

- (void)startTimeline {
  NSLog(@"startTimeline");
  char err[KOPSIK_ERR_LEN];
  kopsik_api_result res = kopsik_timeline_start(ctx, err, KOPSIK_ERR_LEN);
  if (KOPSIK_API_SUCCESS != res) {
    handle_error(res, err);
  }
  NSLog(@"startTimeline done");
}

-(void)eventHandler: (NSNotification *) notification
{
  if ([notification.name isEqualToString:kUICommandShowPreferences]) {
    [self onPreferencesMenuItem:self];
    return;
  }
  
  if ([notification.name isEqualToString:kUIStateUserLoggedIn]) {
    self.lastKnownLoginState = kUIStateUserLoggedIn;

    // Start syncing after a while.
    [self performSelector:@selector(startSync) withObject:nil afterDelay:0.5];
    [self performSelector:@selector(startWebSocket) withObject:nil afterDelay:0.5];
    [self performSelector:@selector(startTimeline) withObject:nil afterDelay:0.5];
    
    renderRunningTimeEntry();

  } else if ([notification.name isEqualToString:kUIStateUserLoggedOut]) {
    self.lastKnownLoginState = kUIStateUserLoggedOut;
    self.lastKnownTrackingState = kUIStateTimerStopped;
    self.running_time_entry = nil;
    kopsik_websocket_stop_async(ctx, handle_error);
    kopsik_timeline_stop(ctx);

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
  [menu addItemWithTitle:@"Sync" action:@selector(onSyncMenuItem:) keyEquivalent:@""].tag = kMenuItemTagSync;
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
  self.idleTimer = [NSTimer scheduledTimerWithTimeInterval:5.0
                                                    target:self
                                                  selector:@selector(idleTimerFired:)
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
  [self startSync];
}

- (IBAction)onHelpMenuItem:(id)sender {
  [[NSWorkspace sharedWorkspace] openURL:[NSURL URLWithString:@"http://support.toggl.com/toggl-on-my-desktop/"]];
}

- (IBAction)onLogoutMenuItem:(id)sender {
  char err[KOPSIK_ERR_LEN];
  if (KOPSIK_API_SUCCESS != kopsik_logout(ctx, err, KOPSIK_ERR_LEN)) {
    [[NSNotificationCenter defaultCenter] postNotificationName:kUIStateError
                                                        object:[NSString stringWithUTF8String:err]];
    return;
  }
  [[NSNotificationCenter defaultCenter] postNotificationName:kUIStateUserLoggedOut object:nil];
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
  ProcessSerialNumber psn = { 0, kCurrentProcess };
  TransformProcessType(&psn, kProcessTransformToForegroundApplication);
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

const int kIdleThresholdSeconds = 10; // lower value for testing

- (void)idleTimerFired:(NSTimer*)timer {
  uint64_t idle_seconds = 0;
  if (0 != get_idle_time(&idle_seconds)) {
    NSLog(@"Achtung! Failed to get idle status.");
    return;
  }
  NSLog(@"Idle seconds: %qi", idle_seconds);

  if (idle_seconds >= kIdleThresholdSeconds && self.lastIdleStarted == nil) {
    self.lastIdleStarted = [NSDate date];
    NSLog(@"User is idle since %@", self.lastIdleStarted);

  } else if (self.lastIdleStarted != nil && self.lastIdleSecondsReading >= idle_seconds) {
    NSDate *now = [NSDate date];
    IdleEvent *idleEvent = [[IdleEvent alloc] init];
    idleEvent.started = self.lastIdleStarted;
    idleEvent.finished = now;
    idleEvent.seconds = self.lastIdleSecondsReading;
    NSLog(@"User is not idle since %@", now);
    [[NSNotificationCenter defaultCenter] postNotificationName:kUIEventIdleFinished
                                                        object:idleEvent];
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
    }
    return YES;
}

void sync_finished(kopsik_api_result result, const char *err) {
  NSLog(@"sync_finished");
  if (KOPSIK_API_SUCCESS != result) {
    [[NSNotificationCenter defaultCenter] postNotificationName:kUIStateError
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
                                                                err, KOPSIK_ERR_LEN,
                                                                item, &is_tracking)) {
    [[NSNotificationCenter defaultCenter] postNotificationName:kUIStateError
                                                        object:[NSString stringWithUTF8String:err]];
    kopsik_time_entry_view_item_clear(item);
    return;
  }

  if (is_tracking) {
    TimeEntryViewItem *te = [[TimeEntryViewItem alloc] init];
    [te load:item];
    [[NSNotificationCenter defaultCenter] postNotificationName:kUIStateTimerRunning object:te];
  } else {
    [[NSNotificationCenter defaultCenter] postNotificationName:kUIStateTimerStopped object:nil];
  }
  kopsik_time_entry_view_item_clear(item);
}

void on_model_change(kopsik_api_result result,
                     const char *errmsg,
                     KopsikModelChange *change) {
  if (KOPSIK_API_SUCCESS != result) {
    [[NSNotificationCenter defaultCenter] postNotificationName:kUIStateError
                                                          object:[NSString stringWithUTF8String:errmsg]];
    return;
  }

  ModelChange *mc = [[ModelChange alloc] init];
  [mc load:change];

  [[NSNotificationCenter defaultCenter] postNotificationName:kUIEventModelChange object:mc];
}

- (void)startSync {
  NSLog(@"startSync");
  kopsik_sync_async(ctx, 1, sync_finished);
  NSLog(@"startSync done");
}

- (void) checkForUpdates {
  kopsik_check_for_updates_async(ctx, check_for_updates_callback);
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
    [[NSNotificationCenter defaultCenter] postNotificationName:kUIStateUpToDate
                                                        object:nil];
    NSLog(@"check_for_updates_callback: no updates available");
    return;
  }

  Update *update = [[Update alloc] init];
  update.URL = [NSString stringWithUTF8String:url];
  update.version = [NSString stringWithUTF8String:version];

  [[NSNotificationCenter defaultCenter] postNotificationName:kUIStateUpdateAvailable
                                                      object:update];

  NSAlert *alert = [[NSAlert alloc] init];
  [alert addButtonWithTitle:@"Yes"];
  [alert addButtonWithTitle:@"No"];
  [alert setMessageText:@"Download new version?"];
  NSString *informative = [NSString stringWithFormat:@"There's a new version of this app available (%@).", update.version];
  [alert setInformativeText:informative];
  [alert setAlertStyle:NSWarningAlertStyle];
  if ([alert runModal] != NSAlertFirstButtonReturn) {
    return;
  }
  
  [[NSWorkspace sharedWorkspace] openURL:[NSURL URLWithString:update.URL]];
}

@end
